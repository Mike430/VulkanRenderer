#include "VK_Renderer.h"



VK_Renderer::VK_Renderer()
{
	cout << "VK_Renderer constructor called" << endl;

	// Declare a lists of all extensions we want to feed into our Vulkan Instance and Device
	_mWantedInstanceExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
#if defined(VK_USE_PLATFORM_WIN32_KHR) // won't stop x64 builds - it's up to vulkan weather it's used
	_mWantedInstanceExtensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#endif

	_mWantedDeviceExtensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	_mWantedDeviceExtensions.push_back( "VK_KHR_shader_draw_parameters" );
	_mWantedDeviceExtensions.push_back( "VK_NV_glsl_shader" );
	_mWantedDeviceExtensions.push_back( "VK_NV_viewport_swizzle" );
	_mWantedDeviceExtensions.push_back( "VK_NV_geometry_shader_passthrough" );


	if( InitVulkanGraphicsPipeline() == VK_SUCCESS )
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
	for( size_t i = 0; i < _mSwapChainImageViews.size(); i++ )
	{
		vkDestroyImageView( _mLogicalDevice, _mSwapChainImageViews[ i ], nullptr );
	}
	vkDestroySwapchainKHR( _mLogicalDevice, _mSwapChainHandle, nullptr );

	vkDestroySurfaceKHR( _mVkInstance, _mWindowSurface, nullptr );
	vkDestroyDevice( _mLogicalDevice, nullptr );
	vkDestroyInstance( _mVkInstance, nullptr );

	cout << "VK_Renderer destructor completed" << endl;
}


VkResult VK_Renderer::InitVulkanGraphicsPipeline()
{
	VkResult returnResult = VK_SUCCESS;

	// Vulkan initialisation methods follow this structure:
	// https://vulkan.lunarg.com/doc/sdk/1.0.46.0/windows/samples_index.html
	if( VK_SUCCESS != ( returnResult = InitInstance() ) )				return returnResult;
	if( VK_SUCCESS != ( returnResult = ChooseAPhysicalDevice() ) )		return returnResult;
	if( VK_SUCCESS != ( returnResult = InitLogicalDevice() ) )			return returnResult;

	if( !glfwInit() )
	{
		cout << "GLFW couldn't be initialised" << endl;
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	else
	{
		if( glfwVulkanSupported() )
		{
			if( VK_SUCCESS != ( returnResult = CreateVulkanWindowSurface() ) ) return returnResult;
		}
		else
		{
			cout << "Vulkan is NOT available to GLFW" << endl;
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	if( VK_SUCCESS != ( returnResult = CreateGraphicalPipeline() ) )	return returnResult;

	return returnResult;
}


VkResult VK_Renderer::InitInstance()
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


VkResult VK_Renderer::ChooseAPhysicalDevice()
{
	vector<VkPhysicalDevice> physicalDevices;
	uint32_t physicalDeviceCount = 0;
	VkResult returnResult = vkEnumeratePhysicalDevices( _mVkInstance,
														&physicalDeviceCount,
														nullptr );

	// If there are any vulkan compatible devices
	if( returnResult == VK_SUCCESS || physicalDeviceCount == 0 )
	{
		// Size the device array appropriately and get the physical device handles
		physicalDevices.resize( physicalDeviceCount );
		returnResult = vkEnumeratePhysicalDevices( _mVkInstance,
												   &physicalDeviceCount,
												   &physicalDevices[ 0 ] );
	}
	else
	{
		cout << "No suitable devices to enumerate" << endl;
		return returnResult;
	}

	cout << physicalDevices.size() << " Physical Device(s) have been found on this system." << endl;

	VkPhysicalDeviceProperties physicalDeviceProperties = VkPhysicalDeviceProperties();
	VkPhysicalDeviceProperties temp_PhysicalDeviceProperties = VkPhysicalDeviceProperties();
	int winningIndex = 0;

	for( int i = 0; i < physicalDevices.size(); i++ )
	{
		vkGetPhysicalDeviceProperties( physicalDevices.at( i ), &temp_PhysicalDeviceProperties );

		cout << i << "\nVkHandle\t" << physicalDevices.at( i ) <<
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

	_mPhysicalDevice = physicalDevices.at( winningIndex );
	cout << "\nWe're using the " << physicalDeviceProperties.deviceName << " for graphics\n" << endl;
	return VK_SUCCESS;
}


VkResult VK_Renderer::InitLogicalDevice()
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


VkResult VK_Renderer::InitSwapChain()
{
	VkResult returnResult;
	uint32_t imageCount = 2; // double buffering

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( _mPhysicalDevice, _mWindowSurface, &surfaceCapabilities );
	VkExtent2D surfaceResolution = surfaceCapabilities.currentExtent;
	uint32_t width = surfaceResolution.width;
	uint32_t height = surfaceResolution.height;

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = _mWindowSurface;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapChainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapChainCreateInfo.imageExtent = surfaceResolution;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapChainCreateInfo.clipped = true;
	swapChainCreateInfo.oldSwapchain = NULL;

	returnResult = vkCreateSwapchainKHR( _mLogicalDevice, &swapChainCreateInfo, 0, &_mSwapChainHandle );

	if( returnResult == VK_SUCCESS )
	{
		uint32_t newImageCount;
		vkGetSwapchainImagesKHR( _mLogicalDevice, _mSwapChainHandle, &newImageCount, nullptr );

		if( imageCount != newImageCount )
		{
			cout << "ERROR: shift in image count from " << imageCount << " to " << newImageCount << endl;
			return VK_ERROR_VALIDATION_FAILED_EXT;
		}

		returnResult = vkGetSwapchainImagesKHR( _mLogicalDevice, _mSwapChainHandle, &imageCount, _mSwapChainImages.data() );

		if( returnResult == VK_SUCCESS )
		{
			for( int i = 0; i < imageCount; i++ )
			{
				VkImageViewCreateInfo imageViewCreateInfo = {};
				imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // render to a 2D image
				imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM; // colour format
				imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
				imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
				imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
				imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
				imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
				imageViewCreateInfo.subresourceRange.levelCount = 1;
				imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				imageViewCreateInfo.subresourceRange.layerCount = 1;
				imageViewCreateInfo.image = _mSwapChainImages.at(i);

				returnResult = vkCreateImageView(_mLogicalDevice, &imageViewCreateInfo, NULL, &_mSwapChainImageViews.at(i));
			}
		}
		else
		{
			cout << endl << "ERROR: could not put images into swap chain" << endl;
		}
	}
	else
	{
		cout << endl << "ERROR: could not create swap chain" << endl;
	}

	return returnResult;
}

VkResult VK_Renderer::CreateGraphicalPipeline()
{
	return VK_SUCCESS;
}


// Needs to be made a part of the core game engine as the renderer shouldn't have responcibility over window and input management.
VkResult VK_Renderer::CreateVulkanWindowSurface()
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

	return returnResult;
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