#include "Utilities.h"



void Utilities::LogIfDebug( string msg, ConsoleColours colour )
{
	if( buildIsDebug )
	{
		if( !UTIL_ConsoleHandle )
		{
			UTIL_ConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
			SetConsoleTextAttribute( UTIL_ConsoleHandle, colour );
		}

		SetConsoleTextAttribute( UTIL_ConsoleHandle, colour );
		cout << msg << endl << endl;
		SetConsoleTextAttribute( UTIL_ConsoleHandle, ConsoleColours::COLOUR_WHITE );
	}
}


void Utilities::LogSuccessIfDebug( string msg )
{
	LogIfDebug( msg, ConsoleColours::COLOUR_GREEN );
}


void Utilities::LogInfoIfDebug( string msg )
{
	LogIfDebug( msg, ConsoleColours::COLOUR_BLUE  );
}


void Utilities::LogWarningIfDebug( string msg )
{
	LogIfDebug( msg, ConsoleColours::COLOUR_YELLOW  );
}


void Utilities::LogErrorIfDebug( string msg )
{
	LogIfDebug( msg, ConsoleColours::COLOUR_RED  );
}


vector<const char*> Utilities::FindCommonCStrings( vector<const char*> wantedList, vector<const char*> avalableList )
{
	vector<const char*> returnList;

	string output = "";

	for( unsigned i = 0; i < avalableList.size(); i++ )
	{
		bool enabled = false;
		for( unsigned j = 0; j < wantedList.size(); j++ )
		{
			if( strcmp( wantedList.at( j ), avalableList.at( i ) ) == 0 )
			{
				returnList.push_back( wantedList.at( j ) );
				enabled = true;
				continue;
			}
		}
		enabled ? output += "enabled - " : output += "dissabled - ";
		output += avalableList.at( i );
		output += "\n";
	}

	cout << endl << output << endl;

	return returnList;
}
