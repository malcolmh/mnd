/*
 ============================================================================
 Name        : e2000.h
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Library
 Copyright   : © 2016 Malcolm Herring.
 This file is part of libmnd.
 ============================================================================
 */

#ifndef e2000_h
#define e2000_h

#include <stdbool.h>
#include <stdint.h>

#include "n2000.h"

extern void encodeN2000(int nargs, T_2000 args[], E_2000* enc);

extern int enframeN2000(E_2000* enc, int seq, F_2000 frames[]);

#endif /* e2000_h */
