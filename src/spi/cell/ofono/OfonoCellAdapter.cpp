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

#include "spi/CellAdapter.h"
#include "spi/Logger.h"
#include "spi/StdLibC.h"
#include "spi/Concurrent.h"

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus.h>

#include "GlibCellWrapper.h"

#include "ofono_dbus_marshalling.h"

#define DBUS_HASHTABLE_STRING_VALUE \
        dbus_g_type_get_map        ("GHashTable", \
                                    G_TYPE_STRING, \
                                    G_TYPE_VALUE)

#define DBUS_STRUCT_OBJECT_HASHTABLE \
        dbus_g_type_get_struct     ("GValueArray", \
                                    DBUS_TYPE_G_OBJECT_PATH, \
                                    DBUS_HASHTABLE_STRING_VALUE, \
                                    G_TYPE_INVALID)

#define DBUS_ARRAY_MODEMS \
        dbus_g_type_get_collection ("GPtrArray", \
                                    DBUS_STRUCT_OBJECT_HASHTABLE)

#define OFONO_BUS_NAME             "org.ofono"
#define OFONO_MANAGER_INTERFACE    "org.ofono.Manager"
#define OFONO_NETWORKREG_INTERFACE "org.ofono.NetworkRegistration"
#define OFONO_MODEM_INTERFACE      "org.ofono.Modem"
#define OFONO_SIGNAL_NAME          "PropertyChanged"

namespace WPS {
namespace SPI {

/**********************************************************************/
/*                                                                    */
/* OfonoCellAdapter                                                   */
/*                                                                    */
/**********************************************************************/

class OfonoCellAdapter
    : public CellAdapter
{
public:

    OfonoCellAdapter()
        : _logger("WPS.SPI.OfonoCellAdapter")
        , _connection(NULL)
        , _proxy(NULL)
        , _listener(NULL)
    {}

    ~OfonoCellAdapter()
    {
        close();
    }

    std::string description() const
    {
        return _currentModem;
    }

    void setListener(Listener* listener)
    {
        assert(_connection == NULL);
        _listener = listener;
    }

    ErrorCode open()
    {
        assert(_listener != NULL);

        if (_connection != NULL)
            return SPI_OK;

        GError* error = NULL;
        _connection = dbus_g_bus_get_private(DBUS_BUS_SYSTEM,
                                             g_main_context_get_thread_default(),
                                             &error);
        if (_connection == NULL)
        {
            _logger.error("unable to connect to dbus: %s", error->message);
            g_error_free(error);
            return SPI_ERROR;
        }

        _currentModem = getCurrentModem();
        if (_currentModem.empty())
        {
            _logger.debug("no online modems found");
            close();
            return SPI_ERROR;
        }

        if (_proxy != NULL)
            g_object_unref(_proxy);

        dbus_g_object_register_marshaller(
            g_cclosure_user_marshal_VOID__STRING_VARIANT,
            G_TYPE_NONE,
            G_TYPE_STRING,
            G_TYPE_VALUE,
            G_TYPE_INVALID);

        _proxy = dbus_g_proxy_new_for_name(_connection,
                                           OFONO_BUS_NAME,
                                           _currentModem.c_str(),
                                           OFONO_NETWORKREG_INTERFACE);

        dbus_g_proxy_add_signal(_proxy,
                                "PropertyChanged",
                                G_TYPE_STRING,
                                G_TYPE_VALUE,
                                G_TYPE_INVALID);

        reportCell();

        dbus_g_proxy_connect_signal(_proxy,
                                    OFONO_SIGNAL_NAME,
                                    G_CALLBACK(propertyChangedCallback),
                                    reinterpret_cast<void*>(this),
                                    NULL);

        return SPI_OK;
    }

    void close()
    {
        if (_proxy != NULL)
        {
            g_object_unref(_proxy);
            _proxy = NULL;
        }

        if (_connection != NULL)
        {
            dbus_connection_close(dbus_g_connection_get_connection(_connection));
            dbus_g_connection_unref(_connection);
            _connection = NULL;
        }
    }

    ErrorCode getIMEI(std::string& imei)
    {
        if (! _imei.empty())
        {
            imei = _imei;
            return SPI_OK;
        }

        bool needsClosing = false;
        if (_connection == NULL)
        {
            const ErrorCode rc = open();
            if (rc != SPI_OK)
                return rc;

            needsClosing = true;
        }

        DBusGProxy* modemProxy =
            dbus_g_proxy_new_for_name(_connection,
                                      OFONO_BUS_NAME,
                                      _currentModem.c_str(),
                                      OFONO_MODEM_INTERFACE);

        ErrorCode rc = SPI_ERROR;
        GValue* imeiVal = NULL;
        GError* error = NULL;
        GHashTable* modemProperties = NULL;

        if (! dbus_g_proxy_call(modemProxy,
                                "GetProperties",
                                &error,
                                G_TYPE_INVALID,
                                DBUS_HASHTABLE_STRING_VALUE,
                                &modemProperties,
                                G_TYPE_INVALID))
        {
            _logger.error(OFONO_MODEM_INTERFACE ".GetProperties failed: %s", error->message);
            g_error_free(error);
            goto exit;
        }

        if (modemProperties == NULL)
        {
            _logger.warn(OFONO_MODEM_INTERFACE ".GetProperties returned NULL");
            goto exit;
        }

        imeiVal = (GValue*) g_hash_table_lookup(modemProperties, "Serial");

        if (imeiVal != NULL)
        {
            imei = _imei = g_value_get_string(imeiVal);
            rc = SPI_OK;
        }

        g_hash_table_unref(modemProperties);

exit:
        g_object_unref(modemProxy);
        if (needsClosing)
            close();

        return rc;
    }

private:

    CellTower::CellTowerType toCellType(GValue* techVal) const
    {
        if (techVal == NULL)
        {
            _logger.error("cell technology value is null");
            return CellTower::UNKNOWN;
        }

        const gchar* techStr = g_value_get_string(techVal);

        if (g_strcmp0(techStr, "gsm") == 0 || g_strcmp0(techStr, "edge") == 0)
            return CellTower::GSM;

        if (g_strcmp0(techStr, "umts") == 0 || g_strcmp0(techStr, "hspa") == 0)
            return CellTower::UMTS;

        if (g_strcmp0(techStr, "lte") == 0)
            return CellTower::LTE;

        _logger.warn("unknown cell tower type: %s", techStr);
        return CellTower::UNKNOWN;
    }

    std::string getCurrentModem()
    {
        GError* error = NULL;
        GPtrArray* modems = NULL;

        DBusGProxy* managerProxy =
            dbus_g_proxy_new_for_name(_connection,
                                      OFONO_BUS_NAME,
                                      "/",
                                      OFONO_MANAGER_INTERFACE);

        bool rc = dbus_g_proxy_call(managerProxy,
                                    "GetModems",
                                    &error,
                                    G_TYPE_INVALID,
                                    DBUS_ARRAY_MODEMS,
                                    &modems,
                                    G_TYPE_INVALID);

        g_object_unref(managerProxy);

        if (! rc)
        {
            _logger.error(OFONO_MANAGER_INTERFACE ".GetModems failed: %s",
                          error->message);
            g_error_free(error);
            return "";
        }

        if (modems == NULL)
        {
            _logger.error("received null modem list");
            return "";
        }

        std::string currentModem;
        for (int i = 0; i < modems->len; ++i)
        {
            GValue* value;
            char* name;
            GHashTable* properties;

            value = g_new0(GValue, 1);
            g_value_init(value, DBUS_STRUCT_OBJECT_HASHTABLE);
            g_value_take_boxed(value, g_ptr_array_index(modems, i));

            dbus_g_type_struct_get(value, 0, &name, 1, &properties, G_MAXUINT);
            g_free(value);

            gboolean online =
                g_value_get_boolean((GValue*) g_hash_table_lookup(properties, "Online"));

            g_hash_table_unref(properties);

            if (_logger.isDebugEnabled())
                _logger.debug("found modem: %s (%s)",
                              name,
                              online ? "online" : "offline");

            if (online)
                currentModem = name;

            g_free(name);

            if (online)
                break;
        }

        for (int i = 0; i < modems->len; ++i)
            g_value_array_free(reinterpret_cast<GValueArray*>(g_ptr_array_index(modems, i)));

        g_ptr_array_free(modems, true);
        return currentModem;
    }

    ErrorCode getScannedCellTower(std::auto_ptr<ScannedCellTower>& scannedCell)
    {
        scannedCell.reset(NULL);

        GError* error = NULL;
        GHashTable* networkProperties = NULL;

        bool rc = dbus_g_proxy_call(_proxy,
                                    "GetProperties",
                                    &error,
                                    G_TYPE_INVALID,
                                    DBUS_HASHTABLE_STRING_VALUE,
                                    &networkProperties,
                                    G_TYPE_INVALID);
        if (! rc)
        {
            _logger.error(OFONO_NETWORKREG_INTERFACE ".GetProperties failed: %s",
                          error->message);
            g_error_free(error);
            return SPI_ERROR;
        }

        if (networkProperties == NULL)
        {
            _logger.warn(OFONO_NETWORKREG_INTERFACE ".GetProperties returned null");
            return SPI_ERROR;
        }

        GValue* techVal   = (GValue*) g_hash_table_lookup(networkProperties, "Technology");
        GValue* mccVal    = (GValue*) g_hash_table_lookup(networkProperties, "MobileCountryCode");
        GValue* mncVal    = (GValue*) g_hash_table_lookup(networkProperties, "MobileNetworkCode");
        GValue* cellIdVal = (GValue*) g_hash_table_lookup(networkProperties, "CellId");
        GValue* lacVal    = (GValue*) g_hash_table_lookup(networkProperties, "LocationAreaCode");
        GValue* signalVal = (GValue*) g_hash_table_lookup(networkProperties, "Strength");

        if (mccVal           != NULL
                && mncVal    != NULL
                && cellIdVal != NULL
                && signalVal != NULL)
        {
            CellTower tower;
            const unsigned short mcc = atoi(g_value_get_string(mccVal));
            const unsigned short mnc = atoi(g_value_get_string(mncVal));
            const int ci = g_value_get_uint(cellIdVal);
            const int lac = lacVal == NULL ? -1
                                           : g_value_get_uint(lacVal);

            if (_logger.isDebugEnabled())
                _logger.debug("received cell values: mcc=%u mnc=%u ci=%d lac=%d",
                              mcc, mnc, ci, lac);

            switch (toCellType(techVal))
            {
            case CellTower::GSM:
                tower = CellTower::GSMTower(mcc, mnc, ci, lac);
                break;

            case CellTower::UMTS:
                tower = CellTower::UMTSTower(mcc, mnc, ci, lac);
                break;

            case CellTower::LTE:
                tower = CellTower::LTETower(mcc, mnc, ci, lac);
                break;
#ifdef WPS_OFONO_DEFAULT_TO_GSM
            default:
                _logger.debug("defaulting to GSM");
                tower = CellTower::GSMTower(mcc, mnc, ci, lac);
                break;
#endif
            }

            if (tower)
            {
                scannedCell.reset(
                    new ScannedCellTower(tower,
                                         0,
                                         g_value_get_uchar(signalVal) - 110));
            }
        }
#ifndef NDEBUG
        else if (_logger.isDebugEnabled())
        {
            _logger.debug("no valid network information");

            if (mccVal == NULL)
                _logger.debug("mccVal is null");
            if (mncVal == NULL)
                _logger.debug("mncVal is null");
            if (cellIdVal == NULL)
                _logger.debug("cellIdVal is null");
            if (lacVal == NULL)
                _logger.debug("lacVal is null");
            if (signalVal == NULL)
                _logger.debug("signalVal is null");
        }
#endif

        g_hash_table_unref(networkProperties);

        return SPI_OK;
    }

    void reportCell()
    {
        std::auto_ptr<ScannedCellTower> scannedCell;
        std::vector<ScannedCellTower> scannedCells;
        ErrorCode rc = getScannedCellTower(scannedCell);

        if (rc != SPI_OK)
        {
            _listener->onCellError(rc);
            return;
        }

        if (! scannedCell.get())
        {
            if (_lastCell.get())
            {
                _logger.debug("lost main cell");
                _listener->onCellChanged(scannedCells);
                _lastCell.reset(NULL);
            }

            _logger.debug("not connected to cell");
            return;
        }

        if (_lastCell.get()
                 && _lastCell->getCell() == scannedCell->getCell()
                 && _lastCell->getRssi() == scannedCell->getRssi())
        {
            if (_logger.isDebugEnabled())
                _logger.debug("not reporting cached cell: %s",
                              scannedCell->toString().c_str());
            return;
        }

        if (_logger.isDebugEnabled())
            _logger.debug("reporting new scanned cell: %s",
                          scannedCell->toString().c_str());

        scannedCells.push_back(*scannedCell);
        _listener->onCellChanged(scannedCells);

        _lastCell = scannedCell;
    }

    static void propertyChangedCallback(DBusGProxy* proxy,
                                        const char* propertyName,
                                        GValue* value,
                                        OfonoCellAdapter* self)
    {
        Logger& logger = self->_logger;
        if (logger.isDebugEnabled())
            logger.debug("propertyChangedCallback: %s", propertyName);

        self->reportCell();
    }

private:

    Logger _logger;
    DBusGConnection* _connection;
    DBusGProxy* _proxy;
    std::string _currentModem;
    std::string _imei;

    Listener* _listener;
    std::auto_ptr<ScannedCellTower> _lastCell;
};


/**********************************************************************/
/*                                                                    */
/* CellAdapter::newInstance                                           */
/*                                                                    */
/**********************************************************************/

CellAdapter*
CellAdapter::newInstance()
{
    return new GlibCellWrapper(new OfonoCellAdapter);
}

}
}
