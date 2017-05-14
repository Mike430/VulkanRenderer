#include <iostream>
#include "VK_Renderer.h"

int main()
{
	VK_Renderer* vkRenderer = new VK_Renderer();
	
	if( vkRenderer->isCorrectlyInitialised == true )
	{
		vkRenderer->GameLoop();
	}
	delete vkRenderer;
	vkRenderer = nullptr;

	system( "pause" );
	return 0;
}