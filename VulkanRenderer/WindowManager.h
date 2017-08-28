#pragma once
#include "GLFW/glfw3.h"

static class WindowManager
{
private:
	static WindowManager* _mMyself;
	static GLFWwindow* _mWindow;
public:
	WindowManager();
	~WindowManager();
	static WindowManager* GetInstance();
	void CreateNewWindow( void* user,
						  int inputMode,
						  uint32_t width,
						  uint32_t height,
						  GLFWwindowsizefun callbackMethod );
	GLFWwindow* GetWindow();
	void PollWindowInput();
	bool WindowCloseRequested();
};

