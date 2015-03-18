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

#include <nmea/NMEA.h>
#include <nmea/Info.h>
#include <nmea/Units.h>

#include <string>
#include <cmath>

#include "spi/Assert.h"

using namespace WPS::SPI;

inline void assert_delta(double n, double n1, double d = 0.00001)
{
    assert(std::fabs(n - n1) < d);
}

void test_loop_back()
{
    const std::string nmea = "$GPGGA,172724.00,0123.4560,N,00987.6540,W,1,08,1.5,,,,,,0004*45\r\n"
                             "$GPGSA,A,3,01,02,03,04,05,06,07,08,,,,,,1.5,*3E\r\n"
                             "$GPGSV,3,1,09,01,10,020,30,02,11,021,31,03,12,022,32,04,13,023,33*76\r\n"
                             "$GPGSV,3,2,09,05,14,024,34,06,15,025,35,07,16,026,36,08,17,027,37*7D\r\n"
                             "$GPGSV,3,3,09,09,18,028,38,,,,,,,,,,,,*41\r\n"
                             "$GPRMC,172724.00,A,0123.4560,N,00987.6540,W,14.4,25.1,160908,,*23\r\n"
                             "$GPGLL,0123.4560,N,00987.6540,W,172724.00,A*15\r\n";

    NMEA::Info info;
    assert(NMEA::parse(nmea.c_str(), info) == nmea.length());

    assert(info.getSatsInUse() == 8);
    assert(info.getSatsInView() == 9);

    std::string s;
    NMEA::generate(info, s);

    assert(s == nmea);
}

void test_parse_wrong_gsa()
{
    NMEA::Info info;
    NMEA::parse("$GPGSA,A,3,01,02,03,04,05,06,07,08,09,10,11,12,99,3.5,1.5,2.5*18\r\n", info);

    assert(info.getFixType() == NMEA::FIX_TYPE_3D);
    assert_delta(info.getPDOP(), 3.5);
    assert_delta(info.getHDOP(), 1.5);
    assert_delta(info.getVDOP(), 2.5);

    for (int i = 0; i < NMEA::MAX_SAT_IN_USE; ++i)
        assert(info.get(NMEA::SAT_IN_USE_01 + i).operator int() == i + 1);
}

void test_parse_new_rmc()
{
    NMEA::Info info;
    NMEA::parse("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W,A*07\r\n", info);

    assert_delta(NMEA::kph2knots(info.getSpeed()), 22.4);
    assert_delta(info.getDirection(), 84.4);
    assert_delta(info.getDeclination(), -3.1);
}

void test_parse_nan()
{
    // Qualcomm MSM72XX chipsets (Xperia, Touch HD) may emit float parameter as 'nan'.
    // That seems to appear only in VTG sentence for DIRECTION parameter,
    // but we will make sure the library won't be upset in any case.

    NMEA::Info info;
    NMEA::parse("$GPRMC,,,,,,,,nan,,12.3,W*4F\r\n", info);

    assert(!info.isPresent(NMEA::DIRECTION));
    assert_delta(info.getDeclination(), -12.3);
}

void test_parse_incomplete()
{
    NMEA::Info info;

    assert(NMEA::parse("$GPGGA,172724.00,0123.4560", info) == 0);
    assert(NMEA::parse("0123456789", info) == 0);
    assert(NMEA::parse("0123456789$GPGGA)", info) == 0);
    assert(NMEA::parse("0123456789$GPGGA,\r\n", info) == 19);
}

void test_generate_sat()
{
    NMEA::Info info;
    NMEA::SatsInView satsInView;
    NMEA::SatsInUse satsInUse = { 0 };

    for (int i = 0; i < NMEA::MAX_SAT_IN_VIEW; ++i)
    {
        NMEA::Satellite sat;
        sat.prn = i + 1;
        sat.elevation = 50;
        sat.azimuth = 90;
        sat.snr = 99;

        if (i % 2 == 0)
            satsInUse[i / 2] = sat.prn;

        satsInView.push_back(sat);
    }

    info.setSatsInView(satsInView);
    info.setSatsInUse(satsInUse);

    std::string s;
    NMEA::generate(info, s);

    assert(s == "$GPGGA,,,,,,0,08,,,,,,,*6E\r\n"
                "$GPGSA,A,1,01,03,05,07,09,11,13,15,,,,,,,*11\r\n"
                "$GPGSV,4,1,16,01,50,090,99,02,50,090,99,03,50,090,99,04,50,090,99*7F\r\n"
                "$GPGSV,4,2,16,05,50,090,99,06,50,090,99,07,50,090,99,08,50,090,99*74\r\n"
                "$GPGSV,4,3,16,09,50,090,99,10,50,090,99,11,50,090,99,12,50,090,99*72\r\n"
                "$GPGSV,4,4,16,13,50,090,99,14,50,090,99,15,50,090,99,16,50,090,99*7A\r\n"
                "$GPRMC,,V,,,,,,,,,*31\r\n"
                "$GPGLL,,,,,,V*06\r\n");
}

void test_parse_sat()
{
    // The GSA sample below simulates 'gaps' between active satellites
    // that might appear on some devices.
    NMEA::Info info;
    NMEA::parse("$GPGSA,A,1,01,03,05,07,09,,,11,13,,,15,,,*11\r\n"
                "$GPGSV,4,1,16,01,50,090,99,02,50,090,99,03,50,090,99,04,50,090,99*7F\r\n"
                "$GPGSV,4,2,16,05,50,090,99,06,50,090,99,07,50,090,99,08,50,090,99*74\r\n"
                "$GPGSV,4,3,16,09,50,090,99,10,50,090,99,11,50,090,99,12,50,090,99*72\r\n"
                "$GPGSV,4,4,16,13,50,090,99,14,50,090,99,15,50,090,99,16,50,090,99*7A\r\n",
                info);

    assert(info.getSatsInUse() == 8);
    assert(info.getSatsInView() == 16);

    NMEA::SatsInView satsInView;
    NMEA::SatsInUse satsInUse;

    info.getSatsInView(satsInView);
    info.getSatsInUse(satsInUse);

    assert(satsInView.size() == 16);

    for (size_t i = 0; i < satsInView.size(); ++i)
    {
        const NMEA::Satellite& sat = satsInView[i];

        assert(sat.prn == i + 1);
        assert(sat.elevation == 50);
        assert(sat.azimuth == 90);
        assert(sat.snr == 99);

        if (i % 2 == 0)
        {
            assert(std::find(satsInUse,
                             satsInUse + NMEA::MAX_SAT_IN_USE,
                             sat.prn) != satsInUse + NMEA::MAX_SAT_IN_USE);
        }
    }
}

void test_parse_sat_decrease()
{
    NMEA::Info info;
    NMEA::parse("$GPGSV,3,1,12,01,50,090,99,02,50,090,99,03,50,090,99,04,50,090,99*7C\r\n"
                "$GPGSV,3,2,12,05,50,090,99,06,50,090,99,07,50,090,99,08,50,090,99*77\r\n"
                "$GPGSV,3,3,12,09,50,090,99,10,50,090,99,11,50,090,99,12,50,090,99*71\r\n",
                info);
    assert(info.getSatsInView() == 12);

    NMEA::SatsInView sats;
    info.getSatsInView(sats);
    assert(sats.size() == 12);

    NMEA::parse("$GPGSV,2,1,08,01,50,090,99,02,50,090,99,03,50,090,99,04,50,090,99*76\r\n"
                "$GPGSV,2,2,08,05,50,090,99,06,50,090,99,07,50,090,99,08,50,090,99*7D\r\n", info);
    assert(info.getSatsInView() == 8);

    sats.clear();
    info.getSatsInView(sats);
    assert(sats.size() == 8);

    NMEA::parse("$GPGSV,1,1,04,01,50,090,99,02,50,090,99,03,50,090,99,04,50,090,99*79\r\n", info);
    assert(info.getSatsInView() == 4);

    sats.clear();
    info.getSatsInView(sats);
    assert(sats.size() == 4);

    info.clearSatellites();
    sats.clear();
    info.getSatsInView(sats);
    assert(sats.size() == 0);
}

void test_parse_returned_mask()
{
    unsigned int s;
    NMEA::Info info;

    s = NMEA::ALL;
    NMEA::parse("$GPGGA,,,,,,0,,,,,,,,*66\r\n", info, s);
    assert(s == NMEA::GGA);

    s = NMEA::ALL;
    NMEA::parse("$GPGSA,A,1,,,,,,,,,,,,,,,*1E\r\n", info, s);
    assert(s == NMEA::GSA);

    s = NMEA::ALL;
    NMEA::parse("$GPGSV,1,1,0,,,,,,,,,,,,,,,,*49\r\n", info, s);
    assert(s == NMEA::GSV);

    s = NMEA::ALL;
    NMEA::parse("$GPRMC,,V,,,,,,,,,*31\r\n", info, s);
    assert(s == NMEA::RMC);

    s = NMEA::ALL;
    NMEA::parse("$GPGLL,,,,,,V*06\r\n", info, s);
    assert(s == NMEA::GLL);

    s = NMEA::ALL;
    NMEA::parse("$GPGGA,,,,,,0,,,,,,,,*66\r\n"
                "$GPGSA,A,1,,,,,,,,,,,,,,,*1E\r\n"
                "$GPGSV,1,1,0,,,,,,,,,,,,,,,,*49\r\n"
                "$GPRMC,,V,,,,,,,,,*31\r\n"
                "$GPGLL,,,,,,V*06\r\n",
                info, s);
    assert(s == (NMEA::GGA | NMEA::GSA | NMEA::GSV | NMEA::RMC | NMEA::GLL));
}

void test_parse_incomplete_gsv()
{
    NMEA::Info info;
    unsigned int s = NMEA::ALL;
    NMEA::parse("$GPGSV,1,1,04,01,50,090,99,02,50,090,99,03,50,090,99,04,50,090,99*79\r\n", info, s);
    assert(s == NMEA::GSV
           && info.getSatsInView() == 4);

    NMEA::SatsInView sats;
    info.getSatsInView(sats);
    assert(sats.size() == 4
           && sats[0].prn == 1
           && sats[1].prn == 2
           && sats[2].prn == 3
           && sats[3].prn == 4);

    s = NMEA::ALL;
    NMEA::parse("$GPGSV,1,1,03,01,50,090,99,02,50,090,99,03,50,090,99*46\r\n", info, s);
    assert(s == NMEA::GSV
           && info.getSatsInView() == 3);

    sats.clear();
    info.getSatsInView(sats);
    assert(sats.size() == 3
           && sats[0].prn == 1
           && sats[1].prn == 2
           && sats[2].prn == 3);

    s = NMEA::ALL;
    NMEA::parse("$GPGSV,1,1,02,01,50,090,99,02,50,090,99*78\r\n", info, s);
    assert(s == NMEA::GSV
           && info.getSatsInView() == 2);

    sats.clear();
    info.getSatsInView(sats);
    assert(sats.size() == 2
           && sats[0].prn == 1
           && sats[1].prn == 2);

    s = NMEA::ALL;
    NMEA::parse("$GPGSV,1,1,01,01,50,090,99*45\r\n", info, s);
    assert(s == NMEA::GSV
           && info.getSatsInView() == 1);

    sats.clear();
    info.getSatsInView(sats);
    assert(sats.size() == 1
           && sats[0].prn == 1);
}

void test_parse_gobi_gsv()
{
    // GOBI 2000 GPS may emit incomplete GSV sentences
    // that lack azimuth/elevation/SNR values for some satellites.
    // We should filter such satellites as invalid.

    NMEA::Info info;
    NMEA::parse("$GPGSA,A,1,01,02,03,04,05,06,07,,,,,,,,*1E\r\n"
                "$GPGSV,2,1,08,01,11,051,91,02,,052,92,03,13,,93,04,14,054,*4A\r\n"
                "$GPGSV,2,2,08,05,,,95,06,16,,,07,,057,,08,18,058,98*71\r\n",
                info);

    assert(info.getSatsInUse() == 7);
    assert(info.getSatsInView() == 2);

    NMEA::SatsInView sats;
    info.getSatsInView(sats);

    assert(sats.size() == 2);

    const NMEA::Satellite& sat1 = sats[0];
    assert(sat1.prn == 1);
    assert(sat1.elevation == 11);
    assert(sat1.azimuth == 51);
    assert(sat1.snr == 91);

    const NMEA::Satellite& sat2 = sats[1];
    assert(sat2.prn == 8);
    assert(sat2.elevation == 18);
    assert(sat2.azimuth == 58);
    assert(sat2.snr == 98);
}

void test_generate_empty()
{
    NMEA::Info info;

    std::string s;
    NMEA::generate(info, s);

    assert(s == "$GPGGA,,,,,,0,,,,,,,,*66\r\n"
                "$GPGSA,A,1,,,,,,,,,,,,,,,*1E\r\n"
                "$GPRMC,,V,,,,,,,,,*31\r\n"
                "$GPGLL,,,,,,V*06\r\n");
}

void test_copy()
{
    const NMEA::Date date = { 12, 3, 00 };
    const NMEA::Time time = { 12, 34, 56, 789 };

    NMEA::Info from;

    from.setFixType(NMEA::FIX_TYPE_3D);
    from.setFixQuality(NMEA::FIX_QUALITY_SPS);
    from.setLatitude(40.);
    from.setLongitude(-70.);
    from.setAltitude(100.);
    from.setSpeed(5.);
    from.setDirection(30.);
    from.setDeclination(-10.);
    from.setDiff(99.);
    from.setDGPSTime(123456.789);
    from.setDGPSId(66);
    from.setDate(date);
    from.setTime(time);

    NMEA::SatsInView satsInView;
    NMEA::SatsInUse satsInUse = { 0 };

    for (int i = 0; i < NMEA::MAX_SAT_IN_VIEW; ++i)
    {
        NMEA::Satellite sat;
        sat.prn = i + 1;
        sat.elevation = 50;
        sat.azimuth = 90;
        sat.snr = 99;

        if (i % 2 == 0)
            satsInUse[i / 2] = sat.prn;

        satsInView.push_back(sat);
    }

    from.setSatsInView(satsInView);
    from.setSatsInUse(satsInUse);

    std::string nmea_from;
    NMEA::generate(from, nmea_from);

    NMEA::Info to;
    from.copyFixInformation(to);
    from.copyLatitude(to);
    from.copyLongitude(to);
    from.copyAltitude(to);
    from.copySpeed(to);
    from.copyDirection(to);
    from.copyDeclination(to);
    from.copyDiff(to);
    from.copyDGPS(to);
    from.copySatellites(to);
    from.copyDateTime(to);

    std::string nmea_to;
    NMEA::generate(to, nmea_to);

    assert(nmea_from == nmea_to);
}

void test_time_hsecond_parse()
{
    NMEA::Info info;
    NMEA::parse("$GPGGA,000000.9,,,,,0,,,,,,,,*71\r\n", info);
    assert(info.getTime().hsecond == 90);

    NMEA::parse("$GPGGA,000000.99,,,,,0,,,,,,,,*48\r\n", info);
    assert(info.getTime().hsecond == 99);

    NMEA::parse("$GPGGA,000000.999,,,,,0,,,,,,,,*71\r\n", info);
    assert(info.getTime().hsecond == 99);
}

void test_time_hsecond_generate()
{
    NMEA::Info info;

    const NMEA::Time time1 = { 0, 0, 0, 9 };
    info.setTime(time1);

    std::string s;
    NMEA::generate(info, s, NMEA::GGA);
    assert(s == "$GPGGA,000000.90,,,,,0,,,,,,,,*41\r\n");

    const NMEA::Time time2 = { 0, 0, 0, 99 };
    info.setTime(time2);

    s.clear();
    NMEA::generate(info, s, NMEA::GGA);
    assert(s == "$GPGGA,000000.99,,,,,0,,,,,,,,*48\r\n");

    const NMEA::Time time3 = { 0, 0, 0, 999 };
    info.setTime(time3);

    s.clear();
    NMEA::generate(info, s, NMEA::GGA);
    assert(s == "$GPGGA,000000.99,,,,,0,,,,,,,,*48\r\n");
}

void test_slipshod()
{
    // Output from Virtual GPS 1.34 for Windows by Zyl Soft

    NMEA::Info info;
    NMEA::parse("$GPGGA,200215.656,1750.002500,N,04742.091167,E,1,4,0,0,M,0,M,,*4E\r\n"
                "$GPVTG,0,T,0,M,0,N,0,K,A*23\r\n"
                "$GPRMC,200215.656,A,1750.002500,N,04742.091167,E,0,0,180308,0,E,A*11\r\n"
                "$GPGSA,A,3,1,2,3,4,,,,,,,,,0,0,0*28\r\n"
                "$GPGSV,1,1,4,1,15,127,70,2,30,155,80,3,50,160,0,4,18,205,65*7B\r\n",
                info);

    std::string s;
    NMEA::generate(info, s);

    assert (s == "$GPGGA,200215.65,1750.0025,N,04742.0912,E,1,04,0.0,0.0,M,0.0,M,,*54\r\n"
                 "$GPGSA,A,3,01,02,03,04,,,,,,,,,0.0,0.0,0.0*36\r\n"
                 "$GPGSV,1,1,04,01,15,127,70,02,30,155,80,03,50,160,00,04,18,205,65*7B\r\n"
                 "$GPRMC,200215.65,A,1750.0025,N,04742.0912,E,0.0,0.0,180308,0.0,E*56\r\n"
                 "$GPGLL,1750.0025,N,04742.0912,E,200215.65,A*08\r\n");
}

void test_pseudolites()
{
    // Output from the Datalogic prototype

    NMEA::Info info;
    NMEA::parse("$GPGSV,2,1,05,11,66,070,31,32,20,099,,07,14,184,24,08,39,209,30*7A\r\n"
                "$GPGSV,2,2,05,28,77,319,29*46\r\n"
                "$GPGGA,184734.0,4233.602104,N,07052.244191,W,1,04,6.2,-19.0,M,,,,*09\r\n"
                "$GPRMC,184734.0,A,4233.602104,N,07052.244191,W,13.3,35.4,280910,,,A*7C\r\n"
                "$GPGSA,A,3,11,08,255,255,,,,,,,,,,7.3,6.2,3.9*1C\r\n",
                info);

    assert(info.getSatsInView() == 4);
    assert(info.getSatsInUse() == 4);

    std::string s;
    NMEA::generate(info, s);

    assert(s == "$GPGGA,184734.00,4233.6021,N,07052.2442,W,1,04,6.2,-19.0,M,,,,*36\r\n"
                "$GPGSA,A,3,11,08,255,255,,,,,,,,,7.3,6.2,3.9*30\r\n"
                "$GPGSV,2,1,05,11,66,070,31,,,,,07,14,184,24,08,39,209,30*49\r\n"
                "$GPGSV,2,2,05,28,77,319,29,,,,,,,,,,,,*46\r\n"
                "$GPRMC,184734.00,A,4233.6021,N,07052.2442,W,13.3,35.4,280910,,*2E\r\n"
                "$GPGLL,4233.6021,N,07052.2442,W,184734.00,A*18\r\n");
}

int main(int argc, char* argv[])
{
    test_loop_back();
    test_parse_wrong_gsa();
    test_parse_new_rmc();
    test_parse_nan();
    test_parse_incomplete();
    test_generate_sat();
    test_parse_sat();
    test_parse_sat_decrease();
    test_parse_returned_mask();
    test_parse_incomplete_gsv();
    test_generate_empty();
    test_copy();
    test_time_hsecond_parse();
    test_time_hsecond_generate();
    test_slipshod();
    test_parse_gobi_gsv();
    test_pseudolites();

    return 0;
}
