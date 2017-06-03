#include "Utilities.h"



vector<const char*> Utilities::FindCommonCStrings( vector<const char*> wantedList, vector<const char*> avalableList )
{
	vector<const char*> returnList;

	string output = "";

	for( int i = 0; i < avalableList.size(); i++ )
	{
		bool enabled = false;
		for( int j = 0; j < wantedList.size(); j++ )
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
