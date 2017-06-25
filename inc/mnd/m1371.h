/*
 ============================================================================
 Name        : m1371.h
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Library
 Copyright   : © 2016 Malcolm Herring.
 This file is part of libmnd.
 ============================================================================
 */

#ifndef m1371_h
#define m1371_h

#include <unistd.h>
#include <stdint.h>

typedef enum { MESG, ATON, NSTS, TYPE } MLU;

extern char* interpretM1371(uint8_t encData[], int encLen, int pad);
extern char* lookupM1371(int val, MLU tab);

#endif /* m1371_h */
