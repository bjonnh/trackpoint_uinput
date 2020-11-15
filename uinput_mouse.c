/* gcc -o uinput uinput.c */

// Serial stuff: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/#basic-setup-in-c
// Uinput stuff: https://gist.github.com/kangear/8823740acf5802ce5afc
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <linux/uinput.h>

static void emit(int fd, int type, int code, int val) {
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

void deleteDevice(int fd) {
    if (fd > 0) {
        ioctl(fd, UI_DEV_DESTROY);
        close(fd);
    }
}

int setupMouse() {
    struct uinput_setup usetup;

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "failed to open device %s\n", strerror(errno));
        return 0;
    }
    /* enable mouse button left and relative events */
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    //ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);  // We don't handle it yet
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);

    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "Bjo's nipple");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);
    sleep(1);
    return fd;
}

// I messed up with the wires' order so 3 and 1 are inverted
//
void pointerClick(int fd, int btn) {
    switch(btn) {
      case 3:
            emit(fd, EV_KEY, BTN_MOUSE, 1);
            break;
      case 2:
            emit(fd, EV_KEY, BTN_RIGHT, 1);
            break;
      case 1:
            emit(fd, EV_KEY, BTN_MIDDLE, 1);
            break;
    }
    emit(fd, EV_SYN, SYN_REPORT, 0);
}

// I messed up with the wires' order so 3 and 1 are inverted
//
void pointerRelease(int fd, int btn) {
    switch(btn) {
      case 3:
            emit(fd, EV_KEY, BTN_MOUSE, 0);
            break;
      case 2:
            emit(fd, EV_KEY, BTN_RIGHT, 0);
            break;
      case 1:
            emit(fd, EV_KEY, BTN_MIDDLE, 0);
            break;
    }

    emit(fd, EV_SYN, SYN_REPORT, 0);
}

void pointerMove(int fd, int x, int y) {
    emit(fd, EV_REL, REL_X, x);
    emit(fd, EV_REL, REL_Y, y);
    emit(fd, EV_SYN, SYN_REPORT, 0);
}

void sighandler(int i) {
    (void)i;
    close(0);
}

int main(int argc, char *argv[]) {
    if (argc !=2 ) {
        printf("Usage: sudo %s /dev/tty***\n", argv[0]);
        return 1;
    }

    int fd = setupMouse();

    int serial_port = open(argv[1], O_RDWR);


    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    struct termios tty;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    char read_buf [256];
    memset(&read_buf, '\0', sizeof(read_buf));
    char msgReceived = 0;

    usleep(1000);  // We need to flush the serial port just in case it was stuck sending messages
    tcflush(serial_port,TCIOFLUSH);

    printf("Sending RESET\n"); // We force a reset that way we can recalibrate at each run
    write(serial_port, "RESET\n", 6);
    usleep ((6 + 25) * 100);

    while(!msgReceived) {
        int count;
        int x, y, btn;

        int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
        if (num_bytes>0) {
          switch (read_buf[0]) {
          case 'c':
            count = sscanf(&read_buf[1], " %d", &btn);
            pointerClick(fd, btn);
            break;
          case 'r':
            count = sscanf(&read_buf[1], " %d", &btn);
            pointerRelease(fd, btn);
            break;
          case 'm':
            count = sscanf(&read_buf[1], " %d %d", &x, &y);
            pointerMove(fd, x, y);
            break;
          case 'O':
            printf("Command acknowledged\n");
            break;
          case 'R':
            printf("System ready\n");
            break;

          }
        }
    }
    deleteDevice(fd);
    return 0;
}
