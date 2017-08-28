#include "GameEngine.h"



GameEngine::GameEngine()
{
	LogInfoIfDebug( "ENGINE CORE CONSTRUCTOR CALLED" );

	_mVulkanRenderer = new VK_Renderer();

	if( _mVulkanRenderer->isCorrectlyInitialised == true )
	{
		_mWinManager = WindowManager::GetInstance();
		_mWindow = _mWinManager->GetWindow();

		isCorrectlyInitialised = true;
		_mLastTime = HR_Clock::now();

		_mDebugTime = dSeconds( 5 );
		_mCumilativeTime = dSeconds( 0 );
		_mInputTimeSumTotal = dSeconds( 0 );
		_mUpdateTimeSumTotal = dSeconds( 0 );
		_mRenderTimeSumTotal = dSeconds( 0 );
		_mInputUpdateCount = dSeconds( 0 );
		_mGameUpdateCount = dSeconds( 0 );
		_mRenderUpdateCount = dSeconds( 0 );
	}
	else
	{
		isCorrectlyInitialised = false;
	}

	LogInfoIfDebug( "ENGINE CORE CONSTRUCTOR COMPLETE" );
}


GameEngine::~GameEngine()
{
	LogInfoIfDebug( "ENGINE CORE DESTRUCTOR CALLED" );

	delete _mVulkanRenderer;
	_mVulkanRenderer = nullptr;

	delete _mWinManager;

	LogInfoIfDebug( "ENGINE CORE DESTRUCTOR COMPLETE" );
}


void GameEngine::RunGameLoop()
{
	while( glfwGetKey( _mWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS && _mWinManager->WindowCloseRequested() )
	{
		_mCurrentTime = HR_Clock::now();
		_mElapsedTime = _mCurrentTime - _mLastTime;

		UpdateUserInput();
		UpdateGame();
		RenderGame();
		IncrementTimeDebug();

		

		_mLastTime = _mCurrentTime;
	}
}


void GameEngine::UpdateUserInput()
{
	_mInputStartTime = HR_Clock::now();
	_mWinManager->PollWindowInput();
	_mInputEndTime = HR_Clock::now();
	_mInputDuration = _mInputEndTime - _mInputStartTime;
}


void GameEngine::UpdateGame()
{
	_mUpdateStartTime = HR_Clock::now();

	_mCumilativeTime += _mElapsedTime;

	if( _mCumilativeTime > _mDebugTime )
	{
		_mTimeDebug = "TimeDebug:";
		_mTimeDebug += "\nInput average = " + to_string( _mInputTimeSumTotal / _mInputUpdateCount ) + " seconds";
		_mTimeDebug += "\nUpdate average = " + to_string( _mUpdateTimeSumTotal / _mGameUpdateCount ) + " seconds";
		_mTimeDebug += "\nRender average = " + to_string( _mRenderTimeSumTotal / _mRenderUpdateCount ) + " seconds";
		_mTimeDebug += "\nElapsed average = " + to_string( _mCumilativeTime / _mGameUpdateCount) + " seconds";

		LogInfoIfDebug( _mTimeDebug );

		_mCumilativeTime = dSeconds( 0 );
		_mInputTimeSumTotal = dSeconds( 0 );
		_mUpdateTimeSumTotal = dSeconds( 0 );
		_mRenderTimeSumTotal = dSeconds( 0 );
		_mInputUpdateCount = dSeconds( 0 );
		_mGameUpdateCount = dSeconds( 0 );
		_mRenderUpdateCount = dSeconds( 0 );
	}

	_mUpdateEndTime = HR_Clock::now();
	_mUpdateDuration = _mUpdateEndTime - _mUpdateStartTime;
}


void GameEngine::RenderGame()
{
	_mRenderStartTime = HR_Clock::now();
	_mVulkanRenderer->RenderScene();	
	_mRenderEndTime = HR_Clock::now();
	_mRenderDuration = _mRenderEndTime - _mRenderStartTime;
}


void GameEngine::IncrementTimeDebug()
{
	_mInputTimeSumTotal += _mInputDuration;
	_mUpdateTimeSumTotal += _mUpdateDuration;
	_mRenderTimeSumTotal += _mRenderDuration;

	_mInputUpdateCount++;
	_mGameUpdateCount++;
	_mRenderUpdateCount++;
}