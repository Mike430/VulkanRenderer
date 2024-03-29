#pragma once

// Custom dependencies
#include "VK_Renderer.h"
#include "Utilities.h"
#include "GraphicalUtilities.h"

using namespace std;
using namespace Utilities;
using namespace GraphicalUtilities;

class GameEngine
{
private:
	WindowManager*								_mWinManager;
	GLFWwindow*									_mWindow;
	VK_Renderer*								_mVulkanRenderer;

	HR_TimePoint								_mInputStartTime;
	HR_TimePoint								_mInputEndTime;
	dSeconds									_mInputDuration;

	HR_TimePoint								_mUpdateStartTime;
	HR_TimePoint								_mUpdateEndTime;
	dSeconds									_mUpdateDuration;

	HR_TimePoint								_mRenderStartTime;
	HR_TimePoint								_mRenderEndTime;
	dSeconds									_mRenderDuration;

	dSeconds									_mInputUpdateCount;
	dSeconds									_mGameUpdateCount;
	dSeconds									_mRenderUpdateCount;

	dSeconds									_mInputTimeSumTotal;
	dSeconds									_mUpdateTimeSumTotal;
	dSeconds									_mRenderTimeSumTotal;

	HR_TimePoint								_mLastTime;
	HR_TimePoint								_mCurrentTime;
	dSeconds									_mElapsedTime;
	dSeconds									_mCumilativeTime;
	dSeconds									_mDebugTime;

	string										_mTimeDebug;
public:
	bool										isCorrectlyInitialised;

	GameEngine();
	~GameEngine();
	
	void										RunGameLoop();

	void										UpdateUserInput();
	void										UpdateGame();
	void										RenderGame();
	void										IncrementTimeDebug();
};

