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

#ifndef WPS_SPI_CELL_TOWER_H_
#define WPS_SPI_CELL_TOWER_H_

#include <string>

#include "spi/StdLibC.h"
#include "spi/Assert.h"

namespace WPS {
namespace SPI {

/**
 * \ingroup nonreplaceable
 *
 * Encapsulate a cell tower identification
 *
 * @see http://en.wikipedia.org/wiki/E.164
 *
 * @since WPS API 2.7
 *
 * @author Skyhook Wireless
 */
class CellTower
{
public:

    enum CellTowerType {UNKNOWN, GSM, UMTS, LTE};

    typedef CellTower (*Creator)(unsigned short mcc,
                                 unsigned short mnc,
                                 int ci,
                                 int lacTac);

    static CellTower GSMTower(unsigned short mcc,
                              unsigned short mnc,
                              int ci,
                              int lac)
    {
        if (! isValidGsmUmtsCommon(mcc, mnc, lac)
                || (ci >> 16) != 0)
            return CellTower();
        return CellTower(GSM, mcc, mnc, ci, lac, -1);
    }

    static CellTower UMTSTower(unsigned short mcc,
                               unsigned short mnc,
                               int ci,
                               int lac)
    {
        if (! isValidGsmUmtsCommon(mcc, mnc, lac)
                || (ci >> 28) != 0)
            return CellTower();
        return CellTower(UMTS, mcc, mnc, ci, lac, -1);
    }

    static CellTower LTETower(unsigned short mcc,
                              unsigned short mnc,
                              int ci,
                              int tac)
    {
        if (! isValidMccMnc(mcc, mnc) || (ci >> 28) != 0)
            return CellTower();
        return CellTower(LTE, mcc, mnc, ci, -1, tac);
    }

    CellTower()
        : _type(UNKNOWN), _mcc(0), _mnc(0), _ci(-1), _lac(-1), _tac(-1)
    {}

    CellTower(const CellTower& rhs)
        : _type(rhs._type)
        , _mcc(rhs._mcc)
        , _mnc(rhs._mnc)
        , _ci(rhs._ci)
        , _lac(rhs._lac)
        , _tac(rhs._tac)
    {}

    ~CellTower()
    {}

    CellTower& operator=(const CellTower& rhs)
    {
        if (this != &rhs)
        {
            _type = rhs._type;
            _mcc = rhs._mcc;
            _mnc = rhs._mnc;
            _ci = rhs._ci;
            _lac = rhs._lac;
            _tac = rhs._tac;
        }
        return *this;
    }

    bool operator==(const CellTower& that) const
    {
        return compare(that) == 0;
    }

    bool operator!=(const CellTower& that) const
    {
        return compare(that) != 0;
    }

    bool operator<(const CellTower& that) const
    {
        return compare(that) < 0;
    }

    operator bool() const
    {
        return ! isNull();
    }

    bool isNull() const
    {
        return _type == UNKNOWN;
    }

    /**
     * @return false if lac is unknown
     */
    bool hasLac() const
    {
        return _lac != -1;
    }

    /*
     * @return cell tower type
     */
    CellTowerType getType() const
    {
        return _type;
    }

    /**
     * @return mobile country code.
     */
    unsigned short getMcc() const
    {
        assert(! isNull());
        return _mcc;
    }

    /**
     * @return mobile network code.
     */
    unsigned short getMnc() const
    {
        assert(! isNull());
        return _mnc;
    }

    /**
     * @return cell identifier, or <code>-1</code> if unknown.
     */
    int getCi() const
    {
        assert(! isNull());
        return _ci;
    }

    /**
     * @return local area code, or <code>-1</code> if unknown.
     */
    int getLac() const
    {
        assert(! isNull());
        return _lac;
    }

    /**
     * @return tracking area code, or <code>-1</code> if unknown.
     */
    int getTac() const
    {
        assert(! isNull());
        return _tac;
    }

    int compare(const CellTower& that) const
    {
        assert(! isNull() && ! that.isNull());
        if (const int r = _type - that._type)
            return r;

        if (const int r = _mcc - that._mcc)
            return r;

        if (const int r = _mnc - that._mnc)
            return r;

        // IMPORTANT: It is safe to subtract the values for _ci and _lac
        // because we currently check that they are <= 65535. If the range for
        // them is ever extended, then it needs to be reexamined whether
        // subtraction can still be used.

        if (const int r = _ci - that._ci)
            return r;

        if (_type == LTE)
            return 0;

        return _lac - that._lac;
    }

    /**
     * @return <code>this</code> cell tower as a string.
     */
    std::string toString() const
    {
        char buf[32];
        snprintf(buf,
                 sizeof(buf),
                 "%s-%03u-%03u-%d-%d-%d",
                 toString(_type).c_str(),
                 _mcc,
                 _mnc,
                 _ci,
                 _lac,
                 _tac);
        return buf;
    }

    unsigned long long getCellGlobalId() const
    {
        return getAdjacentCellGlobalId(0);
    }

    unsigned long long getAdjacentCellGlobalId(int delta) const
    {
        return getCellGlobalId(_type, _mcc, _mnc, _lac, getAdjacentCellId(_type, _ci, delta));
    }

private:

    /**
     * @param type Cell tower type
     * @param mcc Mobile Country Code
     * @param mnc Mobile Network Code
     * @param ci Cell ID (-1 if unknown)
     * @param lac Local Area Code (GSM/UMTS only, -1 for LTE)
     * @param tac Tracking Area Code (LTE only, -1 for GSM/UMTS)
     */
    CellTower(CellTowerType type,
              unsigned short mcc,
              unsigned short mnc,
              int ci,
              int lac,
              int tac)
        : _type(type), _mcc(mcc), _mnc(mnc), _ci(ci), _lac(lac), _tac(tac)
    {
        assert(_mcc <= 999);
        assert(_mnc <= 999);
        assert((_type == GSM && _ci <= 65535) || ((_type == LTE || _type == UMTS) && (_ci < (1ULL << 28))));
        assert(_lac <= 65535);
        assert(_tac <= 65535);
    }

    static bool isValidGsmUmtsCommon(int mcc, int mnc, int lac)
    {
        return isValidMccMnc(mcc, mnc) && isValidLac(lac);
    }

    static bool isValidLac(int lac)
    {
        return (lac >> 16) == 0
                   && lac != 0x0000
                   && lac != 0xFFFE;
    }

    static bool isValidMccMnc(unsigned short mcc, unsigned short mnc)
    {
        return ! ((999 < mnc)
                     || (mcc < 200 || (mcc >= 800 && mcc < 900) || 999 < mcc ));
    }

    static int getAdjacentCellId(CellTowerType type, int ci, int delta)
    {
        if (delta == 0)
            return ci;

        assert(type == UMTS);
        return std::max(0, std::min(0xFFFF, (ci & 0xFFFF) + delta)) | (ci & 0xFFF0000);
    }

    static unsigned long long getCellGlobalId(CellTowerType type,
                                              unsigned short mcc,
                                              unsigned short mnc,
                                              int lac,
                                              int ci)
    {
        switch (type)
        {
            case GSM:
                return (unsigned long long) mcc << (16 + 16 + 10)
                    | (unsigned long long) mnc << (16 + 16)
                    | (unsigned long long) lac <<  16
                    | (ci & 0xFFFF);

            case UMTS:
                return (unsigned long long) mcc << (28 + 16 + 10)
                    | (unsigned long long) mnc << (28 + 16)
                    | (unsigned long long) lac <<  28
                    | (ci & 0xFFFFFFF);

            case LTE:
                assert(lac == -1);
                return (unsigned long long) mcc << (28 + 10)
                    | (unsigned long long) mnc << 28
                    | ci;

            case UNKNOWN:
                assert(false);
        }

        assert(false);
        return 0;
    }

    static std::string toString(CellTowerType type)
    {
#   define CASE_LOG(e)  case e: return (#e);
        switch (type)
        {
        CASE_LOG(UMTS)
        CASE_LOG(LTE)
        CASE_LOG(GSM)
        CASE_LOG(UNKNOWN)
        }
        return "";
#   undef CASE_LOG
    }

private:

    CellTowerType _type; // cell tower type
    unsigned short _mcc; // mobile country code
    unsigned short _mnc; // mobile network code
    int _ci; // cell id. -1 if unknown.
    int _lac; // location area code. -1 if unknown.
    int _tac; // tracking area code (LTE) is a 16-bit ID similar to LAC for GSM/UMTS, -1 if unknown
};

}
}

#endif
