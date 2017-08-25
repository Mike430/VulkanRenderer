#include <iostream>
#include "GameEngine.h"

int main()
{
	GameEngine* myGameEngine = new GameEngine;

	if( myGameEngine->isCorrectlyInitialised )
	{
		myGameEngine->RunGameLoop();
	}

	delete myGameEngine;
	myGameEngine = nullptr;

	system( "pause" );
	return 0;
}