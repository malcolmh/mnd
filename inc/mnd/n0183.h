/*
 ============================================================================
 Name        : n0183.h
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Library
 Copyright   : © 2016 Malcolm Herring.
 This file is part of libmnd.

 libmnd is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3 of the License, or
 any later version.

 libmnd is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 A copy of the GNU General Public License can be found here:
 <http://www.gnu.org/licenses/>.

 ============================================================================
 */

#ifndef n0183_h
#define n0183_h

#include <stdbool.h>
#include <stdint.h>

extern char* decodeN0183(char* ln, char* d0183);
extern char* decodeTTD(uint8_t* buf, int len, char* decttd);

#endif /* n0183_h */
