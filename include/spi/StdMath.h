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

#ifdef isnan
#  warning isnan was defined
#  undef isnan
#endif
#ifdef isinf
#  warning isinf was defined
#  undef isinf
#endif
#ifdef log2
#  warning log2 was defined
#  undef log2
#endif

#ifndef WPS_SPI_STDMATH_H_
#define WPS_SPI_STDMATH_H_

namespace WPS {
namespace SPI {
namespace Math {

/**
 * \addtogroup replaceable
 */
/** @{ */

/**
 * Computes the absolute value of the integer
 * 
 * @param x integer to take absolute value
 *
 * @return abolute value of <code>x</code>
 *
 * @see abs in ISO/IEC 9899:1999(E)
 *
 * @author Skyhook Wireless
 */
long abs(long x);

/**
 * Computes the absolute value of the Long long integer
 * 
 * @param x Long long integer to take absolute value
 *
 * @return abolute value of <code>x</code>
 *
 * @see abs in ISO/IEC 9899 (C99)
 *
 * @author Skyhook Wireless 
 */
long long abs(long long x);

/**
 * Computes the absolute value of argument
 * 
 * @param x Double to take absolute value
 *
 * @return abolute value of <code>x</code>
 *
 * @see fabs in ISO/IEC 9899:1999(E)
 *
 * @author Skyhook Wireless
 */
double fabs(double x);

/**
 * Round to smallest integral value not less than x
 * 
 * @param x Double to round
 *
 * @return The smallest integral value greater than or equal to <code>x</code>.
 *
 * @see ceil in ISO/IEC 9899:1999(E)
 *
 * @author Skyhook Wireless 
 */
double ceil(double x);

/**
 * Computes the value of the natural logarithm of argument
 * 
 * @param x Double to take logarithm
 * 
 * @return natural logarithm of <code>x</code>
 * 
 * @see log in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless  
 */
double log(double x);

/**
 * Computes the value of the logarithm base 10 of argument
 * 
 * @param x Double to take logarithm
 * 
 * @return Logarithm base 10 of <code>x</code>
 * 
 * @see log10 in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double log10(double x);

/**
 * Computes the value of <code>x</code> raised to the power of <code>y</code>
 * 
 * @param x Base
 * @param y Exponent
 * 
 * @return <code>x</code> at the power of <code>y</code>
 * 
 * @see pow in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless 
 */
double pow(double x, double y);

/**
 * Computes squared root of argument
 * 
 * @param x Double to take squared root
 * 
 * @return Squared root of <code>x</code>
 * 
 * @see sqrt in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double sqrt(double x);

/**
 * Computes sine of argument
 * 
 * @param x Double to take sine
 * 
 * @return Sine of <code>x</code>
 * 
 * @see sin in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double sin(double x);

/**
 * Computes cosine of argument
 * 
 * @param x Double to take cosine
 * 
 * @return Cosine of <code>x</code>
 * 
 * @see cos in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double cos(double x);

/**
 * Computes the arc tangent of x; that is the value whose tangent is x
 * 
 * @param x Double to take arc tangent
 * 
 * @return Arc tangent of <code>x</code>.
 * The arc tangent in radians and the value is
 * mathematically defined to be between <code>-PI/2</code>
 * and <code>PI/2</code> (inclusive).
 * 
 * @see atan in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double atan(double x);

/**
 * Computes the principal value of the arc tangent of <code>y/x</code>, using the
 * signs of both arguments to determine the quadrant of the return value
 * 
 * @param x Double divisor
 * @param y Double divider
 * 
 * @return Arc tangent of <code>y/x</code>.
 * The arc tangent in radians and the value is
 * mathematically defined to be between <code>-PI</code>
 * and <code>PI</code> (inclusive).
 * 
 * @see atan2 in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double atan2(double x, double y);

/**
 * Compute the principal value of the arc sine of argument in radians
 * 
 * @param x Double to take arc sine
 * 
 * @return The arc sine in the range [<code>0</code>, <code>PI</code>]
 * 
 * @see asin in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double asin(double x);

/**
 * Computes the principal value of the arc cosine of argument in radians
 * 
 * @param x Double to take the arc cosine
 * 
 * @return The arc cosine in the range [<code>0</code>, <code>PI</code>]
 * 
 * @see acos in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
double acos(double x);

/**
 * Compute the principal value of the arc cosine of argument
 * 
 * @param x Double to take arc cosine
 * 
 * @return The arc cosine in the range [<code>0</code>, <code>PI</code>]
 * 

 * @return true if x is NaN, false otherwise
 * 
 * @see isnan in ISO/IEC 9899:1999(E)
 * 
 * @author Skyhook Wireless
 */
bool isnan(double x);

/**
 * Indicates that <code>x</code> is infinity
 * 
 * @param x Double to check
 * 
 * @return true if <code>x</code> is infinity, false otherwise
 *
 * @see isinf in ISO/IEC 9899:1999(E)
 *
 * @author Skyhook Wireless
 */
bool isinf(double x);

/**
 * Break <code>x</code> into fractional and integral parts
 * 
 * @param x Double to break
 * @param intpart Pointer to an object where the integral part is to be stored
 * 
 * @return The fractional part of <code>x</code>, with the same sign
 *
 * @see modf in ISO/IEC 9899:1999(E)
 *
 * @author Skyhook Wireless
 */
double modf(double x, double* intpart);

/** @} */

}
}
}

#endif
