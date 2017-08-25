#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <chrono>

using namespace std;

typedef enum ConsoleColours : uint8_t
{
	COLOUR_GREEN = 10,
	COLOUR_BLUE = 11,
	COLOUR_RED = 12,
	COLOUR_PURPLE = 13,
	COLOUR_YELLOW = 14,
	COLOUR_WHITE = 15
};

namespace Utilities
{
#ifdef NDEBUG
	static const bool buildIsDebug = false;
#else
	static const bool buildIsDebug = true;
#endif

	static HANDLE UTIL_ConsoleHandle;

	// Timer reference:
	// http://stackoverflow.com/questions/1487695/c-cross-platform-high-resolution-timer
	// https://www.youtube.com/watch?v=M13otPgOePs
	// http://www.cplusplus.com/forum/beginner/68956/ - JLBorges - Apr 21, 2012 at 5:06am
	typedef chrono::steady_clock						Clock;
	typedef chrono::steady_clock::time_point			TimePoint;
	typedef chrono::high_resolution_clock				HR_Clock;
	typedef chrono::high_resolution_clock::time_point	HR_TimePoint;
	typedef chrono::duration<int, milli>				milliseconds;
	typedef chrono::duration<int, micro>				microseconds;
	typedef chrono::duration<float>						fSeconds;
	typedef chrono::duration<double>					dSeconds;

	void												LogIfDebug( string msg, ConsoleColours colour );
	void												LogSuccessIfDebug( string msg );
	void												LogInfoIfDebug( string msg );
	void												LogWarningIfDebug( string msg );
	void												LogErrorIfDebug( string msg );

	vector<char>										ReadFileAsCharVec( string filePathAndName );

	vector<const char*>									FindCommonCStrings( vector<const char*> wantedList, vector<const char*> avalableList );
};

