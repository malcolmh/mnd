/*
 ============================================================================
 Name        : e2000.h
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

#ifndef e2000_h
#define e2000_h

#include <stdbool.h>
#include <stdint.h>

#include "n2000.h"

typedef struct {
  uint32_t hdr;         // Frame header
  uint8_t len;          // Number of bytes
  uint8_t pad1;
  uint8_t pad2;
  uint8_t pad3;
  uint8_t dat[8];       // Frame bytes
} X_2000;               // SocketCAN frame

extern void encodeN2000(int nargs, T_2000 args[], E_2000* enc);

extern int framesN2000(E_2000* dat, int seq, int sads, int dads, X_2000 frames[]);

#endif /* e2000_h */
