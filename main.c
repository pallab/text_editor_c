/*** impoprts */
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

/*** data */
struct termios orig_termios;

/*** raw mode */
void abortp(const char *s) {
	perror(s);
	exit(1);
}

void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1){
		abortp("tcsetattr");
	}
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) abortp("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	raw.c_cc[VMIN] = 0;
  	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) abortp("tcsetattr");
}

/*** init */
int main() {
	enableRawMode();

	while (1)
	{
		char c = '\0';
		if (read(STDERR_FILENO, &c, 1) == -1 && errno != EAGAIN) abortp("read");
		if (iscntrl(c)) {
			printf("%d\r\n", c);
		} else{
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q') break;
	}
	
	return 0;
}

