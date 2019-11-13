/*
Based on timer code from the
Date/time module for the Pawn Abstract Machine
found in amxtime.h, which is Copyright (c) ITB CompuPhase, 2001-2005
*/

#define ISWIN defined __WIN32__ || defined _WIN32 || defined WIN32

#include <stdio.h>
#include <time.h>
#if ISWIN
  #include <windows.h>
  #include <mmsystem.h>
#endif

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
	value = (unsigned long) clock();
#if CLOCKS_PER_SEC != 1000
	/* convert to milliseconds */
	value = 1000L * (value + CLOCKS_PER_SEC / 2);
	value /= CLOCKS_PER_SEC;
#endif
#endif
	return value;
}

unsigned int __stdcall Supports()
{
	return 0x20000 | 0x200;
}

unsigned long startvalue;
unsigned long lastsecond;
unsigned long lastminute;
int seconds;
int minutes;

int __stdcall Load(void *data)
{
	inittimer();
	startvalue = lastsecond = lastminute = gettimestamp();
	seconds = minutes = 0;
	return 1;
}

void __stdcall Unload()
{
}

void __stdcall ProcessTick()
{
	unsigned long t = gettimestamp();
	while (t - lastsecond > 1000) {
		printf("second %d\n", seconds++);
		lastsecond += 1000;
	}
	while (t - lastminute > 60000) {
		printf("minute %d\n", minutes++);
		lastminute += 60000;
	}
}

int __stdcall AmxLoad(struct AMX *amx)
{
	return 0;
}

int __stdcall AmxUnload(struct AMX *amx)
{
	return 0;
}
