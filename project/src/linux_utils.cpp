#ifndef _WIN32

#include "linux_utils.h"

#include <sys/types.h>

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>

/*
http://cboard.cprogramming.com/faq-board/27714-faq-there-getch-conio-equivalent-linux-unix.html

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
*/

int32_t _getch() {
	struct termios oldt, newt;
	int32_t ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

/*
http://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
http://www.linuxquestions.org/questions/programming-9/kbhit-34027/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
*/

int32_t _kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

#endif
