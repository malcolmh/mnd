/*
 ============================================================================
 Name        : m1371.h
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Decoder
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

#ifndef m1371_h
#define m1371_h

#include <unistd.h>
#include <stdint.h>

typedef enum { MESG, ATON, NSTS, TYPE } MLU;

extern char* decodeM1371(uint8_t encData[], int encLen, int pad);
extern char* lookupM1371(int val, MLU tab);

#endif /* m1371_h */
