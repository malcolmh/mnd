/*
 ============================================================================
 Name        : mnde.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <regex.h>

#include "n2000.h"
#include "e2000.h"

#define MaxArgs 100

int main(int argc, const char* argv[]) {

	printf("===========================\n");
	printf("Marine Network Data Encoder\n");
	printf("===========================\n");

  regex_t src_dst;
  regex_t decimal;
  regex_t integer;
  regex_t hexadec;
  regcomp(&src_dst, "^.+>.+$", REG_EXTENDED | REG_NOSUB);
  regcomp(&decimal, "^[+-]?[0-9]+([.]?[0-9])*([eE][+-]?[0-9]+)?$", REG_EXTENDED | REG_NOSUB);
  regcomp(&integer, "^[+-]?[0-9]+$", REG_EXTENDED | REG_NOSUB);
  regcomp(&hexadec, "^0[xX][0-9a-fA-F]+$", REG_EXTENDED | REG_NOSUB);
  T_2000 args[MaxArgs];
  E_2000 enc;
  X_2000 txf[32];
  double farg;
  long iarg;
  bool error = false;
  char ln[500];
  int seq = 0;
  int src, dst;
  for (;;) {
    if (fgets(ln, 500, stdin) == NULL)
      return 0;
    src = dst = 0;
    int idx = 0;
    if ((strlen(ln) <= 1) || (*ln == '#')) {
      printf("%s\n", ln);
    } else {
      for (char* arg = ln; (*arg != 0) && (*arg != '\n') && (idx < MaxArgs); idx++) {
        while (*arg == ' ') arg++;
        char* nxt = arg;
        if ((*arg == '\'') || (*arg == '"')) {
          arg++;
          for (char qte = *nxt++; *nxt != qte; nxt++) ;
          *nxt = 0;
          for (int i = 0; arg[i-1] != 0; idx++) {
            args[idx].typ = M2K_ASC;
            for (int j = 0; j < 8; j++) {
              args[idx].dat.asc[j] = arg[i];
              if (arg[i++] == 0) {
                idx--;
                break;
              }
            }
          }
        } else {
          for (; ((*nxt != ' ') && (*nxt != ',') && (*nxt != '\n')); nxt++) ;
          *nxt = 0;
          while (*nxt == ' ') nxt++;
          if ((strlen(arg) == 0) || (strcmp(arg, "n/a") == 0)) {
            args[idx].typ = M2K_I64;
            args[idx].dat.i64 = INT64_MAX;
          } else if (strcmp(arg, "error") == 0) {
            args[idx].typ = M2K_I64;
            args[idx].dat.i64 = INT64_MAX - 1;
          } else if (strcmp(arg, "-") == 0) {
            args[idx].typ = M2K_I64;
            args[idx].dat.i64 = INT64_MAX - 2;
          } else if (!regexec(&src_dst, arg, 0, NULL, 0)) {
            sscanf(arg, "%d>%d", &src, &dst);
            idx--;
          } else if (!regexec(&hexadec, arg, 0, NULL, 0)) {
            args[idx].typ = M2K_I64;
            sscanf(arg, "%lx", &iarg);
            args[idx].dat.i64 = iarg;
          } else if (!regexec(&integer, arg, 0, NULL, 0)) {
            args[idx].typ = M2K_I64;
            sscanf(arg, "%ld", &iarg);
            args[idx].dat.i64 = iarg;
          } else if (!regexec(&decimal, arg, 0, NULL, 0)) {
            args[idx].typ = M2K_F64;
            sscanf(arg, "%lf", &farg);
            args[idx].dat.f64 = farg;
          }
        }
        arg = ++nxt;
      }
      if ((args[0].typ == M2K_I64) && (idx > 1)) {
        enc.dst = dst;
        enc.src = src;
        encodeN2000(idx, args, &enc);
        printf("Payload bytes:\n  ");
        for (int i = 0; i < enc.len; ) {
          for (int j = 0; (j < 32) && (i < enc.len); j++, i++) {
            printf("%02X ", enc.msg[i]);
          }
          printf("\n  ");
        }
        printf("\nCAN frames:\n");
        int nf = framesN2000(&enc, seq++, src, dst, txf);
        for (int i = 0; i < nf; i++) {
          printf("  ");
          printf("%08X [%d] ", txf[i].hdr, txf[i].len);
          for (int j = 0; j < 8; j++) {
            printf("%02X ", txf[i].dat[j]);
          }
          printf("\n");
        }
        printf("Check decode:\n");
        char dec[4000];
        printf("%s\n", decodeN2000(&enc, dec));
        printf("\n");
      } else {
        fprintf(stderr, "Invalid data\n");
        if (error) {
          return 1;
        } else {
          error = true;
          fprintf(stderr, "Data format: [SA>DA,] <PGN>, <parameter>, <parameter>, ...\n");
          fprintf(stderr, "Optional SA>DA pair. Both must be integers with no spaces. (Defaults are 0)\n");
          fprintf(stderr, "<PGN> must be an integer\n");
          fprintf(stderr, "<parameter> fields can be numbers or quoted strings\n");
          fprintf(stderr, "The values \"-\", \"n/a\" & \"error\" can be used for integer fields\n");
          fprintf(stderr, "Blank fields can be used for unavailable or reserved values\n");
          fprintf(stderr, "Lines beginning with '#' are comments and echoed to output\n");
          fprintf(stderr, "Dates should be integers in the form: YYYYMMDD\n");
          fprintf(stderr, "Times should be integers in the form: HHMMSS\n");

        }
      }
    }
  }
  return 0;
}
