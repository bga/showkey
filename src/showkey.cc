//# [https://gist.github.com/rprichard/9b05a488439455e6e41fd73e5bfa8c15]
// Compile with g++ showkey.cc -o showkey

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static inline char decodeUnixCtrlChar(char ch) {
    const char ctrlKeys[] = {
        /* 0x00 */ '@', /* 0x01 */ 'A', /* 0x02 */ 'B', /* 0x03 */ 'C',
        /* 0x04 */ 'D', /* 0x05 */ 'E', /* 0x06 */ 'F', /* 0x07 */ 'G',
        /* 0x08 */ 'H', /* 0x09 */ 'I', /* 0x0A */ 'J', /* 0x0B */ 'K',
        /* 0x0C */ 'L', /* 0x0D */ 'M', /* 0x0E */ 'N', /* 0x0F */ 'O',
        /* 0x10 */ 'P', /* 0x11 */ 'Q', /* 0x12 */ 'R', /* 0x13 */ 'S',
        /* 0x14 */ 'T', /* 0x15 */ 'U', /* 0x16 */ 'V', /* 0x17 */ 'W',
        /* 0x18 */ 'X', /* 0x19 */ 'Y', /* 0x1A */ 'Z', /* 0x1B */ '[',
        /* 0x1C */ '\\', /* 0x1D */ ']', /* 0x1E */ '^', /* 0x1F */ '_',
    };
    unsigned char uch = ch;
    if (uch < 32) {
        return ctrlKeys[uch];
    } else if (uch == 127) {
        return '?';
    } else {
        return '\0';
    }
}

// Put the input terminal into non-canonical mode.
static termios setRawTerminalMode()
{
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "input is not a tty\n");
        exit(1);
    }
    if (!isatty(STDOUT_FILENO)) {
        fprintf(stderr, "output is not a tty\n");
        exit(1);
    }

    termios buf;
    if (tcgetattr(STDIN_FILENO, &buf) < 0) {
        perror("tcgetattr failed");
        exit(1);
    }
    termios saved = buf;
    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    buf.c_cflag &= ~(CSIZE | PARENB);
    buf.c_cflag |= CS8;
    buf.c_oflag &= ~OPOST;
    buf.c_cc[VMIN] = 1;  // blocking read
    buf.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &buf) < 0) {
        fprintf(stderr, "tcsetattr failed\n");
        exit(1);
    }
    return saved;
}

static void restoreTerminalMode(termios original)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) < 0) {
        perror("error restoring terminal mode");
        exit(1);
    }
}

int main()
{
    printf("\r\nPress any keys -- Ctrl-D exits\r\n\r\n");
    const termios saved = setRawTerminalMode();
    char buf[128];
    while (true) {
        const ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
        if (len <= 0) {
            break;
        }
        for (int i = 0; i < len; ++i) {
            char ctrl = decodeUnixCtrlChar(buf[i]);
            if (ctrl == '\0') {
                putchar(buf[i]);
            } else {
                putchar('^');
                putchar(ctrl);
            }
        }
        for (int i = 0; i < len; ++i) {
            unsigned char uch = buf[i];
            printf("\t%3d %04o 0x%02x\r\n", uch, uch, uch);
        }
        if (buf[0] == 4) {
            // Ctrl-D
            break;
        }
    }
    restoreTerminalMode(saved);
}
