#include "VK_Renderer.h"



VK_Renderer::VK_Renderer()
{
	cout << "VK_Renderer constructor called" << endl;
	if( initVulkanGraphicsPipeline() == VK_SUCCESS )
	{
		cout << "VK_Renderer constructor complete" << endl;
	}
	else
	{
		cout << "VK_Renderer constructor failed" << endl;
	}
}


VK_Renderer::~VK_Renderer()
{
	cout << "VK_Renderer destructor called" << endl;
	
	// Shutdown GLFW first
	glfwTerminate(); // window delete cannot be done before glfwTerminate as glfwTerminate cleans windows
	glfwDestroyWindow( _mWindow ); // Can use delete because terminate alters all windows
	
	// Shutdown Vulkan next
	vkDestroySurfaceKHR( _mVkInstance, _mWindowSurface, nullptr );
	vkDestroyInstance( _mVkInstance, nullptr );

	cout << "VK_Renderer destructor completed" << endl;
}


VkResult VK_Renderer::initVulkanGraphicsPipeline()
{
	VkResult returnResult = VK_SUCCESS;

	// Vulkan initialisation methods follow this structure:
	// https://vulkan.lunarg.com/doc/sdk/1.0.46.0/windows/samples_index.html
	if( VK_SUCCESS != ( returnResult = initInstance() ) )				return returnResult;
	if( VK_SUCCESS != ( returnResult = chooseAPhysicalDevice() ) )		return returnResult;
	if( VK_SUCCESS != ( returnResult = initPhysicalDevice() ) )			return returnResult;
	if( VK_SUCCESS != ( returnResult = initLogicalDevice() ) )			return returnResult;

	if( !glfwInit() )
	{
		cout << "GLFW couldn't be initialised" << endl;
	}
	else
	{
		if( glfwVulkanSupported() )
		{
			CreateVulkanWindowSurface();
		}
		else
		{
			cout << "Vulkan is NOT available to GLFW" << endl;
		}
	}

	return returnResult;
}


VkResult VK_Renderer::initInstance()
{
	VkResult returnResult;

	_mAppInfo = {}; // information about your application and Vulkan compatibility
	_mInstanceCreateInfo = {}; // informations about the specific type of Vulkan instance you wish to create

	_mAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	_mAppInfo.pApplicationName = "Blank Vulkan Window";
	_mAppInfo.engineVersion = 1;
	_mAppInfo.apiVersion = VK_MAKE_VERSION( 1, 0, 0 );

	_mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_mInstanceCreateInfo.pApplicationInfo = &_mAppInfo;

	returnResult = vkCreateInstance( &_mInstanceCreateInfo,
									 nullptr,
									 &_mVkInstance );

	return returnResult;
}


VkResult VK_Renderer::chooseAPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	VkResult returnResult = vkEnumeratePhysicalDevices( _mVkInstance,
														&physicalDeviceCount,
														nullptr );

	// If there are any vulkan compatible devices
	if( returnResult == VK_SUCCESS || physicalDeviceCount == 0 )
	{
		// Size the device array appropriately and get the physical device handles
		_mPhysicalDevices.resize( physicalDeviceCount );
		returnResult = vkEnumeratePhysicalDevices( _mVkInstance,
												   &physicalDeviceCount,
												   &_mPhysicalDevices[ 0 ] );
	}
	else
	{
		cout << "No suitable devices to enumerate" << endl;
		return returnResult;
	}

	cout << _mPhysicalDevices.size() << " Physical Device(s) have been found on this system." << endl;

	VkPhysicalDeviceProperties physicalDeviceProperties = VkPhysicalDeviceProperties();
	VkPhysicalDeviceProperties temp_PhysicalDeviceProperties = VkPhysicalDeviceProperties();
	int winningIndex = 0;

	for( int i = 0; i < _mPhysicalDevices.size(); i++ )
	{
		vkGetPhysicalDeviceProperties( _mPhysicalDevices.at( i ), &temp_PhysicalDeviceProperties );

		cout << i << "\nVkHandle\t" << _mPhysicalDevices.at( i ) <<
			"\nDevice name:\t" << temp_PhysicalDeviceProperties.deviceName <<
			"\nDevice type:\t" << temp_PhysicalDeviceProperties.deviceType << // typedef enum VKPhysicalDeviceType {0-4} - 2 = Descrete GPU
			"\nDevice cpty:\t" << temp_PhysicalDeviceProperties.limits.maxMemoryAllocationCount << endl;

		if( ( physicalDeviceProperties.deviceName == "" ||
			  physicalDeviceProperties.limits.maxMemoryAllocationCount < temp_PhysicalDeviceProperties.limits.maxMemoryAllocationCount ) &&
			temp_PhysicalDeviceProperties.deviceType == 2 )
		{
			physicalDeviceProperties = temp_PhysicalDeviceProperties;
			winningIndex = i;
		}
	}

	if( physicalDeviceProperties.deviceName == "" )
	{
		cout << "\nNo intergrated GPU device was found, exiting." << endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	_mGraphicsCard = _mPhysicalDevices.at( winningIndex );
	cout << "\nWe're using the " << physicalDeviceProperties.deviceName << " for graphics\n" << endl;
	return VK_SUCCESS;
}


VkResult VK_Renderer::initPhysicalDevice()
{
	VkResult returnResult = VK_SUCCESS;

	uint32_t numDeviceLayers = 0;
	vector<VkLayerProperties> deviceLayerProperties;

	vkEnumerateDeviceLayerProperties( _mGraphicsCard,
									  &numDeviceLayers,
									  nullptr );
	if( numDeviceLayers > 0 )
	{
		deviceLayerProperties.resize( numDeviceLayers );
		vkEnumerateDeviceLayerProperties( _mGraphicsCard,
										  &numDeviceLayers,
										  deviceLayerProperties.data() );
		cout << "We have " << numDeviceLayers << " Layers" << endl;
		// I have VK_LAYER_NV_optimus
		// But no VK_LAYER_LUNARG_swapchain - Scratches head
		return VK_SUCCESS;
	}

	cout << "Missing approapriate Vulkan Layers for graphical functionality" << endl;
	return VK_ERROR_INITIALIZATION_FAILED;
}


VkResult VK_Renderer::initLogicalDevice()
{
	VkResult returnResult;

	VkPhysicalDeviceFeatures supportedFeatures;
	VkPhysicalDeviceFeatures requiredFeatures = {};
	vkGetPhysicalDeviceFeatures( _mGraphicsCard, &supportedFeatures );

	requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
	requiredFeatures.tessellationShader = VK_TRUE;
	requiredFeatures.geometryShader = VK_TRUE;

	const VkDeviceQueueCreateInfo deviceQueueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	nullptr, 0, 0, 1, nullptr };

	const VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1,	&deviceQueueCreateInfo,
		0, nullptr,	0, nullptr,&requiredFeatures };

	returnResult = vkCreateDevice( _mGraphicsCard, &deviceCreateInfo, nullptr, &_mLogicalDevice );

	returnResult == VK_SUCCESS ? cout << "It worked\n\n" : cout << "It did not work\n\n";

	return returnResult;
}


void VK_Renderer::CreateVulkanWindowSurface()
{
	// Creates a window "without a context" - Go here to find out more: http://www.glfw.org/docs/latest/context_guide.html#context_less
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	_mWindow = glfwCreateWindow( 1600, 900, "Vulkan Renderer", NULL, NULL );

	// Creating a Vulkan window surface
	VkResult returnResult = glfwCreateWindowSurface( _mVkInstance, _mWindow, NULL, &_mWindowSurface );
	if( returnResult )
	{
		cout << "Could not create a window in which to draw." << endl;
	}

	glfwMakeContextCurrent( _mWindow );
	glfwSetInputMode( _mWindow, GLFW_STICKY_KEYS, VK_TRUE );
	glfwSetInputMode( _mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
}









void VK_Renderer::GameLoop()
{
	//glfwSetWindowCloseCallback( _mWindow, window_close_callback );
	while( glfwGetKey( _mWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( _mWindow ) == 0 )
	{		//render( _mWindow );
		glfwSwapBuffers( _mWindow );
		glfwPollEvents();
	}
}