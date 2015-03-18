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

#ifndef _ICONVERT_H
#define _ICONVERT_H

#include <iconv.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <stdio.h>

/**
 * Uses iconv to convert between character sets and between storage
 * types. Using the correct FROM and TO storage types for a given
 * from_charset to to_charset encoding is the responsibility of the
 * caller. Caveat emptor.
 *
 * @param     FROM The base type of the src array
 * @param     TO   The base type of the dest array
 * @param[in] from_charset The charset of the src array
 * @param[in] to_charset   The charset of the dest array
 * @param[in] src          The src array
 * @param[in] src_size     The # of elements in src, n/i terminating 0
 * @return    an allocated array of TO, caller responsible for delete
 */
template <typename FROM, typename TO>
std::basic_string<TO> iconvert(const char* from_charset,
                               const char* to_charset,
                               const FROM* src,
                               size_t src_size)
{
    const size_t  dest_bytes = (src_size + 2) * 4; //worst case, with room for leading byte order & trailing \0
    const size_t  src_bytes  = (src_size + 1) * sizeof(FROM);

    iconv_t       converter  = iconv_open(to_charset, from_charset);

    if (converter == (iconv_t)(-1))
        return std::basic_string<TO>();

    char  outbuf[dest_bytes];
    char* outptr            = outbuf;
#if HAVE_ICONV_WITH_CONST_INPUT
    const char* inptr       = reinterpret_cast<const char*>(src);
#else
    char* inptr             = const_cast<char*>(reinterpret_cast<const char*>(src));
#endif
    size_t inleft           = src_bytes;
    size_t outleft          = dest_bytes;

    // init/reset the converter
    iconv(converter, 0, 0, &outptr, &outleft);

    // convert (remember, ret value is # of irreversable conversions)
    size_t nconv = iconv(converter, &inptr, &inleft, &outptr, &outleft);

    iconv_close(converter);

    if (nconv == (size_t)(-1))
        return std::basic_string<TO>();

    const size_t to_length = (dest_bytes - outleft) / sizeof(TO) - 1;
    std::basic_string<TO> to;
    to.resize(to_length);
    for(size_t i = 0; i < to_length; i++)
        to[i] = reinterpret_cast<TO*>(outbuf)[i];
    return to;
}

#if __GLIBC__ == 2 && __GBLIC_MINOR <= 3

  namespace std {

    template<>
    struct char_traits<unsigned char>
    {
      typedef unsigned char     char_type;
      typedef int               int_type;
      typedef streampos         pos_type;
      typedef streamoff         off_type;
      typedef mbstate_t         state_type;

      static void 
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static bool 
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static bool 
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int 
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      { return memcmp(__s1, __s2, __n); }

      static size_t
      length(const char_type* __s)
      { return strlen(reinterpret_cast<const char*>(__s)); }

      static const char_type* 
      find(const char_type* __s, size_t __n, const char_type& __a)
      { return static_cast<const char_type*>(memchr(__s, __a, __n)); }

      static char_type* 
      move(char_type* __s1, const char_type* __s2, size_t __n)
      { return static_cast<char_type*>(memmove(__s1, __s2, __n)); }

      static char_type* 
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {  return static_cast<char_type*>(memcpy(__s1, __s2, __n)); }

      static char_type* 
      assign(char_type* __s, size_t __n, char_type __a)
      { return static_cast<char_type*>(memset(__s, __a, __n)); }

      static char_type 
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      // To keep both the byte 0xff and the eof symbol 0xffffffff
      // from ending up as 0xffffffff.
      static int_type 
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(static_cast<unsigned char>(__c)); }

      static bool 
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static int_type 
      eof() { return static_cast<int_type>(EOF); }

      static int_type 
      not_eof(const int_type& __c)
      { return (__c == eof()) ? 0 : __c; }
    };

    template<>
    struct char_traits<unsigned short>
    {
      typedef unsigned short    char_type;
      typedef int             int_type;
      typedef streampos     pos_type;
      typedef streamoff     off_type;
      typedef mbstate_t     state_type;

      static void 
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static bool 
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static bool 
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int 
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
        for (size_t __i = 0; __i < __n; ++__i)
          if (lt(__s1[__i], __s2[__i]))
            return -1;
          else if (lt(__s2[__i], __s1[__i]))
            return 1;
        return 0;
      }

      static size_t
      length(const char_type* __s)
      {
        std::size_t __i = 0;
        while (!eq(__s[__i], char_type()))
          ++__i;
        return __i;
      }

      static const char_type* 
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
        for (std::size_t __i = 0; __i < __n; ++__i)
          if (eq(__s[__i], __a))
            return __s + __i;
        return 0;
      }

      static char_type* 
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
        return static_cast<char_type*>(std::memmove(__s1, __s2,
                                                    __n * sizeof(char_type)));
      }

      static char_type* 
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
        std::copy(__s2, __s2 + __n, __s1);
        return __s1;
      }

      static char_type* 
      assign(char_type* __s, size_t __n, char_type __a)
      {
        std::fill_n(__s, __n, __a);
        return __s;
      }

      static char_type 
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      // To keep both the byte 0xff and the eof symbol 0xffffffff
      // from ending up as 0xffffffff.
      static int_type 
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(static_cast<unsigned char>(__c)); }

      static bool 
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static int_type 
      eof() { return static_cast<int_type>(EOF); }

      static int_type 
      not_eof(const int_type& __c)
      { return (__c == eof()) ? 0 : __c; }
    };

  }

#endif

#endif 
