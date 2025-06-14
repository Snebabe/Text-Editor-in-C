#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// store original terminal settings 
struct termios orig_termios;

void die(const char *s) {
    perror(s);
    exit(1);
}
// Disable raw mode, i.e. restore terminal to original settings
void disableRawMode() {
    // apply original terminal settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

// Set the terminal to "raw" mode, i.e. alter settings and flags
void enableRawMode() {
    // populate settings from STDIN (0) into orig_termios
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    } 
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    /*
     * disable input/output/local flags
     * desc. omitted for BRKINT/INPCK/ISTRIP/CS8
     * ICRNL = Input Carrage Return New Line (disable CR -> NL)
     * IXON = X ON/OFF: flow control
     * OPOST = Output version of ICRNL
     * echo = display input 
     * canon = read line by line instead of byte by byte
     * IEXTEN = Input extended functions, here ctrl:warn("%s"); v 
     * ISIG = SIGINT/SIGTSTP = Signal Interrupt/TerminalStop
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // cc = control characters, 
    // VMIN = min bytes before read() returns (0 = insta return) 
    // VTIME = min time before read() returns (1 = 100ms)
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // FLUSH = input buffer wiped, transmitted output stored
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}

int main(){ 
    enableRawMode();

    while (1) {
        char c = '\0';
        // != EAGAIN for Cygwin
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}
