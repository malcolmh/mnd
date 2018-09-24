/*
 ============================================================================
 Name        : mndb.c
 Author      : Malcolm Herring
 Version     : 0.1
 Description : Marine Network Data Bridge
 Copyright   : © 2018 Malcolm Herring.

 mndb is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3 of the License, or
 any later version.

 mndb is distributed in the hope that it will be useful,
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
#include <pthread.h>
#include <signal.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <net/if.h>

#ifdef __linux__
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

#include <mnd/mnd.h>
extern int convertN2000(int nargs,  MND_PAR args[]);
extern int convertN0183(int nargs,  MND_PAR args[]);

MND_PAR preq[] = {{MND_I64, {.i64=59904}}, {MND_I64, {.i64=60928}}};
MND_PAR aclm[] = {{MND_I64, {.i64=60928}}, {MND_NUL, {.i64=INT64_MAX}},
    {MND_NUL, {.i64=INT64_MAX}}, {MND_NUL, {.i64=INT64_MAX}},
    {MND_NUL, {.i64=INT64_MAX}}, {MND_NUL, {.i64=INT64_MAX}},
    {MND_NUL, {.i64=INT64_MAX}}, {MND_NUL, {.i64=INT64_MAX}},
    {MND_NUL, {.i64=INT64_MAX}}, {MND_NUL, {.i64=INT64_MAX}},
    {MND_NUL, {.i64=INT64_MAX}}};

typedef struct {
  int dev;
  int inip;
  int inop;
  int outip;
  int outop;
  uint8_t* inring[10];
  uint8_t* outring[10];
} IO_RING;

pthread_t n2000rx, n2000tx, n0183rx, n0183tx;

sigset_t sigs;

void* ngt_inp(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  for (;;) {
    // read device, convert, add to queue
    pthread_kill(n0183tx, SIGUSR1);
  }
/*  char buf[1000];
  E_2000 e2k;
  uint8_t ch;
  long siz = 0;
  enum {
    NGT_CC0, NGT_CC1, NGT_CC2, NGT_LEN, NGT_BUF, NGT_ESC
  } state = NGT_CC0;
  int chk = 0;
  int len = 0;
  int idx = 0;
  do {
    sleep(1);
    while ((siz = read(inring->dev, buf, 500)) != 0) {
      for (int i = 0; i < siz; i++) {
        ch = buf[i];
        switch (state) {
        case NGT_CC0:
          if (ch == 0x10)
            state = NGT_CC1;
          break;
        case NGT_CC1:
          if (ch == 0x02)
            state = NGT_CC2;
          else
            state = NGT_CC0;
          break;
        case NGT_CC2:
          if (ch == 0x93) {
            chk = ch;
            state = NGT_LEN;
          } else
            state = NGT_CC0;
          break;
        case NGT_LEN:
          chk += ch;
          len = ch;
          if ((len < 12) || (len > 250)) {
            state = NGT_CC0;
          } else {
            idx = 0;
            state = NGT_BUF;
          }
          break;
        case NGT_BUF:
          if (len > 0) {
            if (ch == 0x10) {
              state = NGT_ESC;
            } else {
              chk += ch;
              e2k.msg[idx++] = ch;
            }
            len--;
          } else {
            chk += ch;
            if ((chk & 0xff) == 0) {
              e2k.pgn = e2k.msg[1] + ((e2k.msg[2] + (e2k.msg[3] << 8)) << 8);
              e2k.pri = e2k.msg[0];
              e2k.dst = e2k.msg[4];
              e2k.src = e2k.msg[5];
              e2k.len = idx - 11;
              for (int i = 0; i < e2k.len; i++) {
                e2k.msg[i] = e2k.msg[i+11];
              }
              translateN2000(&e2k, dec);
              if (strlen(dec) > 3)
                printf("%s\n", dec);
            } else {
              printf("*** NGT Checksum error ***\n");
            }
            state = NGT_CC0;
          }
          break;
        case NGT_ESC:
          chk += ch;
          e2k.msg[idx++] = ch;
          state = NGT_BUF;
          break;
        }
      }
    }
  } while (errno == EAGAIN);
  perror("NGT read");
  close(dev);
*/  return NULL;
}

void* ngt_out(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  int signum;
  for(;;) {
    sigwait(&sigs, &signum);
    if (signum == SIGUSR1) {
      // Empty send queue
    }
  }
  return NULL;
}

void* can_in(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  S_2000 frame;
  E_2000 enc;

#ifdef __linux__
    struct sockaddr_can addr;
    struct ifreq ifr;

    dev = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, argv[2]);
    if (ioctl(dev, SIOCGIFINDEX, & ifr) < 0) {
      perror("CAN ioctl");
      return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(dev, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("CAN bind");
      return 1;
    }

    while (read(dev, &frame, sizeof(struct can_frame)) > 0) {
      int msg = deframeN2000(&frame, &enc);
      if (msg > 0) printf("%s\n", translateN2000(&enc, dec));
    }
    perror("CAN read");
    return 1;
#endif
    return NULL;
}

void* can_out(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

void* ser_in(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

void* ser_out(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

void* dmp_in(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
/*  FILE* in = fdopen(dev, "r");

  while (fgets(buf, 1000, in) != NULL) {
    strtok(buf, " ");
    char* header = strtok(NULL, " ");
    unsigned long hdr = strtol(header, NULL, 16);
    frame.hdr = hdr;
    strtok(NULL, " ");
    frame.len = 0;
    for (char* body = strtok(NULL, " "); body != NULL; body = strtok(NULL, " ")) {
      unsigned int byte = 0;
      sscanf(body, "%02X", &byte);
      frame.dat[frame.len++] = byte;
    }
    int msg = deframeN2000(&frame, &enc);
    if (msg > 0) printf("%s\n", translateN2000(&enc, dec));
   }
  fclose(in);
}*/
  return NULL;
}

void* dmp_out(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

void* com_in(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
/*  char buf[1000];
  FILE* in = fdopen(inring->dev, "r");
  while (fgets(buf, 1000, in) != NULL) {
    strtok(buf, ",");
    E_2000 e2k;
    e2k.pri = (int) strtol(strtok(NULL, ","), NULL, 10);
    e2k.pgn = (int) strtol(strtok(NULL, ","), NULL, 10);
    e2k.src = (int) strtol(strtok(NULL, ","), NULL, 10);
    e2k.dst = (int) strtol(strtok(NULL, ","), NULL, 10);
    e2k.len = (int) strtol(strtok(NULL, ","), NULL, 10);
    for (int i = 0; i < e2k.len; i++) {
      e2k.msg[i] = strtol(strtok(NULL, ",\n"), NULL, 16);
    }
    translateN2000(&e2k, dec);
    if (strlen(dec) > 3)
      printf("%s\n", dec);
  }
  fclose(in);
*/
  return NULL;
}

void* com_out(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

void* txt_in(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

void* txt_out(void* ring) {
  IO_RING* inring = (IO_RING*)ring;
  return NULL;
}

int main(int argc, const char* argv[]) {

  sigemptyset(&sigs);
  sigaddset(&sigs, SIGUSR1);

//  pthread_create(pthread_t* thread, NULL, void *(*start_routine) (void *), NULL);

  printf("==========================\n");
  printf("Marine Network Data Bridge\n");
  printf("==========================\n");

  int argi = 1;
  if (argc > 3) {
    if (strcmp(argv[argi], "ngt") == 0) {
      argi++;
// set up ngt
    } else if (strcmp(argv[argi++], "can") == 0) {
// set up socketcan
    } else {
      argi = 99;
    }
    if (argc > argi) {
      if (strcmp(argv[argi], "to") == 0) {
// flag n2k>183
        argi++;
      } else if (strcmp(argv[argi], "from") == 0) {
// flag n2k<183
        argi++;
      } else if (strcmp(argv[argi], "com") != 0) {
        argi = 99;
      }
    }
    if ((argc > argi) && (strcmp(argv[argi], "com") == 0)) {
// set up com port
    }
    if ((argc > argi) && (strncmp(argv[argi], "h=", 2) == 0)) {
// set up heading
    }

    /* NGT-1 input */
    /* Thanks to Kees Verruijt of CANboat for hacking this device */

 /*   if (strcmp(argv[1], "ngt") == 0) {
      if (argc > 2) {
        struct termios attr;
        if ((dev = open(argv[2], O_RDWR | O_NOCTTY)) < 0) {
          perror("NGT open");
          return 1;
        }
        memset(&attr, 0, sizeof(attr));
        cfsetspeed(&attr, B115200);
        cfmakeraw(&attr);
        attr.c_cc[VMIN] = 1;
        attr.c_cc[VTIME] = 0;
        tcflush(dev, TCIFLUSH);
        tcsetattr(dev, TCSANOW, &attr);
        uint8_t cmd[] = { 0x00, 0x00, 0x10, 0x02, 0xA1, 0x03, 0x11, 0x02, 0x00, 0x49, 0x10, 0x03 };
        write(dev, cmd, sizeof(cmd));
        sleep(1);
        return 1;

      } else {

        /* CANboat actisense-serial logfile input
      }

    } else if (strcmp(argv[1], "can") == 0) {

      /* SocketCAN input
#ifdef __linux__

#else
        fprintf(stderr, "SocketCAN not available\n");
        return 1;
#endif
      } else {

        /* candump logfile input


    } else if (strcmp(argv[1], "com") == 0) {

      /* COM input

      if (argc > 2) {
        if ((dev = open(argv[2], O_RDONLY | O_NOCTTY | O_NDELAY)) < 0) {
          perror("COM open");
          return 1;
        }
        struct termios config;
        if (tcgetattr(dev, &config) < 0) {
          perror("COM tcgetattr");
          return 1;
        }
        cfsetspeed(&config, B115200);
        if (argc > 3) {
          int baud = atoi(argv[3]);
          switch (baud) {
          case 4800:
            cfsetspeed(&config, B4800);
            break;
          case 9600:
            cfsetspeed(&config, B9600);
            break;
          case 19200:
            cfsetspeed(&config, B19200);
            break;
          case 38400:
            cfsetspeed(&config, B38400);
            break;
          }
        }
        config.c_cflag |= (CLOCAL | CREAD);
        config.c_cflag &= ~PARENB;
        config.c_cflag &= ~CSTOPB;
        config.c_cflag &= ~CSIZE;
        config.c_cflag |= CS8;
        config.c_cflag &= ~CRTSCTS;
        config.c_lflag &= ~( ECHO | ECHOE);
        config.c_lflag |= ICANON;
        if (tcsetattr(dev, TCSANOW, &config) < 0) {
          perror("COM tcsetattr");
          return 1;
        }
      }
      FILE* in = fdopen(dev, "r");
      do {
        sleep(1);
        while (fgets(buf, 1000, in) != NULL) {
          if ((strlen(buf) > 10) && (strlen(buf) < 85))
            printf("%s\n", translateN0183(buf, dec));
        }
      } while (errno == EAGAIN);

      fclose(in);
    }
*/  }
  fprintf(stderr, "===========================\n");
  fprintf(stderr, "Marine Network Data Bridge\n");
  fprintf(stderr, "===========================\n");
  fprintf(stderr, "Usage: mndb (can|ngt) [<device> [s=<speed>]] [to|from] (com [<device> [s=<speed>]]...) [h=<heading>]\n");
  fprintf(stderr, "default is bi-directional conversion\n");
  fprintf(stderr, "to: N2000 conversion to N0183 only\n");
  fprintf(stderr, "from: N0183 conversion to N2000 only\n");
  fprintf(stderr, "can: N2000 SocketCAN (Byte aligned binary data)\n");
  fprintf(stderr, "  (if no device, candump log file/pipe on stdin/stdout)\n");
  fprintf(stderr, "ngt: N2000 Actisense NGT-1 (Byte aligned binary data)\n");
  fprintf(stderr, "  (if no device, actisense-serial log file/pipe on stdin/stdout)\n");
  fprintf(stderr, "com: N0183 com port (ASCII data)\n");
  fprintf(stderr, "  more than one device may be specified. (if no device, text file/pipe on stdin/stdout)\n");
  fprintf(stderr, "heading: (0-359) output fixed heading\n");

}
