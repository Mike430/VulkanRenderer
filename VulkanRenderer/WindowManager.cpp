#include "WindowManager.h"



WindowManager* WindowManager::_mMyself = nullptr;
GLFWwindow* WindowManager::_mWindow = nullptr;


WindowManager::WindowManager()
{
	glfwInit();
}


WindowManager::~WindowManager()
{
	glfwTerminate(); // window delete cannot be done before glfwTerminate as glfwTerminate cleans windows
	glfwDestroyWindow( _mWindow ); // Can't use delete because terminate alters all windows
}


WindowManager* WindowManager::GetInstance()
{
	if( _mMyself == nullptr )
	{
		_mMyself = new WindowManager();
	}
	return _mMyself;
}

GLFWwindow* WindowManager::GetWindow()
{
	if( _mWindow != nullptr )
	{
		return _mWindow;
	}

	return nullptr;
}


void WindowManager::CreateNewWindow( void* user,
									 int inputMode,
									 uint32_t width,
									 uint32_t height,
									 GLFWwindowsizefun callbackMethod )
{
	// Creates a window "without a context" - Go here to find out more: http://www.glfw.org/docs/latest/context_guide.html#context_less
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	_mWindow = glfwCreateWindow( width, height, "Vulkan Renderer", nullptr, nullptr );
	glfwSetWindowUserPointer( _mWindow, user );
	glfwSetWindowSizeCallback( _mWindow, callbackMethod );

	//glfwMakeContextCurrent( _mWindow );
	glfwSetInputMode( _mWindow, GLFW_STICKY_KEYS, inputMode );
	glfwSetInputMode( _mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
}


void WindowManager::PollWindowInput()
{
	glfwPollEvents();
}


bool WindowManager::WindowCloseRequested()
{
	return glfwWindowShouldClose( _mWindow ) == 0;
}