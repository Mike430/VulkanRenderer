#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>

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

	// Windows
	static HANDLE UTIL_ConsoleHandle;

	void LogIfDebug( string msg, ConsoleColours colour );
	void LogSuccessIfDebug( string msg );
	void LogInfoIfDebug(string msg);
	void LogWarningIfDebug( string msg );
	void LogErrorIfDebug( string msg );

	vector<const char*> FindCommonCStrings( vector<const char*> wantedList, vector<const char*> avalableList);
};

