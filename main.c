/*** impoprts */
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

/*** defines */
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data */
struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

/*** terminal */
void abortp(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
  	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1){
		abortp("tcsetattr");
	}
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) abortp("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	raw.c_cc[VMIN] = 0;
  	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) abortp("tcsetattr");
}

char editorReadKey() {
	int nread;
	char c;
	while( (nread = read(STDERR_FILENO, &c, 1)) != 1 ){
		if (nread == -1 & errno != EAGAIN) abortp("read");
	}
	return c;
}

// int getCursorPosition(int *rows, int *cols) {
//   if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
//   printf("\r\n");
//   char c;
//   while (read(STDIN_FILENO, &c, 1) == 1) {
//     if (iscntrl(c)) {
//       printf("%d\r\n", c);
//     } else {
//       printf("%d ('%c')\r\n", c, c);
//     }
//   }
//   editorReadKey();
//   return -1;
// }

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
   if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    // editorReadKey();
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** append buffer */
struct abuf {
	char *b;
	int len;
};

#define ABUF_INIT { NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len){
	char *new = realloc(ab->b, ab->len + len);
	if (new == NULL) return;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len = ab->len + len;
}

void abFree(struct abuf *ab) {
	free(ab->b);
	ab->len = 0
}

/*** output */

void editorDrawRows() {
	int y;
	for (y =0; y < E.screenrows; y ++) {
		write(STDOUT_FILENO, "~", 1);
		if (y < E.screenrows - 1) {
			write(STDOUT_FILENO, "\r\n", 2);
		}
	}
}

void editorRefreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input */

void editorProcessKeyPress() {
	char c = editorReadKey();

	switch (c)
	{
	case CTRL_KEY('q'):
		write(STDOUT_FILENO, "\x1b[2J", 4);
		write(STDOUT_FILENO, "\x1b[H", 3);
		exit(0);
		break;
	}
}

/*** init */

void initEditor() {
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) abortp("getWindowSize");
}

int main() {
	enableRawMode();
	initEditor();

	while (1) {
		editorRefreshScreen();
		editorProcessKeyPress();
	}
	
	return 0;
}

