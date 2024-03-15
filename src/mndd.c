/*
 ============================================================================
 Name        : mndd.c
 Author      : Malcolm Herring
 Version     : 0.2
 Description : Marine Network Data Decoder
 Copyright   : © 2016,2023 Malcolm Herring.

 mndd is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3 of the License, or
 any later version.

 mndd is distributed in the hope that it will be useful,
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
#include <ctype.h>

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

typedef union {
    char nsf[4];
    int pgn;
} filter;

filter filters[16] = { 0 };

bool filterPGN(char str[]) {
    if (filters[0].pgn == 0) {
        return true;
    }
    int pgn = 0;
    for (int i = 0; str[i] != 0; i++) {
        if (isdigit(str[i])) {
            pgn = atoi(&str[i]);
            break;
        }
    }
    for (int i = 0; filters[i].pgn != 0; i++) {
        if (filters[i].pgn == pgn) {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {

    int dev = 0;
    char buf[1000];
    char dec[4000];
    char *filter = 0;
    char *device = 0;
    char *speed = 0;
    char *type = 0;

    if (argc > 1) {

        printf("===========================\n");
        printf("Marine Network Data Decoder\n");
        printf("===========================\n");

        type = argv[1];

        int opt;

        while ((opt = getopt(argc, argv, "d:s:f:")) != -1) {
            switch (opt) {
            case 'd':
                device = optarg;
                break;
            case 's':
                speed = optarg;
                break;
            case 'f':
                filter = optarg;
                break;
            }
        }

        if (filter != NULL) {
            int i = 0;
            char *tok = strtok(filter, ",");
            while (tok != NULL) {
                if (isdigit(tok[0])) {
                    filters[i++].pgn = atoi(tok);
                } else {
                    strncpy((char*) &filters[i++], tok, 3);
                }
                tok = strtok(NULL, ",");
            }
        }

        if (strcmp(type, "NGT") == -1) {

            /* NGT-1 input */
            /* Thanks to Kees Verruijt of CANboat for hacking this device */

            if (device != NULL) {
                struct termios attr;
                if ((dev = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
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
                uint8_t init[] = { 0x10, 0x02, 0xA1, 0x03, 0x11, 0x02, 0x00, 0x49, 0x10, 0x03 };
                write(dev, init, sizeof(init));
                sleep(1);
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
                    while ((siz = read(dev, buf, 500)) != 0) {
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
                                            e2k.msg[i] = e2k.msg[i + 11];
                                        }
                                        translateN2000(&e2k, dec);
                                        if (strlen(dec) > 3) {
                                            if (filterPGN(dec)) printf("%s\n", dec);
                                        } else {
                                            printf("*** NGT Checksum error ***\n");
                                        }
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
                return 1;

            } else {

                /* CANboat actisense-serial logfile input */

                FILE *in = fdopen(dev, "r");
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
                    if (filterPGN(dec)) printf("%s\n", dec);
                }
                fclose(in);
            }

        } else if (strcmp(type, "can") == 0) {

            /* SocketCAN input */

            S_2000 frame;
            E_2000 enc;

            if (device != NULL) {
#ifdef __linux__
                struct sockaddr_can addr;
                struct ifreq ifr;

                dev = socket(PF_CAN, SOCK_RAW, CAN_RAW);

                strcpy(ifr.ifr_name, argv[2]);
                if (ioctl(dev, SIOCGIFINDEX, &ifr) < 0) {
                    perror("CAN ioctl");
                    return 1;
                }

                addr.can_family = AF_CAN;
                addr.can_ifindex = ifr.ifr_ifindex;

                if (bind(dev, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
                    perror("CAN bind");
                    return 1;
                }

                while (read(dev, &frame, sizeof(struct can_frame)) > 0) {
                    int msg = deframeN2000(&frame, &enc);
                    if (msg > 0) {
                        translateN2000(&enc, dec);
                        if (filterPGN(dec)) printf("%s\n", dec);
                    }
                }
                perror("CAN read");
                return 1;
#else
                fprintf(stderr, "SocketCAN not available\n");
                return 1;
#endif
            } else {

                /* candump logfile input */

                FILE *in = fdopen(dev, "r");

                while (fgets(buf, 1000, in) != NULL) {
                    strtok(buf, " ");
                    char *header = strtok(NULL, " ");
                    unsigned long hdr = strtol(header, NULL, 16);
                    frame.hdr = hdr;
                    strtok(NULL, " ");
                    frame.len = 0;
                    for (char *body = strtok(NULL, " "); body != NULL; body = strtok(NULL, " ")) {
                        unsigned int byte = 0;
                        sscanf(body, "%02X", &byte);
                        frame.dat[frame.len++] = byte;
                    }
                    int msg = deframeN2000(&frame, &enc);
                    if (msg > 0) {
                        translateN2000(&enc, dec);
                        if (filterPGN(dec)) printf("%s\n", dec);
                    }
                }
                fclose(in);
            }

        } else if (strcmp(type, "tty") == 0) {

            /* TTY input */

            if (device != NULL) {
                if ((dev = open(device, O_RDONLY | O_NOCTTY | O_NDELAY)) < 0) {
                    perror("TTY open");
                    return 1;
                }
                struct termios config;
                if (tcgetattr(dev, &config) < 0) {
                    perror("TTY tcgetattr");
                    return 1;
                }
                cfsetspeed(&config, 4800);
                if (speed != NULL) {
                    cfsetspeed(&config, atoi(speed));
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
                    perror("TTY tcsetattr");
                    return 1;
                }
            }
            FILE *in = fdopen(dev, "r");
            do {
                sleep(1);
                while (fgets(buf, 1000, in) != NULL) {
                    if ((strlen(buf) > 10) && (strlen(buf) < 85)) {
                        if (filter == NULL) {
                            printf("%s\n", translateN0183(buf, dec));
                        } else {
                            for (int i = 0; *filters[i].nsf != 0; i++) {
                                if (strncmp(&buf[3], filters[i].nsf, 3) == 0) {
                                    printf("%s\n", translateN0183(buf, dec));
                                }
                            }
                        }
                    }
                }
            } while (errno == EAGAIN);

            fclose(in);
        }

    } else {
        fprintf(stderr, "===========================\n");
        fprintf(stderr, "Marine Network Data Decoder\n");
        fprintf(stderr, "===========================\n");
        fprintf(stderr, "Usage: mndd {tty|can|ngt} [-d <device> [-s <speed>]] [-f <filters>]\n");
        fprintf(stderr, "Default Input from stdin, Output to stdout\n");
        fprintf(stderr, "Default decoding is N0183\n");
        fprintf(stderr, "tty: N0183 input from serial port (ASCII data)\n");
        fprintf(stderr, "  (if no device, input from text file/pipe on stdin)\n");
        fprintf(stderr, "ngt: N2000 input from Actisense NGT-1 (Byte aligned binary data)\n");
        fprintf(stderr, "  (if no device, input from actisense-serial log file/pipe on stdin)\n");
        fprintf(stderr, "can: N2000 input from SocketCAN (Byte aligned binary data)\n");
        fprintf(stderr, "  (if no device, input from candump log file/pipe on stdin)\n");
        fprintf(stderr, "filters: comma separated list of sentence formatters or PGNs\n");
    }
    close(dev);
    return 0;
}
