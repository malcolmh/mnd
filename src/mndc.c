/*
 ============================================================================
 Name        : mndc.c
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Conversions
 Copyright   : © 2018 Malcolm Herring.

 mndc is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3 of the License, or
 any later version.

 mndc is distributed in the hope that it will be useful,
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

#include <mnd/mnd.h>

typedef struct sen Sen;

typedef struct {
  int64_t pgn;
  char* sen;
  MND_PAR** vars;
} PGN;

struct sen {
  char* sen;
  int64_t pgn;
  MND_PAR** vars;
};

// Stateful variables
static MND_PAR null = {MND_NUL, {.i64=INT64_MAX}};
static MND_PAR date = {0};
static MND_PAR time = {0};
static MND_PAR lat = {0};
static MND_PAR lon = {0};
static MND_PAR cog = {0};
static MND_PAR sog = {0};

// Stateless variables
static MND_PAR tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
static MND_PAR tmp11, tmp12, tmp13, tmp14, tmp16, tmp17;

static MND_PAR* P0[] = {0};
static MND_PAR* P126992[] = {&null, &null, &null, &date, &time, NULL};
static MND_PAR* P129038[] = {&tmp2, &tmp3, &tmp4, &tmp9, &tmp10, &tmp8, &tmp16,
    &tmp13, &tmp11, &tmp7, &tmp17, &tmp1, &tmp12, &tmp6, &tmp5, &tmp14, NULL};
static MND_PAR* P129039[] = {NULL};
static MND_PAR* P129040[] = {NULL};
static MND_PAR* P129041[] = {NULL};
static MND_PAR* P129792[] = {NULL};
static MND_PAR* P129793[] = {NULL};
static MND_PAR* P129794[] = {NULL};
static MND_PAR* P129795[] = {NULL};
static MND_PAR* P129796[] = {NULL};
static MND_PAR* P129797[] = {NULL};
static MND_PAR* P129798[] = {NULL};
static MND_PAR* P129800[] = {NULL};
static MND_PAR* P129801[] = {NULL};
static MND_PAR* P129802[] = {NULL};
static MND_PAR* P129803[] = {NULL};
static MND_PAR* P129804[] = {NULL};
static MND_PAR* P129805[] = {NULL};
static MND_PAR* P129806[] = {NULL};
static MND_PAR* P129807[] = {NULL};
static MND_PAR* P129809[] = {NULL};
static MND_PAR* P129810[] = {NULL};
static MND_PAR* P129811[] = {NULL};
static MND_PAR* P129812[] = {NULL};
static MND_PAR* P129813[] = {NULL};

static PGN pgns[] = {{126992, "ZDA", P126992}, {0}};

static MND_PAR** ais[] = {P0, P129038, P129038, P129038, P129793,
    P129794, P129795, P129796, P129797, P129798,
    P129800, P0, P129801, P129796, P129802,
    P129803, P129804, P129792, P129039, P129040,
    P129805, P129041, P129806, P129807, NULL,
    P129811, P129812, P129813, P0, P0, P0, P0};

static MND_PAR** ais24[] = { P129809, P129810 };

static MND_PAR* ZDA[] = {&time, &date, NULL};
static Sen sens[] = {{"ZDA", 126992, ZDA}, {NULL}};

int convertN2000(int nargs,  MND_PAR args[]) {
  int siz = 0;
  for (int i = 0; pgns[i].vars != NULL; i++) {
    if (pgns[i].pgn == args[0].dat.i64) {
      for (int j = 0; (j < nargs) && (pgns[i].vars[j] != NULL); j++) {
        if (pgns[i].vars[j] != &null) {
          *pgns[i].vars[j] = args[j+1];
        }
      }
      MND_PAR** pars = NULL;
      for (int j = 0; sens[j].sen != NULL; j++) {
        if (strcmp(pgns[i].sen, sens[j].sen) == 0) {
          pars = sens[j].vars;
          break;
        }
      }
      if (pars != NULL) {
        args[siz].typ = MND_ASC;
        strcpy((char*)args[siz].dat.asc, "$UP");
        strcat((char*)&args[siz++].dat.asc[3], pgns[i].sen);
        for (int j = 0; pars[j] != NULL; j++) {
          args[siz++] = *pars[j];
        }
      }
      break;
    }
  }
  return siz;
}

int convertN0183(int nargs,  MND_PAR args[]) {
  int siz = 0;
  for (int i = 0; sens[i].vars != NULL; i++) {
    if (strcmp(sens[i].sen, (char*)&args[0].dat.asc[3]) == 0) {
      for (int j = 0; (j < nargs) && (sens[i].vars[j] != NULL); j++) {
        if (sens[i].vars[j] != &null) {
          *sens[i].vars[j] = args[j+1];
        }
      }
      MND_PAR** pars = NULL;
      for (int j = 0; pgns[j].sen != NULL; j++) {
        if (sens[i].pgn == pgns[j].pgn) {
          pars = pgns[j].vars;
          break;
        }
      }
      if (pars != NULL) {
        args[siz].typ = MND_I64;
        args[siz++].dat.i64 = sens[i].pgn;
        for (int j = 0; pars[j] != NULL; j++) {
          args[siz++] = *pars[j];
        }
      }
      break;
    }
  }
  return siz;
}
