/*
 ============================================================================
 Name        : n2000.h
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Library
 Copyright   : © 2016 Malcolm Herring.
 This file is part of libmnd.
 ============================================================================
 */

#ifndef n2000_h
#define n2000_h

#include <stdbool.h>
#include <stdint.h>

typedef struct p_2000 P_2000;

typedef struct {
  int pgn;          // PGN
  int pri;          // Priority
  int dst;          // Destination address
  int src;          // Source address
  bool fpp;         // Fast packet flag
  int len;          // Number of bytes in message
  uint8_t msg[250]; // Message bytes
} E_2000;           // Message contents

typedef enum { M2K_ASC, M2K_UNI, M2K_I64, M2K_F64 } M2K_TYP;

typedef struct {
  M2K_TYP typ;          // Parameter type
  union {
    uint8_t asc[8];     // Ascii
    uint16_t uni[4];    // Unicode
    int64_t i64;        // Integer
    double f64;         // Real
  }dat;
} T_2000;               // Encode parameter

typedef struct {
  int pgn;        // PGN
  char* cap;      // Text name
  bool fpp;       // Fast packet flag
  int pri;        // Default priority
  P_2000* fmt;    // Format table
} M_2000;         // Message parameters

typedef struct {
  uint32_t hdr;         // Frame header
  uint8_t len;          // Number of bytes
  uint8_t pad1;
  uint8_t pad2;
  uint8_t pad3;
  uint8_t dat[8];       // Frame bytes
} S_2000;               // SocketCAN frame

int deframeN2000(S_2000* frame, E_2000* enc);
extern char* interpretN2000(E_2000* enc, char* d2000);
//extern int unpackN2000(int pgn, int len, unsigned char* msg, int nargs, T_2000 args[]);

#endif /* n2000_h */
