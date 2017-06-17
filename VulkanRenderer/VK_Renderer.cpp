#include "VK_Renderer.h"



VK_Renderer::VK_Renderer()
{
	_mConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( _mConsoleHandle, 15 );
	cout << "VK_Renderer constructor called" << endl;

	//_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_api_dump" ); // Doesn't show errors, just shows the evolution of your vulkan app by dumping every key change that occures
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_core_validation" ); // Logs every error that occures
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_monitor" );
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_object_tracker" ); // tells you have you haven't deleted when cleaning up
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_parameter_validation" );
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_screenshot" );
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_swapchain" ); // specifically targets swapchain issues
	_mWantedInstanceLayers.push_back( "VK_LAYER_GOOGLE_threading" );
	_mWantedInstanceLayers.push_back( "VK_LAYER_GOOGLE_unique_objects" );
	//_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_vktrace" ); // breaks my application for some reason at instance initialisation
	_mWantedInstanceLayers.push_back( "VK_LAYER_NV_optimus" );
	//_mWantedInstanceLayers.push_back( "VK_LAYER_RENDERDOC_Capture" ); // fix pipeline and this should work with renderdoc
	//_mWantedInstanceLayers.push_back( "VK_LAYER_VALVE_steam_overlay" );
	_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_standard_validation" ); // additional error loggings

	// Declare a lists of all extensions we want to feed into our Vulkan Instance and Device
	_mWantedInstanceExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
	_mWantedInstanceExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
#if defined(VK_USE_PLATFORM_WIN32_KHR) // won't stop x64 builds - it's up to vulkan weather it's used
	_mWantedInstanceExtensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#endif

	_mWantedDeviceExtensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	_mWantedDeviceExtensions.push_back( "VK_KHR_shader_draw_parameters" );
	_mWantedDeviceExtensions.push_back( "VK_NV_glsl_shader" );
	_mWantedDeviceExtensions.push_back( "VK_NV_viewport_swizzle" );
	_mWantedDeviceExtensions.push_back( "VK_NV_geometry_shader_passthrough" );


	IfVKErrorPrintMSG( InitVulkanAndGLFW(),
					   "Constructor failed.",
					   "Constructor complete." ) ? isCorrectlyInitialised = false : isCorrectlyInitialised = true;
}


VK_Renderer::~VK_Renderer()
{
	cout << "VK_Renderer destructor called" << endl;


	// Shutdown Vulkan
	for( auto i = 0; i < _mSwapChainSize; i++ )
	{
		vkDestroyImageView( _mLogicalDevice, _mSwapChainImageViews[ i ], nullptr );
		vkDestroyFramebuffer( _mLogicalDevice, _mSwapChainFrameBuffers.at( i ), nullptr );
		vkDestroyFence( _mLogicalDevice, _mSwapChainRenderFences.at( i ), nullptr );
	}
	vkFreeMemory( _mLogicalDevice, _mVertexBufferMemory, nullptr );
	vkDestroyBuffer( _mLogicalDevice, _mVertexBuffer, nullptr );
	vkDestroySwapchainKHR( _mLogicalDevice, _mSwapChainHandle, nullptr );
	vkDestroyRenderPass( _mLogicalDevice, _mRenderPass, nullptr ); // Destroys the swap chain images
	vkDestroyCommandPool( _mLogicalDevice, _mGraphicsQueueCmdPool, nullptr );
	vkDestroySurfaceKHR( _mVkInstance, _mWindowSurface, nullptr );
	vkDestroyDevice( _mLogicalDevice, nullptr );
	DestroyDebugReportCallbackEXT( _mVkInstance, _mDebugCallbackHandle, nullptr );
	vkDestroyInstance( _mVkInstance, nullptr );


	// Shutdown GLFW
	glfwTerminate(); // window delete cannot be done before glfwTerminate as glfwTerminate cleans windows
	glfwDestroyWindow( _mWindow ); // Can use delete because terminate alters all windows

	cout << "VK_Renderer destructor completed" << endl;
}

// returns true if we threw an error
bool VK_Renderer::IfVKErrorPrintMSG( VkResult VkState, string output )
{
	if( _mIsDebugBuild )
	{
		if( VkState != VK_SUCCESS )
		{
			SetConsoleTextAttribute( _mConsoleHandle, 12 );
			cout << endl << _mVkReportPrefix << output << endl;
			SetConsoleTextAttribute( _mConsoleHandle, 15 );
		}
	}

	return VkState != VK_SUCCESS;
}

// returns true if we threw an error
bool VK_Renderer::IfVKErrorPrintMSG( VkResult VkState, string errOutput, string successOutput )
{
	if( _mIsDebugBuild )
	{
		VkState == VK_SUCCESS ? SetConsoleTextAttribute( _mConsoleHandle, 10 ) : SetConsoleTextAttribute( _mConsoleHandle, 12 );
		cout << endl << _mVkReportPrefix << ( VkState == VK_SUCCESS ? successOutput : errOutput ) << endl;
		SetConsoleTextAttribute( _mConsoleHandle, 15 );
	}

	return VkState != VK_SUCCESS;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VK_Renderer::DebugCallback( VkDebugReportFlagsEXT msgTypesAsFlags,
														   VkDebugReportObjectTypeEXT msgSubjectObj,
														   uint64_t object,
														   size_t location,
														   int32_t code,
														   const char* layerPrefix,
														   const char* message,
														   void* userData )
{
	HANDLE console = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( console, 14 );
	cerr << "Debug Callback was called, message reads: " << layerPrefix << " -> " << message << endl;
	SetConsoleTextAttribute( console, 15 );
	return VK_FALSE;
}


VkResult VK_Renderer::CreateDebugReportCallbackEXT( VkInstance instance,
													const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
													const VkAllocationCallbacks* pAllocator,
													VkDebugReportCallbackEXT* pCallback )
{
	auto func = ( PFN_vkCreateDebugReportCallbackEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
	if( func != nullptr )
	{
		return func( instance, pCreateInfo, pAllocator, pCallback );
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


void VK_Renderer::DestroyDebugReportCallbackEXT( VkInstance instance,
												 VkDebugReportCallbackEXT callback,
												 const VkAllocationCallbacks* pAllocator )
{
	auto func = ( PFN_vkDestroyDebugReportCallbackEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	if( func != nullptr )
	{
		func( instance, callback, pAllocator );
	}
}


VkResult VK_Renderer::InitVulkanAndGLFW()
{
	if( !glfwInit() )
	{
		cout << "GLFW couldn't be initialised" << endl;
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	CreateGLFWWindow();
	return InitVulkanRenderer();
}


VkResult VK_Renderer::InitVulkanRenderer()
{
	VkResult returnResult = VK_SUCCESS;

	if( IfVKErrorPrintMSG( InitInstance(),
						   "Failed to initialise instance.",
						   "Initialised instance." ) )						return returnResult;
	if( IfVKErrorPrintMSG( SetUpDebugCallback(),
						   "Failed to setup debug.",
						   "Initialised debug." ) )							return returnResult;
	if( IfVKErrorPrintMSG( ChooseAPhysicalDevice(),
						   "Failed to choose a device.",
						   "Chose device." ) )								return returnResult;
	if( IfVKErrorPrintMSG( InitLogicalDevice(),
						   "Failed to initialise device.",
						   "Initialised device." ) )						return returnResult;
	if( IfVKErrorPrintMSG( InitialiseWindowSurface(),
						   "Failed to initialise surface.",
						   "Initialised surface." ) )						return returnResult;
	if( IfVKErrorPrintMSG( InitVulkanGraphicalPipeline(),
						   "Failed to initialise pipeline.",
						   "Initialised pipeline." ) )						return returnResult;

	return returnResult;
}


VkResult VK_Renderer::InitInstance()
{
	VkResult returnResult;

	// Layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
	cout << "We have " << layerCount << " layers available to the instance. They are:" << endl;
	vector<VkLayerProperties> vkLayerProps( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, vkLayerProps.data() );

	vector<const char*> avalableNames;
	if( _mIsDebugBuild )
	{
		for( unsigned i = 0; i < vkLayerProps.size(); i++ )
		{
			avalableNames.push_back( vkLayerProps.at( i ).layerName );
		}
		_mTurnedOnInstanceLayers = FindCommonCStrings( _mWantedInstanceLayers, avalableNames );
	}

	// Extensions
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
	cout << "We have " << extensionCount << " extensions available to the instance. They are:" << endl;
	vector<VkExtensionProperties> vkInstanceExtensionProps( extensionCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, vkInstanceExtensionProps.data() );

	avalableNames.clear();
	for( unsigned i = 0; i < vkInstanceExtensionProps.size(); i++ )
	{
		avalableNames.push_back( vkInstanceExtensionProps.at( i ).extensionName );
	}
	_mTurnedOnInstanceExtensions = FindCommonCStrings( _mWantedInstanceExtensions, avalableNames );


	_mAppInfo = {}; // information about your application and Vulkan compatibility
	_mInstanceCreateInfo = {}; // informations about the specific type of Vulkan instance you wish to create

	_mAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	_mAppInfo.pNext = nullptr;
	_mAppInfo.pApplicationName = "Vulkan Renderer";
	_mAppInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	_mAppInfo.pEngineName = "No Engine";
	_mAppInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	_mAppInfo.apiVersion = VK_MAKE_VERSION( 1, 0, 0 );

	_mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_mInstanceCreateInfo.pNext = nullptr;
	_mInstanceCreateInfo.pApplicationInfo = &_mAppInfo;
	_mInstanceCreateInfo.enabledLayerCount = _mIsDebugBuild ? _mTurnedOnInstanceLayers.size() : 0;
	_mInstanceCreateInfo.ppEnabledLayerNames = _mIsDebugBuild ? _mTurnedOnInstanceLayers.data() : nullptr;
	_mInstanceCreateInfo.enabledExtensionCount = _mTurnedOnInstanceExtensions.size();
	_mInstanceCreateInfo.ppEnabledExtensionNames = _mTurnedOnInstanceExtensions.data();

	returnResult = vkCreateInstance( &_mInstanceCreateInfo,
									 nullptr,
									 &_mVkInstance );

	IfVKErrorPrintMSG( returnResult, "Instance could not be created" );

	return returnResult;
}


VkResult VK_Renderer::SetUpDebugCallback()
{
	if( !_mIsDebugBuild ) return VK_SUCCESS;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback;

	VkResult returnResult = CreateDebugReportCallbackEXT( _mVkInstance, &createInfo, nullptr, &_mDebugCallbackHandle );
	IfVKErrorPrintMSG( returnResult, "Could not set up a debug callback" );
	return returnResult;
}


VkResult VK_Renderer::ChooseAPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	VkResult returnResult = vkEnumeratePhysicalDevices( _mVkInstance,
														&physicalDeviceCount,
														nullptr );

	if( returnResult != VK_SUCCESS || physicalDeviceCount == 0 )
	{
		IfVKErrorPrintMSG( returnResult, "System scan for secondary processors either failed or couldn't find a secondary processor. Terminating." );

		return returnResult;
	}

	vector<VkPhysicalDevice> physicalDevices( physicalDeviceCount );
	returnResult = vkEnumeratePhysicalDevices( _mVkInstance,
											   &physicalDeviceCount,
											   physicalDevices.data() );

	int winningIndex = -1;
	uint64_t highestScore = 0;
	VkPhysicalDeviceProperties winningDeviceProps = {};
	VkPhysicalDeviceProperties tempDeviceProps = {};

	cout << endl << "There are " << physicalDeviceCount << " secondary processor(s) available, they are: " << endl;

	for( unsigned i = 0; i < physicalDevices.size(); i++ )
	{
		uint64_t currentScore = RatePhysicalDeviceForGameGraphics( &physicalDevices.at( i ) );
		vkGetPhysicalDeviceProperties( physicalDevices.at( i ), &tempDeviceProps );
		cout << ( i + 1 ) << " : " << tempDeviceProps.deviceName << " - score: " << currentScore << endl;

		if( currentScore > highestScore )
		{
			highestScore = currentScore;
			winningIndex = i;
			winningDeviceProps = tempDeviceProps;
		}
	}

	if( winningIndex == -1 )
	{
		IfVKErrorPrintMSG( VK_ERROR_INITIALIZATION_FAILED, "No intergrated GPU device was found, Terminating." );
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	cout << "This application will use: " << winningDeviceProps.deviceName << endl << endl;
	_mPhysicalDevice = physicalDevices.at( winningIndex );

	return VK_SUCCESS;
}


uint64_t VK_Renderer::RatePhysicalDeviceForGameGraphics( VkPhysicalDevice* physicalDevice )
{
	uint64_t gpuScore = 0;

	VkPhysicalDeviceProperties deviceProps;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties( *physicalDevice, &deviceProps );
	vkGetPhysicalDeviceFeatures( *physicalDevice, &deviceFeatures );

	if( deviceProps.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || !deviceFeatures.geometryShader )
	{
		return gpuScore;
	}

	gpuScore += deviceProps.limits.maxMemoryAllocationCount;
	gpuScore += deviceProps.limits.maxImageDimension2D;

	return gpuScore;
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
	_mGraphicsQueueDeviceIndex = 0;

	for( unsigned i = 0; i < queueFamilyCount; i++ )
	{
		cout << VK_QUEUE_GRAPHICS_BIT << " - " << familyProperties.at( i ).queueFlags;

		if( VK_QUEUE_GRAPHICS_BIT & familyProperties.at( i ).queueFlags )
		{
			cout << " : This is the graphics queue." << endl;
			_mGraphicsQueueDeviceIndex = i;
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
	deviceQueueCreateInfo.queueFamilyIndex = _mGraphicsQueueDeviceIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;


	// Get the available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties( _mPhysicalDevice, nullptr, &extensionCount, nullptr );
	cout << endl << "We have " << extensionCount << " extensions on the chosen graphics card. They are:" << endl;
	vector<VkExtensionProperties> vkDeviceExtensionProps( extensionCount );
	vkEnumerateDeviceExtensionProperties( _mPhysicalDevice, nullptr, &extensionCount, vkDeviceExtensionProps.data() );


	vector<const char*> avalableNames;
	for( unsigned i = 0; i < vkDeviceExtensionProps.size(); i++ )
	{
		avalableNames.push_back( vkDeviceExtensionProps.at( i ).extensionName );
	}
	_mTurnedOnDeviceExtensions = FindCommonCStrings( _mWantedDeviceExtensions, avalableNames );


	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = _mTurnedOnDeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = _mTurnedOnDeviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &requiredFeatures;


	returnResult = vkCreateDevice( _mPhysicalDevice, &deviceCreateInfo, nullptr, &_mLogicalDevice );

	vkGetDeviceQueue( _mLogicalDevice, _mGraphicsQueueDeviceIndex, 0, &_mGraphicsQueue );

	return returnResult;
}


VkResult VK_Renderer::InitVulkanGraphicalPipeline()
{
	VkResult returnResult = VK_SUCCESS;
	if( VK_SUCCESS != ( returnResult = InitSwapChain() ) )				return returnResult;
	if( VK_SUCCESS != ( returnResult = InitGraphicsQueue() ) )			return returnResult;
	if( VK_SUCCESS != ( returnResult = InitFrameBuffers() ) )			return returnResult;
	if( VK_SUCCESS != ( returnResult = InitRenderFences() ) )			return returnResult;
	if( VK_SUCCESS != ( returnResult = InitVertexBuffer() ) )			return returnResult;

	return returnResult;
}


VkResult VK_Renderer::InitSwapChain()
{
	VkResult returnResult;

	uint32_t surfaceFormatsCount;
	_mDeviceSurfaceFormats = {};
	returnResult = vkGetPhysicalDeviceSurfaceFormatsKHR( _mPhysicalDevice, _mWindowSurface, &surfaceFormatsCount, &_mDeviceSurfaceFormats );
	if( IfVKErrorPrintMSG( returnResult, "Couldn't get Device's Surface Format for the Swaph Chain's Create info." ) ) return returnResult;

	uint32_t presentModeCount;
	_mPresentModeKHR = {};
	returnResult = vkGetPhysicalDeviceSurfacePresentModesKHR( _mPhysicalDevice, _mWindowSurface, &presentModeCount, &_mPresentModeKHR );
	if( IfVKErrorPrintMSG( returnResult, "Couldn't get Device's Present Modes for the Swaph Chain's Create info." ) ) return returnResult;

	_mSurfaceCapabilities = {};
	returnResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( _mPhysicalDevice, _mWindowSurface, &_mSurfaceCapabilities );
	if( IfVKErrorPrintMSG( returnResult, "Couldn't get Device's Surface Capabilities for the Swaph Chain's Create info." ) ) return returnResult;
	VkExtent2D surfaceResolution = _mSurfaceCapabilities.currentExtent;
	uint32_t width = surfaceResolution.width;
	uint32_t height = surfaceResolution.height;

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = _mWindowSurface;
	swapChainCreateInfo.minImageCount = _mSwapChainSize;
	swapChainCreateInfo.imageFormat = _mDeviceSurfaceFormats.format;
	swapChainCreateInfo.imageColorSpace = _mDeviceSurfaceFormats.colorSpace;
	swapChainCreateInfo.imageExtent = surfaceResolution;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = _mPresentModeKHR;
	swapChainCreateInfo.clipped = true;
	swapChainCreateInfo.oldSwapchain = NULL;

	returnResult = vkCreateSwapchainKHR( _mLogicalDevice, &swapChainCreateInfo, 0, &_mSwapChainHandle );

	if( returnResult == VK_SUCCESS )
	{
		uint32_t newImageCount;
		vkGetSwapchainImagesKHR( _mLogicalDevice, _mSwapChainHandle, &newImageCount, nullptr );

		if( _mSwapChainSize != newImageCount )
		{
			cout << "ERROR: shift in image count from " << _mSwapChainSize << " to " << newImageCount << endl;
			return VK_ERROR_VALIDATION_FAILED_EXT;
		}

		_mSwapChainImages.resize( _mSwapChainSize );
		_mSwapChainImageViews.resize( _mSwapChainSize );
		returnResult = vkGetSwapchainImagesKHR( _mLogicalDevice, _mSwapChainHandle, &newImageCount, _mSwapChainImages.data() );

		if( returnResult == VK_SUCCESS )
		{
			for( auto i = 0; i < _mSwapChainSize; i++ )
			{
				VkImageViewCreateInfo imageViewCreateInfo = {};
				imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // render to a 2D image
				imageViewCreateInfo.image = _mSwapChainImages.at( i ); // passing by reference
				imageViewCreateInfo.format = _mDeviceSurfaceFormats.format;
				imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
				imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
				imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
				imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
				imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
				imageViewCreateInfo.subresourceRange.levelCount = 1;
				imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				imageViewCreateInfo.subresourceRange.layerCount = 1;

				returnResult = vkCreateImageView( _mLogicalDevice, &imageViewCreateInfo, NULL, &_mSwapChainImageViews.at( i ) );
			}

			if( returnResult == VK_SUCCESS )
			{
				cout << endl << "Swap Chain initialised correctly" << endl << endl;
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


VkResult VK_Renderer::InitGraphicsQueue()
{
	VkResult returnResult = VK_SUCCESS;
	// _mGraphicsQueue fetched with vkGetDeviceQueue in initLogicalDevice

	VkCommandPoolCreateInfo cmdPoolCreateInfo;
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = _mGraphicsQueueDeviceIndex;

	returnResult = vkCreateCommandPool( _mLogicalDevice, &cmdPoolCreateInfo, NULL, &_mGraphicsQueueCmdPool );

	if( returnResult != VK_SUCCESS )
	{
		cout << "Graphics command pool could not be initialised." << endl;
		return returnResult;
	}

	VkCommandBufferAllocateInfo cmdBufferAllocInfo;
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = _mGraphicsQueueCmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1; // Test significance

	returnResult = vkAllocateCommandBuffers( _mLogicalDevice, &cmdBufferAllocInfo, &_mGraphicsQueueCmdBuffer );

	if( returnResult != VK_SUCCESS )
	{
		cout << "Graphics command buffer could not be initialised." << endl;
		return returnResult;
	}

	cout << "Graphics command pool & buffer initialised." << endl;
	return returnResult;
}


VkResult VK_Renderer::InitFrameBuffers()
{
	VkResult returnResult = VK_SUCCESS;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = _mDeviceSurfaceFormats.format; // needs the same format as what it will render to which is equal to what what the graphics card is compatible with.
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // render pixels multiple times
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference attachmentRef = {};
	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &attachmentRef;
	subPass.pDepthStencilAttachment = nullptr;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subPass;

	returnResult = vkCreateRenderPass( _mLogicalDevice, &renderPassCreateInfo, nullptr, &_mRenderPass );

	if( returnResult != VK_SUCCESS )
	{
		cout << "We were not able to create a render pass." << endl;
		return returnResult;
	}
	cout << "Render pass created" << endl;

	_mSwapChainFrameBuffers.resize( _mSwapChainSize );

	for( auto i = 0; i < _mSwapChainSize; i++ )
	{
		VkImageView attachments[] = {
			_mSwapChainImageViews.at( i )
		};

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.renderPass = _mRenderPass; // parsing by reference
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = _mWidth;
		frameBufferCreateInfo.height = _mHeight;
		frameBufferCreateInfo.layers = 1;

		returnResult = vkCreateFramebuffer( _mLogicalDevice, &frameBufferCreateInfo, NULL, &_mSwapChainFrameBuffers.at( i ) );

		if( returnResult != VK_SUCCESS )
		{
			cout << "FrameBuffer " << i << " was not initialised properly" << endl;
		}
	}

	return returnResult;
}


VkResult VK_Renderer::InitRenderFences()
{
	VkResult returnResult;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	_mSwapChainRenderFences.resize( _mSwapChainSize );
	for( auto i = 0; i < _mSwapChainSize; i++ )
	{
		returnResult = vkCreateFence( _mLogicalDevice, &fenceCreateInfo, NULL, &_mSwapChainRenderFences.at( i ) );
		IfVKErrorPrintMSG( returnResult, "Fence " + to_string( i ) + " Faled to be initialised" );
	}

	return returnResult;
}


VkResult VK_Renderer::InitVertexBuffer()
{
	VkResult returnResult;

	vertex vert1 = {};
	vert1.pos.x = 1.0f;
	vert1.colour.r = 1.0f;
	vertex vert2 = {};
	vert2.pos.x = -1.0f;
	vert2.colour.g = 1.0f;
	vertex vert3 = {};
	vert3.pos.z = 1.0f;
	vert3.colour.b = 1.0f;
	vector<vertex> mesh1;
	mesh1.push_back( vert1 );
	mesh1.push_back( vert2 );
	mesh1.push_back( vert3 );
	_mVertexBufferData.push_back( mesh1 );


	VkBufferCreateInfo vBufferCreateInfo = {};
	vBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferCreateInfo.size = sizeof( _mVertexBufferData );
	vBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vBufferCreateInfo.queueFamilyIndexCount = 0;
	vBufferCreateInfo.pQueueFamilyIndices = nullptr;

	returnResult = vkCreateBuffer( _mLogicalDevice, &vBufferCreateInfo, nullptr, &_mVertexBuffer );
	if( IfVKErrorPrintMSG( returnResult, "Could not initialise a vertexBuffer complete with data." ) ) return returnResult;

	VkMemoryRequirements vBufferMemReq;
	vkGetBufferMemoryRequirements( _mLogicalDevice, _mVertexBuffer, &vBufferMemReq );
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties( _mPhysicalDevice, &memProps );

	VkMemoryAllocateInfo vBufferAllocInfo = {};
	vBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vBufferAllocInfo.pNext = nullptr;
	vBufferAllocInfo.allocationSize = vBufferMemReq.size;

	for( auto i = 0; i < VK_MAX_MEMORY_TYPES; i++ )
	{
		VkMemoryType memType = memProps.memoryTypes[ i ];

		if( memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
		{
			vBufferAllocInfo.memoryTypeIndex = i;
		}
	}


	returnResult = vkAllocateMemory( _mLogicalDevice, &vBufferAllocInfo, nullptr, &_mVertexBufferMemory );
	if( IfVKErrorPrintMSG( returnResult, "Couldn't create memory for the Vertex Buffer." ) ) return returnResult;

	void* mappedData;
	//returnResult = vkMapMemory( _mLogicalDevice, vBufferMemory, 0, VK_WHOLE_SIZE, 0, &mappedData );
	returnResult = vkMapMemory( _mLogicalDevice, _mVertexBufferMemory, 0, vBufferCreateInfo.size, 0, &mappedData );
	if( IfVKErrorPrintMSG( returnResult, "Could not map the Vertex Buffer Memory." ) ) return returnResult;

	memcpy( mappedData, _mVertexBufferData.data(), ( size_t ) vBufferCreateInfo.size );

	vkUnmapMemory( _mLogicalDevice, _mVertexBufferMemory );
	returnResult = vkBindBufferMemory( _mLogicalDevice, _mVertexBuffer, _mVertexBufferMemory, 0 );
	IfVKErrorPrintMSG( returnResult, "Could not bind the Vertex Buffer to the GPU" );

	return returnResult;
}



// Needs to be made a part of the core game engine as the renderer shouldn't have responcibility over window and input management.
void VK_Renderer::CreateGLFWWindow()
{
	// Creates a window "without a context" - Go here to find out more: http://www.glfw.org/docs/latest/context_guide.html#context_less
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	_mWindow = glfwCreateWindow( _mWidth, _mHeight, "Vulkan Renderer", NULL, NULL );

	/*glfwMakeContextCurrent( _mWindow );
	glfwSetInputMode( _mWindow, GLFW_STICKY_KEYS, VK_TRUE );
	glfwSetInputMode( _mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );*/
}


VkResult VK_Renderer::InitialiseWindowSurface()
{
	// Creating a Vulkan window surface
	VkBool32 state;
	VkResult returnResult;
	returnResult = glfwCreateWindowSurface( _mVkInstance, _mWindow, NULL, &_mWindowSurface );
	if( returnResult != VK_SUCCESS )
	{
		cout << "Could not create a surface in which to draw. VK_ERROR: " << returnResult << endl;
	}

	returnResult = vkGetPhysicalDeviceSurfaceSupportKHR( _mPhysicalDevice, _mGraphicsQueueDeviceIndex, _mWindowSurface, &state );
	if( returnResult != VK_SUCCESS )
	{
		cout << "Could not Validate surface with GPU format. VK_ERROR: " << returnResult << endl;
	}

	return returnResult;
}






void VK_Renderer::GameLoop()
{
	//glfwSetWindowCloseCallback( _mWindow, window_close_callback );
	//RenderScene();
	while( glfwGetKey( _mWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( _mWindow ) == 0 )
	{
		RenderScene();
		glfwPollEvents();
	}
	vkDeviceWaitIdle( _mLogicalDevice );
}


void VK_Renderer::RenderScene()
{
	cout << endl << endl << "NEXT_RENDER_PASS" << endl;

	//UINT_MAX
	//UINT64_MAX
	//vkAcquireNextImageKHR( _mLogicalDevice, _mSwapChainHandle, UINT64_MAX, VK_NULL_HANDLE, _mSwapChainRenderFences.at(_mChainNextImageIndex), &_mChainNextImageIndex );
	_mChainNextImageIndex == 0 ? _mChainNextImageIndex = 1 : _mChainNextImageIndex = 0;
	cout << _mChainNextImageIndex;

	VkCommandBufferBeginInfo newBufferInfo = {};
	newBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	newBufferInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer( _mGraphicsQueueCmdBuffer, &newBufferInfo );
	{
		VkClearValue cv = {};
		cv.color.float32[ 0 ] = 1; // R
		cv.color.float32[ 1 ] = 0; // G
		cv.color.float32[ 2 ] = 0; // B
		cv.color.float32[ 3 ] = 1; // A

		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = ( uint32_t ) 0.0f;
		VkClearValue clearImageValue[] = { cv };

		VkOffset2D start = {};

		VkExtent2D dimensions;
		dimensions.height = _mHeight;
		dimensions.width = _mWidth;

		VkRect2D rect;
		rect.offset = start;
		rect.extent = dimensions;

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = _mRenderPass;
		renderPassBeginInfo.framebuffer = _mSwapChainFrameBuffers.at( _mChainNextImageIndex );
		renderPassBeginInfo.renderArea = rect;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearImageValue;

		vkCmdBeginRenderPass( _mGraphicsQueueCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_END_RANGE );

		// Custom render code starts here

		// Custom render code stops here

		vkCmdEndRenderPass( _mGraphicsQueueCmdBuffer );



		// for submitting the Fence to the Queue
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
		submitInfo.pWaitDstStageMask = NULL;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_mGraphicsQueueCmdBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

		vkEndCommandBuffer( _mGraphicsQueueCmdBuffer );

		// submit the all the above work to the GPU queue
		vkQueueSubmit( _mGraphicsQueue, 1, &submitInfo, _mSwapChainRenderFences.at( _mChainNextImageIndex ) );

		vkWaitForFences( _mLogicalDevice, 1, &_mSwapChainRenderFences.at( _mChainNextImageIndex ), VK_TRUE, UINT64_MAX );

		vkResetFences( _mLogicalDevice, 1, &_mSwapChainRenderFences.at( _mChainNextImageIndex ) );
	}
}