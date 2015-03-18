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

#ifndef WPS_SPI_NMEA_UNITS_H_
#define WPS_SPI_NMEA_UNITS_H_

#include "spi/StdMath.h"

namespace WPS {
namespace SPI {
namespace NMEA {

const double PI    = 3.141592653589793;
const double PI180 = PI / 180;

/**********************************************************************/
/* degree <-> radian                                                  */
/**********************************************************************/

inline double degree2radian(double degree)
{
    return degree * PI180;
}

inline double radian2degree(double radian)
{
    return radian / PI180;
}

/**********************************************************************/
/* NDEG (NMEA degree)                                                 */
/**********************************************************************/

inline double ndeg2degree(double ndeg)
{
    double d;
    Math::modf(ndeg / 100, &d);
    return d + (ndeg - d * 100) / 60;
}

inline double degree2ndeg(double degree)
{
    double int_part;
    double fra_part = Math::modf(degree, &int_part);
    return int_part * 100 + fra_part * 60;
}

inline double ndeg2radian(double ndeg)
{
    return degree2radian(ndeg2degree(ndeg));
}

inline double radian2ndeg(double radian)
{
    return degree2ndeg(radian2degree(radian));
}

/**********************************************************************/
/* DOP                                                                */
/**********************************************************************/

const double DOP_TO_METER_FACTOR = 10; // approximate value

inline double DOP2meters(double dop)
{
    return dop * DOP_TO_METER_FACTOR;
}

inline double meters2DOP(double meters)
{
    return meters / DOP_TO_METER_FACTOR;
}

inline double calcPDOP(double hdop, double vdop)
{
    return Math::sqrt(Math::pow(hdop, 2) + Math::pow(vdop, 2));
}

/**********************************************************************/
/* Speed                                                              */
/**********************************************************************/

const double KPH_IN_KNOT = 1.852; // knot is 1.852 kilometers per hour

inline double kph2knots(double kph)
{
    return kph / KPH_IN_KNOT;
}

inline double knots2kph(double knots)
{
    return knots * KPH_IN_KNOT;
}

}
}
}

#endif
