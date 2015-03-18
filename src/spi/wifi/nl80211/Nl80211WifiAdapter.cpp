/*
 * Copyright 2014-present Skyhook Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spi/WifiAdapter.h"
#include "spi/Concurrent.h"
#include "spi/Logger.h"
#include "spi/Time.h"

#include <string>
#include <vector>
#include <memory>

#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <errno.h>

#include <netlink/version.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/route/link.h>

#include <linux/nl80211.h>
#include <linux/if.h>

#define WPS_LOG_CATEGORY "WPS.SPI.Nl80211WifiAdapter"

#define WLAN_CAPABILITY_IBSS (1<<1)

namespace WPS {
namespace SPI {

/**********************************************************************/
/*                                                                    */
/* Nl80211WifiAdapter                                                 */
/*                                                                    */
/**********************************************************************/

class Nl80211WifiAdapter
    : public WifiAdapter
{
public:

    Nl80211WifiAdapter(const std::string& ifname, unsigned int ifindex)
        : _logger(WPS_LOG_CATEGORY)
        , _listener(NULL)
        , _ifindex(ifindex)
        , _ifname(ifname)
        , _familyId(-1)
        , _nl80211Sock(NULL)
        , _routeSock(NULL)
        , _listeningThread(0)
        , _cancelFd(-1)
        , _shouldBringDown(false)
    {
        init();
    }

    ~Nl80211WifiAdapter()
    {
        close();
        deinit();
    }

    void setListener(Listener* listener)
    {
        assert(! isOpen());
        _listener = listener;
    }

    std::string description() const
    {
        return _ifname;
    }

    ErrorCode open()
    {
        assert(_listener != NULL);

        if (! isInitialized())
            return SPI_ERROR;

        if (isOpen())
            return SPI_OK;

        _cancelFd = eventfd(0, 0);
        if (_cancelFd < 0)
        {
            _logger.error("eventfd() failed: %s", strerror(errno));
            return SPI_ERROR;
        }

        int rc = pthread_create(&_listeningThread,
                                NULL,
                                listeningThreadCallback,
                                reinterpret_cast<void*>(this));
        if (rc != 0)
        {
            _logger.error("pthread_create() failed: %s", strerror(rc));
            ::close(_cancelFd);
            _cancelFd = -1;
            return SPI_ERROR;
        }

        power(FULL);

        return SPI_OK;
    }

    void close()
    {
        if (! isOpen())
            return;

        // interrupt our background thread by writing into the event fd
        uint64_t increment = 1;
        ssize_t rc = write(_cancelFd,
                           reinterpret_cast<void*>(&increment),
                           sizeof(increment));
        if (rc < 0)
            _logger.warn("write() failed while closing: %s", strerror(errno));

        pthread_join(_listeningThread, NULL);
        _listeningThread = 0;

        ::close(_cancelFd);
        _cancelFd = -1;

        power(POWER_SAVING);
    }

    void startScan()
    {
        assert(isOpen());

        _logger.debug("starting scan");

        int rc;
        nl_msg* msg = prepareMessage(NL80211_CMD_TRIGGER_SCAN, 0);
        if (! msg)
        {
            _logger.error("prepareMessage(NL80211_CMD_TRIGGER_SCAN) failed");
            return;
        }

        // use wildcard ssid to scan all aps
        nl_msg* ssids = nlmsg_alloc();
        if (! ssids)
        {
            _logger.error("nlmsg_alloc() failed");
            nlmsg_free(msg);
            return;
        }

        rc = nla_put(ssids, 1, 0, "");
        if (rc < 0)
        {
            _logger.error("nla_put() failed: %s", nl_geterror(rc));
            nlmsg_free(msg);
            nlmsg_free(ssids);
            return;
        }

        rc = nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids);
        if (rc < 0)
        {
            _logger.error("nla_put_nested() failed: %s", nl_geterror(rc));
            nlmsg_free(msg);
            nlmsg_free(ssids);
            return;
        }

        if (! doRequest(msg, NULL, NULL))
        {
            _logger.error("failed to start scan");
            _listener->onScanFailed(SPI_ERROR);
            return;
        }

        _logger.debug("scan started");
    }

    ErrorCode getConnectedMAC(MAC& mac)
    {
        if (! isInitialized())
            return SPI_ERROR;

        // Note that this may block for a couple of seconds
        // if a background scan is in progress.
        ErrorCode rc = getScan(parseConnectedAp, &mac);
        if (rc != SPI_OK)
            return rc;

        if (mac == MAC())
        {
            _logger.debug("not connected");
            return SPI_ERROR_NOT_READY;
        }

        if (_logger.isDebugEnabled())
            _logger.debug("connected to %s", mac.toString().c_str());

        return SPI_OK;
    }

    ErrorCode getHardwareMAC(MAC& mac)
    {
        if (! isInitialized())
            return SPI_ERROR;

        rtnl_link* link = getWifiLink();
        if (! link)
            return SPI_ERROR;

        nl_addr* addr = rtnl_link_get_addr(link);
        if (! addr)
        {
            _logger.error("rtnl_link_get_addr() failed");
            rtnl_link_put(link);
            return SPI_ERROR;
        }

        mac = toMAC(nl_addr_get_binary_addr(addr));
        rtnl_link_put(link);

        if (_logger.isDebugEnabled())
            _logger.debug("hardware mac: %s", mac.toString().c_str());

        return SPI_OK;
    }

    ErrorCode power(PowerState powerState)
    {
        if (! isInitialized())
            return SPI_ERROR;

        if (_logger.isDebugEnabled())
            _logger.debug("power request: %s",
                          powerState == FULL ? "full"
                                             : "power saving");

        MAC mac;
        if (getConnectedMAC(mac) == SPI_OK)
        {
            _logger.info("not changing power state of the interface because it is associated");
            return SPI_OK;
        }

        ErrorCode code = SPI_ERROR;
        rtnl_link* link = NULL;
        rtnl_link* request = NULL;
        bool isUp;
        const bool bringUp = (powerState == FULL);
        int rc;

        link = getWifiLink();
        if (! link)
            goto cleanup;

        request = rtnl_link_alloc();
        if (! request)
        {
            _logger.error("rtnl_link_alloc() failed");
            goto cleanup;
        }

        isUp = rtnl_link_get_flags(link) & IFF_UP;

        if (_logger.isDebugEnabled())
            _logger.debug("interface is %s", isUp ? "up" : "down");

        if (isUp == bringUp)  // all set
        {
            code = SPI_OK;
            goto cleanup;
        }

        if (! bringUp && ! _shouldBringDown)
        {
            _logger.debug("not bringing interface down since we didn't bring it up");
            goto cleanup;
        }

        if (_logger.isDebugEnabled())
            _logger.debug("bringing interface %s", bringUp ? "up" : "down");

        if (bringUp)
            rtnl_link_set_flags(request, IFF_UP);
        else
            rtnl_link_unset_flags(request, IFF_UP);

        rc = rtnl_link_change(_routeSock, link, request, 0);
        if (rc < 0)
        {
            _logger.error("rtnl_link_change() failed: %s", nl_geterror(rc));
            goto cleanup;
        }
        
        code = SPI_OK;
        _shouldBringDown = bringUp;

cleanup:
        if (link)
            rtnl_link_put(link);
        if (request)
            rtnl_link_put(request);
        return code;
     }

private:

    // Initialization

    void init()
    {
        assert(_nl80211Sock == NULL && _routeSock == NULL);

        int rc;

        _nl80211Sock = nl_socket_alloc();
        _routeSock = nl_socket_alloc();

        if (! _nl80211Sock || ! _routeSock)
        {
            _logger.error("nl_socket_alloc() failed");
            deinit();
            return;
        }

        rc = genl_connect(_nl80211Sock);
        if (rc < 0)
        {
            _logger.error("genl_connect(_nl80211Sock) failed: %s",
                          nl_geterror(rc));
            deinit();
            return;
        }

        _familyId = genl_ctrl_resolve(_nl80211Sock, "nl80211");
        if (_familyId < 0)
        {
            _logger.error("genl_ctrl_resolve() failed: %s",
                          nl_geterror(_familyId));
            deinit();
            return;
        }

        rc = nl_connect(_routeSock, NETLINK_ROUTE);
        if (rc < 0)
        {
            _logger.error("nl_connect(_routeSock) failed: %s",
                          nl_geterror(rc));
            deinit();
            return;
        }
    }

    void deinit()
    {
        if (_nl80211Sock)
        {
            nl_socket_free(_nl80211Sock);
            _nl80211Sock = NULL;
        }

        if (_routeSock)
        {
            nl_socket_free(_routeSock);
            _routeSock = NULL;
        }
    }

    bool isInitialized() const
    {
        return _nl80211Sock != NULL && _routeSock != NULL;
    }

    bool isOpen() const
    {
        return _listeningThread != 0;
    }

    // NetLink request

    nl_msg* prepareMessage(uint8_t cmd, int flags)
    {
        int rc;
        nl_msg* result = nlmsg_alloc();
        if (! result)
        {
            _logger.error("nlmsg_alloc() failed");
            return NULL;
        }

        if (! genlmsg_put(result, 0, 0, _familyId, 0, flags, cmd, 0))
        {
            nlmsg_free(result);
            _logger.error("genlmsg_put() failed");
            return NULL;
        }

        rc = nla_put_u32(result, NL80211_ATTR_IFINDEX, _ifindex);
        if (rc < 0)
        {
            nlmsg_free(result);
            _logger.error("nla_put_u32() failed: %s", nl_geterror(rc));
            return NULL;
        }

        return result;
    }

    bool doRequest(nl_msg* msg, nl_recvmsg_msg_cb_t handler, void* arg)
    {
        int rc = nl_send_auto_complete(_nl80211Sock, msg);
        if (rc < 0)
        {
            _logger.error("nl_send_auto_complete() failed: %s",
                          nl_geterror(rc));
            return false;
        }

        nl_cb* cb = nl_cb_alloc(NL_CB_DEFAULT);
        if (! cb)
        {
            _logger.error("nl_cb_alloc() failed");
            return false;
        }

        if (handler)
            nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handler, arg);

        rc = nl_recvmsgs(_nl80211Sock, cb);
        nl_cb_put(cb);

        if (rc < 0)
        {
            _logger.error("nl_recvmsgs() failed: %s", nl_geterror(rc));
            return false;
        }

        return true;
    }

    // BSS IE parsing routine

    typedef bool (*IeHandler)(unsigned char type,
                              unsigned char size,
                              unsigned char* data,
                              void* arg);

    static void parseBssIe(nlattr* ie, IeHandler handler, void* arg)
    {
        int totalSize = nla_len(ie);
        unsigned char* p0 = static_cast<unsigned char*>(nla_data(ie));
        unsigned char* p = p0;

        while (p - p0 < totalSize)
        {
            // BSS Information Element:
            //   0x00: type (unsigned char)
            //   0x01: size (unsigned char)
            //   0x02: data

            const unsigned char type = *p++;
            const unsigned char size = *p++;

            if (! handler(type, size, p, arg))
                break;

            p += size;
        }
    }

    static bool ssidIeHandler(unsigned char type,
                              unsigned char size,
                              unsigned char* data,
                              void* arg)
    {
        // Skip a non-SSID type
        if (type != 0x00)
            return true;  // continue

        ScannedAccessPoint::SSID* ssid =
            reinterpret_cast<ScannedAccessPoint::SSID*>(arg);

        ssid->assign(data, data + size);
        return false;  // stop
    }

    // Scan result parsing routines

    typedef std::pair<ScannedAccessPoint, bool> AP;  // bool for the "associated" flag
    typedef std::vector<ScannedAccessPoint> Scan;

    static MAC toMAC(void* data)
    {
        MAC::raw_type mac;
        unsigned char* p = reinterpret_cast<unsigned char*>(data);
        std::reverse_copy(p, p + 6, mac);
        return MAC(mac);
    }

    static int parseConnectedAp(nl_msg* msg, void* arg)
    {
        std::auto_ptr<AP> ap;

        if (! parseAccessPoint(msg, ap))
            return NL_SKIP;

        if (! ap->second)  // not connected
            return NL_SKIP;

        MAC* mac = reinterpret_cast<MAC*>(arg);
        *mac = ap->first.getMAC();
        return NL_OK;  // we need to parse all messages so don't return NL_STOP
    }

    static int parseScannedAp(nl_msg* msg, void* arg)
    {
        std::auto_ptr<AP> ap;

        if (! parseAccessPoint(msg, ap))
            return NL_SKIP;

        Scan* scan = reinterpret_cast<Scan*>(arg);
        scan->push_back(ap->first);
        return NL_OK;
    }

    static bool parseAccessPoint(nl_msg* msg, std::auto_ptr<AP>& ap)
    {
        Logger logger(WPS_LOG_CATEGORY ".parseAccessPoint");

        genlmsghdr* hdr =
            reinterpret_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(msg)));

        if (hdr->cmd != NL80211_CMD_NEW_SCAN_RESULTS)
            return false;

        int rc;
        nlattr* tb[NL80211_ATTR_MAX + 1];
        nlattr* bss[NL80211_BSS_MAX + 1];
        static nla_policy bssPolicy[NL80211_BSS_MAX + 1] = {
            { },         // ---
            { },         //NL80211_BSS_BSSID
            { NLA_U32 }, //NL80211_BSS_FREQUENCY
            { NLA_U64 }, //NL80211_BSS_TSF,
            { NLA_U16 }, //NL80211_BSS_BEACON_INTERVAL,
            { NLA_U16 }, //NL80211_BSS_CAPABILITY,
            { },         //NL80211_BSS_INFORMATION_ELEMENTS,
            { NLA_U32 }, //NL80211_BSS_SIGNAL_MBM,
            { NLA_U8 },  //NL80211_BSS_SIGNAL_UNSPEC,
            { NLA_U32 }, //NL80211_BSS_STATUS,
            { NLA_U32 }, //NL80211_BSS_SEEN_MS_AGO,
            { },         //NL80211_BSS_BEACON_IES,
        };

        rc = nla_parse(tb,
                       NL80211_ATTR_MAX,
                       genlmsg_attrdata(hdr, 0),
                       genlmsg_attrlen(hdr, 0),
                       NULL);
        if (rc < 0)
        {
            logger.error("nla_parse() failed: %s", nl_geterror(rc));
            return false;
        }

        if (! tb[NL80211_ATTR_BSS])
        {
            logger.error("NL80211_ATTR_BSS was not found in the netlink message");
            return false;
        }

        rc = nla_parse_nested(bss,
                              NL80211_BSS_MAX,
                              tb[NL80211_ATTR_BSS],
                              bssPolicy);
        if (rc < 0)
        {
            logger.error("nla_parse_nested() failed: %s", nl_geterror(rc));
            return false;
        }

        if (! bss[NL80211_BSS_BSSID]
                || ! bss[NL80211_BSS_CAPABILITY]
                || ! bss[NL80211_BSS_SEEN_MS_AGO]
                || ! bss[NL80211_BSS_SIGNAL_MBM])
        {
            logger.error("some of the BSS attributes are missing");
            return false;
        }

        // Skip ad-hoc points
        unsigned int capability = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
        if (capability & WLAN_CAPABILITY_IBSS)
            return false;

        const MAC mac = toMAC(nla_data(bss[NL80211_BSS_BSSID]));

        Timer age;
        age.reset(nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]));

        // Search for SSID in Wi-Fi Information Elements
        ScannedAccessPoint::SSID ssid;
        parseBssIe(bss[NL80211_BSS_INFORMATION_ELEMENTS],
                   ssidIeHandler,
                   &ssid);

        // Signal value is signed while being encoded into uint32_t.
        // Also it's in mBm, so we convert it into dBm.
        const int signal =
            static_cast<int32_t>(nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM])) / 100;

        nlattr* nlStatus = bss[NL80211_BSS_STATUS];
        const int status =
            nlStatus ? static_cast<int>(nla_get_u32(nlStatus))
                     : -1;

        if (logger.isDebugEnabled())
            logger.debug("scanned AP %s %d %lums (status: %d)",
                         MAC(mac).toString().c_str(),
                         signal,
                         age.elapsed(),
                         status);

        ap.reset(new AP(ScannedAccessPoint(mac, signal, age, ssid),
                        status == NL80211_BSS_STATUS_ASSOCIATED));
        return true;
    }

    // NL80211_CMD_GET_SCAN wrapper

    ErrorCode getScan(nl_recvmsg_msg_cb_t handler, void* arg)
    {
        nl_msg* msg = prepareMessage(NL80211_CMD_GET_SCAN, NLM_F_DUMP);
        if (! msg)
        {
            _logger.error("prepareMessage(NL80211_CMD_GET_SCAN) failed");
            return SPI_ERROR;
        }

        if (! doRequest(msg, handler, arg))
            return SPI_ERROR;

        return SPI_OK;
    }

    // Scan listening thread

    void onScanCompleted()
    {
        assert(_listener != NULL);

        _logger.debug("scan completed");

        std::vector<ScannedAccessPoint> scan;
        ErrorCode code = getScan(parseScannedAp, &scan);
        if (code == SPI_OK)
            _listener->onScanCompleted(scan);
        else
            _listener->onScanFailed(code);
    }

    static int parseEvent(nl_msg* msg, void* arg)
    {
        Nl80211WifiAdapter* _this = reinterpret_cast<Nl80211WifiAdapter*>(arg);
        genlmsghdr* hdr = reinterpret_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(msg)));

        if (hdr->cmd != NL80211_CMD_NEW_SCAN_RESULTS)
            return NL_SKIP;

        _this->onScanCompleted();
        return NL_OK;
    }

    static int removeSeqCheck(struct nl_msg* msg, void* arg)
    {
        return NL_OK;
    }

    void listeningThread()
    {
        _logger.debug("listening thread started");

        int rc, groupId;
        nl_sock* eventSock = nl_socket_alloc();
        if (! eventSock)
        {
            _logger.error("nl_socket_alloc failed()");
            return;
        }

        rc = genl_connect(eventSock);
        if (rc < 0)
        {
            _logger.error("genl_connect(eventSock) failed: %s",
                          nl_geterror(rc));
            goto cleanup;
        }

        // register on the "scan" multicast group
        groupId = genl_ctrl_resolve_grp(eventSock, "nl80211", "scan");
        if (groupId < 0)
        {
            _logger.error("genl_ctrl_resolve_grp() failed: %s",
                          nl_geterror(groupId));
            goto cleanup;
        }

        rc = nl_socket_add_membership(eventSock, groupId);
        if (rc < 0)
        {
            _logger.error("nl_socket_add_membership() failed: %s",
                          nl_geterror(rc));
            goto cleanup;
        }

        // remove the sequence number checking function by replacing with a stub
        rc = nl_socket_modify_cb(eventSock,
                                 NL_CB_SEQ_CHECK,
                                 NL_CB_CUSTOM,
                                 removeSeqCheck,
                                 NULL);
        if (rc < 0)
        {
            _logger.error("nl_socket_modify_cb(removeSeqCheck) failed: %s",
                          nl_geterror(rc));
            goto cleanup;
        }

        rc = nl_socket_modify_cb(eventSock,
                                 NL_CB_VALID,
                                 NL_CB_CUSTOM,
                                 parseEvent,
                                 reinterpret_cast<void*>(this));
        if (rc < 0)
        {
            _logger.error("nl_socket_modify_cb(parseEvent) failed: %s",
                          nl_geterror(rc));
            goto cleanup;
        }

        while (true)
        {
            struct pollfd fds[2];
            pollfd* pfdNetLink = &fds[0];
            pollfd* pfdCancel = &fds[1];

            pfdNetLink->fd = nl_socket_get_fd(eventSock);
            pfdNetLink->events = POLLIN | POLLRDHUP;
            pfdCancel->fd = _cancelFd;
            pfdCancel->events = POLLIN | POLLRDHUP;

            int rc = poll(fds, 2, -1);
            if (rc < 0)
            {
                _logger.debug("poll() failed: %s", strerror(errno));
                break;
            }

            // check if a thread cancel was requested
            if ((pfdCancel->revents & (POLLIN | POLLRDHUP)) != 0)
            {
                _logger.debug("listening thread cancelled");
                break;
            }

            rc = nl_recvmsgs_default(eventSock);
            if (rc < 0)
            {
                _logger.error("nl_recvmsgs_default(eventSock) failed: %s",
                              nl_geterror(rc));
                break;
            }
        }

cleanup:
        nl_socket_free(eventSock);
    }

    static void* listeningThreadCallback(void* arg)
    {
        Nl80211WifiAdapter* _this = reinterpret_cast<Nl80211WifiAdapter*>(arg);
        _this->listeningThread();
        return NULL;
    }

    // Wifi route netlink

    rtnl_link* getWifiLink() const
    {
        rtnl_link* link;
        int rc;

#if defined(LIBNL_VER_MAJ) && defined(LIBNL_VER_MIN) && LIBNL_VER_MAJ == 3 && LIBNL_VER_MIN >= 1
        rc = rtnl_link_get_kernel(_routeSock, _ifindex, _ifname.c_str(), &link);
        if (rc < 0)
        {
            _logger.error("rtnl_link_get_kernel() failed: %s",
                          nl_geterror(rc));
            return 0;
        }
#else
        nl_cache* cache;

#  if defined(LIBNL_VER_MAJ) && defined(LIBNL_VER_MIN) && LIBNL_VER_MAJ == 3 && LIBNL_VER_MIN == 0
        rc = rtnl_link_alloc_cache(_routeSock, AF_UNSPEC, &cache);
#  else
        rc = rtnl_link_alloc_cache(_routeSock, &cache);
#  endif
        if (rc < 0)
        {
            _logger.error("rtnl_link_alloc_cache() failed: %s",
                          nl_geterror(rc));
            return 0;
        }

        link = rtnl_link_get_by_name(cache, _ifname.c_str());
        if (! link)
            _logger.error("rtnl_link_get_by_name() failed");

        nl_cache_free(cache);
#endif

        return link;
    }

private:

    // not implemented
    Nl80211WifiAdapter(const Nl80211WifiAdapter&);
    Nl80211WifiAdapter& operator=(const Nl80211WifiAdapter&);

private:

    Logger _logger;
    Listener* _listener;
    unsigned int _ifindex;
    std::string _ifname;
    int _familyId;
    nl_sock* _nl80211Sock;  // for nl80211 communication
    nl_sock* _routeSock;  // for route netlink communication
    pthread_t _listeningThread;
    int _cancelFd;
    bool _shouldBringDown;
};

/**********************************************************************/
/*                                                                    */
/* WifiAdapter::newInstance                                           */
/*                                                                    */
/**********************************************************************/

typedef std::pair<std::string, int> IfNameIndex;

static int
getInterfaceCallback(nl_msg* msg, void* arg)
{
    Logger logger(WPS_LOG_CATEGORY ".getInterfaceCallback");

    genlmsghdr* genlMsgHeader =
        reinterpret_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(msg)));

    if (genlMsgHeader->cmd != NL80211_CMD_NEW_INTERFACE)
        return NL_SKIP;

    struct nlattr* tb[NL80211_ATTR_MAX + 1];
    int rc = nla_parse(tb,
                       NL80211_ATTR_MAX,
                       genlmsg_attrdata(genlMsgHeader, 0),
                       genlmsg_attrlen(genlMsgHeader, 0),
                       NULL);
    if (rc < 0)
    {
        logger.error("nla_parse() failed: %s", nl_geterror(rc));
        return NL_SKIP;
    }

    IfNameIndex* ifNameIndex = reinterpret_cast<IfNameIndex*>(arg);
    ifNameIndex->first = nla_get_string(tb[NL80211_ATTR_IFNAME]);
    ifNameIndex->second = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);

    const std::string& name = ifNameIndex->first;
    if (name.find("p2p", 0) != std::string::npos)
    {
        logger.debug("skipping wifi direct interface: %s", name.c_str());
        return NL_SKIP;
    }

    if (logger.isDebugEnabled())
        logger.debug("found wifi interface: %s", name.c_str());

    return NL_STOP;
}

WifiAdapter*
WifiAdapter::newInstance()
{
    Logger logger(WPS_LOG_CATEGORY ".newInstance");

    IfNameIndex ifNameIndex("", -1);

    int rc, familyId;
    nl_sock* sock = NULL;
    nl_msg* msg = NULL;

    sock = nl_socket_alloc();
    if (! sock)
    {
        logger.error("nl_socket_alloc() failed");
        goto error;
    }

    rc = genl_connect(sock);
    if (rc < 0)
    {
        logger.error("genl_connect() failed: %s", nl_geterror(rc));
        goto error;
    }

    familyId = genl_ctrl_resolve(sock, "nl80211");
    if (familyId < 0)
    {
        logger.error("genl_ctrl_resolve() failed");
        goto error;
    }

    msg = nlmsg_alloc();
    if (! msg)
    {
        logger.error("nlmsg_alloc() failed");
        goto error;
    }

    if (! genlmsg_put(msg, 0, 0, familyId, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0))
    {
        logger.error("genlmsg_put() failed");
        goto error;
    }

    rc = nl_socket_modify_cb(sock,
                             NL_CB_VALID,
                             NL_CB_CUSTOM,
                             getInterfaceCallback,
                             &ifNameIndex);
    if (rc < 0)
    {
        logger.error("nl_socket_modify_cb() failed: %s", nl_geterror(rc));
        goto error;
    }

    rc = nl_send_auto_complete(sock, msg);
    if (rc < 0)
    {
        logger.error("nl_send_auto_complete() failed: %s", nl_geterror(rc));
        goto error;
    }

    rc = nl_recvmsgs_default(sock);
    if (rc < 0)
    {
        logger.error("nl_recvmsgs_default() failed: %s", nl_geterror(rc));
        goto error;
    }

    nl_socket_free(sock);

    if (ifNameIndex.second == -1)
        return NULL;

    return new Nl80211WifiAdapter(ifNameIndex.first, ifNameIndex.second);

error:
    if (msg)
        nlmsg_free(msg);
    if (sock)
        nl_socket_free(sock);
    return NULL;
}

}
}
