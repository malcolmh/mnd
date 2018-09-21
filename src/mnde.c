/*
 ============================================================================
 Name        : mnde.c
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Encoder
 Copyright   : © 2016 Malcolm Herring.

 mnde is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3 of the License, or
 any later version.

 mnde is distributed in the hope that it will be useful,
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

#include <mnd/mnd.h>

//*********
extern int convertN2000(int nargs,  MND_PAR args[]);
extern int convertN0183(int nargs,  MND_PAR args[]);
//******

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
  MND_PAR args[MaxArgs];
  E_2000 enc;
  S_2000 txf[50];
  char dec[4000];
  char sen[100];
  double farg;
  long iarg;
  bool error = false;
  char ln[500];
  int seq = 0;
  int src, dst;
  int nargs;
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
          *nxt++ = 0;
        } else {
          for (; ((*nxt != ',') && (*nxt != '\n')); nxt++) ;
          *nxt = 0;
        }
        while (*nxt == ' ') nxt++;
        if ((strlen(arg) == 0) || (strcmp(arg, "n/a") == 0)) {
          args[idx].typ = MND_I64;
          args[idx].dat.i64 = INT64_MAX;
        } else if (strcmp(arg, "error") == 0) {
          args[idx].typ = MND_I64;
          args[idx].dat.i64 = INT64_MAX - 1;
        } else if (strcmp(arg, "-") == 0) {
          args[idx].typ = MND_I64;
          args[idx].dat.i64 = INT64_MAX - 2;
        } else if (!regexec(&src_dst, arg, 0, NULL, 0)) {
          sscanf(arg, "%d>%d", &src, &dst);
          idx--;
        } else if (!regexec(&hexadec, arg, 0, NULL, 0)) {
          args[idx].typ = MND_I64;
          sscanf(arg, "%lx", &iarg);
          args[idx].dat.i64 = iarg;
        } else if (!regexec(&integer, arg, 0, NULL, 0)) {
          args[idx].typ = MND_I64;
          sscanf(arg, "%ld", &iarg);
          args[idx].dat.i64 = iarg;
        } else if (!regexec(&decimal, arg, 0, NULL, 0)) {
          args[idx].typ = MND_F64;
          sscanf(arg, "%lf", &farg);
          args[idx].dat.f64 = farg;
        } else {
          for (int i = 0; true; idx++) {
            args[idx].typ = MND_ASC;
            for (int j = 0; j < 8; j++) {
              args[idx].dat.asc[j] = arg[i];
              if (arg[i++] == 0) break;
            }
            if (arg[i-1] == 0) break;
          }
        }
        arg = ++nxt;
      }
      if ((args[0].typ == MND_I64) && (idx > 1)) {
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
        int nf = enframeN2000(&enc, seq++, txf);
        for (int i = 0; i < nf; i++) {
          printf("  ");
          printf("%08X [%d] ", txf[i].hdr, txf[i].len);
          for (int j = 0; j < 8; j++) {
            printf("%02X ", txf[i].dat[j]);
          }
          printf("\n");
        }
        printf("\nCheck decodes:\n");
        printf("%s\n", translateN2000(&enc, dec));
        nargs = decodeN2000(&enc, args);
        error = false;
      } else if ((args[0].typ == MND_ASC) && ((args[0].dat.asc[0] == '$') || (args[0].dat.asc[0] == '!'))) {
    	  bool ok = encodeN0183(idx, args, sen);
    	  printf("\n%s\n", sen);
        if (ok) {
          printf("Check decodes:\n");
          printf("%s\n", translateN0183(sen, dec));
          nargs = decodeN0183(sen, args);
          error = false;
        }
      } else {
        fprintf(stderr, "Invalid data\n");
        if (error) {
          return 1;
        } else {
          error = true;
          fprintf(stderr, "Data format:\n");
          fprintf(stderr, "  NMEA.0183: ($|!)<address>,<parameter>,<parameter>,...\n");
          fprintf(stderr, "  NMEA.2000: [SA>DA,]<PGN>,<parameter>,<parameter>,...\n");
          fprintf(stderr, "Optional SA>DA pair. Both must be integers with no spaces. (Defaults are 0)\n");
          fprintf(stderr, "<PGN> must be an integer\n");
          fprintf(stderr, "<parameter> fields can be numbers or quoted strings\n");
          fprintf(stderr, "The values \"-\", \"n/a\" & \"error\" can be used for integer fields\n");
          fprintf(stderr, "Blank fields should be used for unavailable or reserved values\n");
          fprintf(stderr, "Parameters must be terminated with a comma. Leading white space is ignored\n");
          fprintf(stderr, "Lines beginning with '#' are comments and echoed to output\n");
          fprintf(stderr, "Dates should be integers in the form: YYYYMMDD\n");
          fprintf(stderr, "Times should be integers or decimals in the form: HHMMSS[.sss]\n");
          fprintf(stderr, "Lat/Lon should be signed decimal degrees\n\n");
        }
      }
      if (!error) {
        bool multi = false;
        int count = 0;
        for (int i = 0; i < nargs; i++) {
          switch (args[i].typ) {
          case MND_NUL:
            printf("<null>");
            break;
          case MND_I64:
            switch (args[i].dat.i64) {
            case INT64_MAX:
              printf("n/a");
              break;
            case INT64_MAX - 1:
              printf("error");
              break;
            case INT64_MAX - 2:
              printf("-");
              break;
            default:
              printf("%lld", args[i].dat.i64);
              break;
            }
            break;
          case MND_F64:
            printf("%.10lg", args[i].dat.f64);
            break;
          case MND_ASC:
            if (!multi) {
              multi = true;
              printf("0x(");
            }
            for (int j = 0; j < 8; j++) {
              if (args[i].dat.asc[j] == 0) {
                multi = false;
                printf("0)");
                break;
              }
              printf("%02X ", args[i].dat.asc[j]);
            }
            break;
          case MND_UNI:
            if (!multi) {
              multi = true;
              printf("0x(");
            }
            for (int j = 0; j < 4; j++) {
              if (args[i].dat.uni[j] == 0) {
                multi = false;
                printf("0)");
                break;
              }
              printf("%04X ", args[i].dat.uni[j]);
            }
            break;
          case MND_BIN:
            if (!multi) {
              multi = true;
              count = args[i].dat.bin[0];
              if (count & 0x1) count |= (args[i].dat.bin[1] << 8);
              count >>= 1;
              printf("0x(");
            }
            for (int j = 0; j < 8; j++) {
              if (--count == 0) {
                multi = false;
                printf(")");
                break;
              }
              printf("%02X ", args[i].dat.asc[j]);
            }
            break;
          default:
            break;
          }
          if ((i < (nargs - 1)) && !multi)
            printf(", ");
        }
        printf("\n\n");
      }
    }
  }
  return 0;
}
