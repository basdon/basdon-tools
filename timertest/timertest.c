/*
Based on timer code from the
Date/time module for the Pawn Abstract Machine
found in amxtime.h, which is Copyright (c) ITB CompuPhase, 2001-2005
*/

#define ISWIN defined __WIN32__ || defined _WIN32 || defined WIN32

#if ISWIN
	#include <windows.h>
	#include <stdio.h>
#else
	#include <sys/time.h>
	#define __stdcall
	#define printf logprintf
	#define NULL 0
#endif

typedef void (*logprintf_t)(char* format, ...);

static
void inittimer()
{
#if ISWIN
	timeBeginPeriod(1);
#endif
}

static
unsigned long gettimestamp()
{
	unsigned long value;

#if ISWIN
	value = (unsigned long) timeGetTime();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	value = (unsigned long) (tv.tv_sec * 1000L);
	value += (unsigned long) (tv.tv_usec / 1000L);
#endif
	return value;
}

unsigned int __stdcall Supports()
{
	return 0x20000 | 0x200;
}

logprintf_t logprintf;

unsigned long startvalue;

unsigned long lastsecond;
unsigned long lastminute;
int seconds;
int minutes;

int __stdcall Load(void **data)
{
	logprintf = (logprintf_t) data[0];
	inittimer();
	startvalue = gettimestamp();
	lastsecond = lastminute = startvalue;
	seconds = minutes = 0;
	printf("initial value %ld\n", startvalue);
	return 1;
}

void __stdcall Unload()
{
}

void __stdcall ProcessTick()
{
	unsigned long t = gettimestamp();
	if (t - lastsecond > 1000) {
		printf("value %ld\n", t);
		printf("second %d\n", seconds++);
		lastsecond += 1000;
	}
	if (t - lastminute > 60000) {
		printf("minute %d\n", minutes++);
		lastminute += 60000;
	}
}

int __stdcall AmxLoad(void *amx)
{
	return 0;
}

int __stdcall AmxUnload(void *amx)
{
	return 0;
}
