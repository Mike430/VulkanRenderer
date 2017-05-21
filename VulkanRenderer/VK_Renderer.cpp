#include "VK_Renderer.h"



VK_Renderer::VK_Renderer()
{
	cout << "VK_Renderer constructor called" << endl;

	// Declare a lists of all extensions we want to feed into our Vulkan Instance and Device
	_mWantedInstanceExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
#if defined(VK_USE_PLATFORM_WIN32_KHR) // won't stop x64 builds - it's up to vulkan weather it's used
	_mWantedInstanceExtensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#endif

	_mWantedDeviceExtensions.push_back( "VK_KHR_swapchain" );
	_mWantedDeviceExtensions.push_back( "VK_KHR_shader_draw_parameters" );
	_mWantedDeviceExtensions.push_back( "VK_NV_glsl_shader" );
	_mWantedDeviceExtensions.push_back( "VK_NV_viewport_swizzle" );
	_mWantedDeviceExtensions.push_back( "VK_NV_geometry_shader_passthrough" );


	if( initVulkanGraphicsPipeline() == VK_SUCCESS )
	{
		cout << "VK_Renderer constructor complete" << endl;
		isCorrectlyInitialised = true;
	}
	else
	{
		cout << "VK_Renderer constructor failed" << endl;
		isCorrectlyInitialised = false;
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
	vkDestroyDevice( _mLogicalDevice, nullptr );
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


	uint32_t extensionCount;
	uint32_t finalExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
	cout << "We have " << extensionCount << " extensions available to the instance. They are:" << endl;
	vector<VkExtensionProperties> vkInstanceExtensionProps( extensionCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, vkInstanceExtensionProps.data() );

	for( int i = 0; i < vkInstanceExtensionProps.size(); i++ )
	{
		bool enabled = false;
		for( int j = 0; j < _mWantedInstanceExtensions.size(); j++ )
		{
			if( strcmp( _mWantedInstanceExtensions.at( j ), vkInstanceExtensionProps.at( i ).extensionName ) == 0 )
			{
				_mTurnedOnInstanceExtensions.push_back( _mWantedInstanceExtensions.at( j ) );
				finalExtensionCount++;
				enabled = true;
				continue;
			}
		}
		enabled ? cout << "enabled - " : cout << "dissabled - ";
		cout << vkInstanceExtensionProps.at( i ).extensionName << endl;
	}


	_mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_mInstanceCreateInfo.pApplicationInfo = &_mAppInfo;
	_mInstanceCreateInfo.enabledExtensionCount = finalExtensionCount;
	_mInstanceCreateInfo.ppEnabledExtensionNames = _mTurnedOnInstanceExtensions.data();

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

	_mPhysicalDevice = _mPhysicalDevices.at( winningIndex );
	cout << "\nWe're using the " << physicalDeviceProperties.deviceName << " for graphics\n" << endl;
	return VK_SUCCESS;
}


VkResult VK_Renderer::initLogicalDevice()
{
	VkResult returnResult;

	VkPhysicalDeviceFeatures supportedFeatures;
	VkPhysicalDeviceFeatures requiredFeatures = {};
	vkGetPhysicalDeviceFeatures( _mPhysicalDevice, &supportedFeatures );

	requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
	requiredFeatures.tessellationShader = VK_TRUE;
	requiredFeatures.geometryShader = VK_TRUE;


	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( _mPhysicalDevice, &queueFamilyCount, nullptr );
	vector<VkQueueFamilyProperties> familyProperties( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( _mPhysicalDevice, &queueFamilyCount, familyProperties.data() );
	int graphicsQueueFamilyIndex = 0;

	for( int i = 0; i < queueFamilyCount; i++ )
	{
		cout << VK_QUEUE_GRAPHICS_BIT << " - " << familyProperties.at( i ).queueFlags;

		if( VK_QUEUE_GRAPHICS_BIT & familyProperties.at( i ).queueFlags )
		{
			cout << " : This is the graphics queue." << endl;
			graphicsQueueFamilyIndex = i;
		}
		else
		{
			cout << " : This isn't the graphics queue." << endl;
		}
	}

	float queuePriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;


	// Get the available extensions
	uint32_t extensionCount;
	uint32_t finalExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties( _mPhysicalDevice, nullptr, &extensionCount, nullptr );
	cout << endl << "We have " << extensionCount << " extensions on the chosen graphics card. They are:" << endl;
	vector<VkExtensionProperties> vkDeviceExtensionProps( extensionCount );
	vkEnumerateDeviceExtensionProperties( _mPhysicalDevice, nullptr, &extensionCount, vkDeviceExtensionProps.data() );

	// If the extensions we want exist, add them to the vector we'll use for enabling them
	for( int i = 0; i < vkDeviceExtensionProps.size(); i++ )
	{
		bool enabled = false;
		for( int j = 0; j < _mWantedDeviceExtensions.size(); j++ )
		{
			if( strcmp( _mWantedDeviceExtensions.at( j ), vkDeviceExtensionProps.at( i ).extensionName ) == 0 )
			{
				_mTurnedOnDeviceExtensions.push_back( _mWantedDeviceExtensions.at( j ) );
				finalExtensionCount++;
				enabled = true;
				continue;
			}
		}
		enabled ? cout << "enabled - " : cout << "dissabled - ";
		cout << vkDeviceExtensionProps.at( i ).extensionName << endl;
	}


	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = finalExtensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = _mTurnedOnDeviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &requiredFeatures;



	returnResult = vkCreateDevice( _mPhysicalDevice, &deviceCreateInfo, nullptr, &_mLogicalDevice );

	returnResult == VK_SUCCESS ? cout << "It worked\n\n" : cout << "It did not work\n\n";

	vkGetDeviceQueue( _mLogicalDevice, graphicsQueueFamilyIndex, 0, &_mGraphicsQueue );

	return returnResult;
}


void VK_Renderer::CreateVulkanWindowSurface()
{
	// Creates a window "without a context" - Go here to find out more: http://www.glfw.org/docs/latest/context_guide.html#context_less
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	_mWindow = glfwCreateWindow( 1600, 900, "Vulkan Renderer", NULL, NULL );

	// Creating a Vulkan window surface
	VkResult returnResult = glfwCreateWindowSurface( _mVkInstance, _mWindow, NULL, &_mWindowSurface );
	if( returnResult != VK_SUCCESS )
	{
		cout << "Could not create a window in which to draw. VK_ERROR: " << returnResult << endl;
	}

	glfwMakeContextCurrent( _mWindow );
	glfwSetInputMode( _mWindow, GLFW_STICKY_KEYS, VK_TRUE );
	glfwSetInputMode( _mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
}









void VK_Renderer::GameLoop()
{
	//glfwSetWindowCloseCallback( _mWindow, window_close_callback );
	while( glfwGetKey( _mWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( _mWindow ) == 0 )
	{
		//render( _mWindow );
		glfwSwapBuffers( _mWindow );
		glfwPollEvents();
	}
}