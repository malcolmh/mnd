#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/termbits.h>
#include <ctype.h>

void wav_init(int dev) {

    uint8_t init[] = { 0xaa, 0x55, 0x12, 0x05, 0x02, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0x01, 0, 0, 0, 0, 0x1a }; // Speed=250000, Extended Frame, Mode=Normal
    struct termios2 tio;

    if (ioctl(dev, TCGETS2, &tio) < 0) {
        perror("TTY tcgets2");
        exit(1);
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag = BOTHER | CS8 | CSTOPB;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_ispeed = 2000000;
    tio.c_ospeed = 2000000;

    if (ioctl(dev, TCSETS2, &tio) < 0) {
        perror("TTY tcsets2");
        exit(1);
    }

    if (write(dev, init, sizeof(init)) < 0) {
        perror("Write init");
        exit(1);
    }
    return;
}
