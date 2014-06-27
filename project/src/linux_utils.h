#ifndef __LINUX_UTILS_H__
#define __LINUX_UTILS_H__

#ifdef _WIN32

#include "windows.h"
#include <conio.h>

#else

#include <sys/types.h>
#include <unistd.h>

#define Sleep(a) usleep(1000*a)

/* A function to detect a keyboard press on Linux */
int32_t _getch();
int32_t _kbhit(void);

#endif

#endif // __COMMON_PICO_H__
