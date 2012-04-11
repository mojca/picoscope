#ifndef __LINUX_UTILS__
#define __LINUX_UTILS__

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef WIN32
#define __cdecl
#define __stdcall
#define __fastcall

#define TRUE  1
#define FALSE 0

#define Sleep(a) usleep(1000*a)
/*
#define scanf_s scanf
#define fscanf_s fscanf
int fopen_s(FILE ** fp, const char * filePath, const char * mode) 
{
 *fp = fopen(filePath, mode);
 if (*fp == NULL)
   return 0;
 return 1;
}
typedef enum enBOOL{FALSE,TRUE} BOOL;
*/
/* A function to detect a keyboard press on Linux */
/*
int _getch()
{
	struct termios oldt, newt;
	int ch;
	int bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	do {
		ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
		if (bytesWaiting)
			getchar();
	} while (bytesWaiting);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

int _kbhit()
{
	struct termios oldt, newt;
	int bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return bytesWaiting;
}

/* A function to get a single character on Linux */
// #define max(a,b) ((a) > (b) ? a : b)
// #define min(a,b) ((a) < (b) ? a : b)
/* End of Linux-specific definitions */
#endif

#endif
