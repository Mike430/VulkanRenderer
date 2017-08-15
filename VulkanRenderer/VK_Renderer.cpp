#include "VK_Renderer.h"



VK_Renderer::VK_Renderer()
{
	Utilities::LogInfoIfDebug( "VK_Renderer constructor called" );

	//_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_api_dump" ); // Doesn't show errors, just shows the evolution of your vulkan app by dumping every key change that occures
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_core_validation" ); // Logs every error that occures
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_monitor" );
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_object_tracker" ); // tells you have you haven't deleted when cleaning up
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_parameter_validation" );
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_screenshot" );
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_swapchain" ); // specifically targets swapchain issues
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_GOOGLE_threading" );
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_GOOGLE_unique_objects" );
	//_mWantedInstanceLayers.push_back( "VK_LAYER_LUNARG_vktrace" ); // breaks my application for some reason at instance initialisation
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_NV_optimus" );
	//_mWantedInstanceLayers.push_back( "VK_LAYER_RENDERDOC_Capture" ); // fix pipeline and this should work with renderdoc
	//_mWantedInstanceLayers.push_back( "VK_LAYER_VALVE_steam_overlay" );
	_mWantedInstanceAndDeviceLayers.push_back( "VK_LAYER_LUNARG_standard_validation" ); // additional error loggings

	// Declare a lists of all extensions we want to feed into our Vulkan Instance and Device
	_mWantedInstanceExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
	_mWantedInstanceExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
#if defined(VK_USE_PLATFORM_WIN32_KHR) // Vulkan suface extension for windows specifically - nothing to do with 32 & 64 bit windows OSs
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
	Utilities::LogInfoIfDebug( "VK_Renderer destructor called" );


	// Shutdown Vulkan
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

	Utilities::LogInfoIfDebug( "VK_Renderer destructor completed" );
}

// returns true if we threw an error
bool VK_Renderer::IfVKErrorPrintMSG( VkResult VkState, string output )
{
	bool hasFailed = VkState != VK_SUCCESS;
	if( hasFailed )
	{
		Utilities::LogErrorIfDebug( _mVkReportPrefix + output );
	}

	return hasFailed;
}

// returns true if we threw an error
bool VK_Renderer::IfVKErrorPrintMSG( VkResult VkState, string errOutput, string successOutput )
{
	bool hasFailed = VkState != VK_SUCCESS;
	if( hasFailed )
	{
		Utilities::LogErrorIfDebug( _mVkReportPrefix + errOutput );
	}
	else
	{
		Utilities::LogSuccessIfDebug( _mVkReportPrefix + successOutput );
	}

	return hasFailed;
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
	Utilities::LogWarningIfDebug( "Debug Callback was called, message reads: " + ( ( string ) layerPrefix ) + " -> " + ( ( string ) message ) );
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
		Utilities::LogErrorIfDebug( "GLFW couldn't be initialised" );
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	CreateGLFWWindow();
	return InitVulkanRenderer();
}


VkResult VK_Renderer::InitVulkanRenderer()
{
	VkResult returnResult = VK_SUCCESS;

	returnResult = InitInstance();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise instance.",
						   "Initialised instance." ) )						return returnResult;

	returnResult = SetUpDebugCallback();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to setup debug.",
						   "Initialised debug." ) )							return returnResult;

	returnResult = InitialiseWindowSurface();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise surface.",
						   "Surface initialised." ) )						return returnResult;

	returnResult = ChooseAPhysicalDevice();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to choose a device.",
						   "Physical device chosen." ) )					return returnResult;

	returnResult = InitLogicalDevice();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise logical device.",
						   "Logical device initialised." ) )				return returnResult;

	returnResult = InitSwapChain();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise swap chain.",
						   "Swap chain initialised." ) )					return returnResult;

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
	if( Utilities::buildIsDebug )
	{
		for( unsigned i = 0; i < vkLayerProps.size(); i++ )
		{
			avalableNames.push_back( vkLayerProps.at( i ).layerName );
		}
		_mTurnedOnInstanceLayers = Utilities::FindCommonCStrings( _mWantedInstanceAndDeviceLayers, avalableNames );
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
	_mTurnedOnInstanceExtensions = Utilities::FindCommonCStrings( _mWantedInstanceExtensions, avalableNames );


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
	_mInstanceCreateInfo.enabledLayerCount = Utilities::buildIsDebug ? _mTurnedOnInstanceLayers.size() : 0;
	_mInstanceCreateInfo.ppEnabledLayerNames = Utilities::buildIsDebug ? _mTurnedOnInstanceLayers.data() : nullptr;
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
	if( !Utilities::buildIsDebug ) return VK_SUCCESS;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback;

	VkResult returnResult = CreateDebugReportCallbackEXT( _mVkInstance, &createInfo, nullptr, &_mDebugCallbackHandle );
	IfVKErrorPrintMSG( returnResult, "Could not set up a debug callback" );
	return returnResult;
}


VkResult VK_Renderer::InitialiseWindowSurface()
{
	return glfwCreateWindowSurface( _mVkInstance, _mWindow, nullptr, &_mWindowSurface );
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
	DeviceQueueFamilyIndexes indicies;
	DeviceQueueFamilyIndexes finalIndicies;
	VkPhysicalDeviceProperties winningDeviceProps = {};
	VkPhysicalDeviceProperties tempDeviceProps = {};

	cout << endl << "There are " << physicalDeviceCount << " secondary processor(s) available, they are: " << endl;

	for( unsigned i = 0; i < physicalDevices.size(); i++ )
	{
		indicies = FindDeviceQueueFamilies( &physicalDevices.at( i ) );
		bool isSuitable = indicies.HasAllNeededQueues();
		uint64_t currentScore = RatePhysicalDeviceForGameGraphics( &physicalDevices.at( i ) );
		vkGetPhysicalDeviceProperties( physicalDevices.at( i ), &tempDeviceProps );
		cout << ( i + 1 ) << " : " << tempDeviceProps.deviceName << " - score: " << currentScore << " - Has req queues: " << ( isSuitable ? "true" : "false" ) << endl;

		if( currentScore > highestScore && isSuitable )
		{
			highestScore = currentScore;
			winningIndex = i;
			winningDeviceProps = tempDeviceProps;
			finalIndicies = indicies;
		}
	}

	if( winningIndex == -1 )
	{
		IfVKErrorPrintMSG( VK_ERROR_INITIALIZATION_FAILED, "No intergrated GPU device was found, Terminating." );
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	cout << "Chosen GPU: " << winningDeviceProps.deviceName << endl;
	_mPhysicalDevice = physicalDevices.at( winningIndex );
	_mPhysicalDeviceQueueFamilyIndexes = finalIndicies;

	return VK_SUCCESS;
}


VkResult VK_Renderer::InitLogicalDevice()
{
	VkResult returnResult;

	VkPhysicalDeviceFeatures requiredFeatures = {};
	requiredFeatures.tessellationShader = VK_TRUE;
	requiredFeatures.geometryShader = VK_TRUE;
	requiredFeatures.samplerAnisotropy = VK_TRUE;

	float queuePriority = 1.0f;
	vector<int> deviceQueueIndexes;
	// Vulkan preferes to work with unique index identifiers.
	if( _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex != _mPhysicalDeviceQueueFamilyIndexes._mPresentFamilyIndex )
	{
		deviceQueueIndexes = { _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex, _mPhysicalDeviceQueueFamilyIndexes._mPresentFamilyIndex };
	}
	else
	{
		deviceQueueIndexes = { _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex };
	}

	vector<VkDeviceQueueCreateInfo> queuesForLogicalDevice;

	for( auto i = 0; i < deviceQueueIndexes.size(); i++ )
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo;
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pNext = nullptr;
		deviceQueueCreateInfo.flags = 0;
		deviceQueueCreateInfo.queueFamilyIndex = deviceQueueIndexes.at( i );
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

		queuesForLogicalDevice.push_back( deviceQueueCreateInfo );
	}

	// Get the available layers
	uint32_t layerCount;
	vkEnumerateDeviceLayerProperties( _mPhysicalDevice, &layerCount, nullptr );
	cout << endl << "We have " << layerCount << " layers on the chose graphics card. They are:" << endl;
	vector<VkLayerProperties> vkDeviceLayerProps( layerCount );
	vkEnumerateDeviceLayerProperties( _mPhysicalDevice, &layerCount, vkDeviceLayerProps.data() );

	vector<const char*> avalableNames;
	for( unsigned i = 0; i < vkDeviceLayerProps.size(); i++ )
	{
		avalableNames.push_back( vkDeviceLayerProps.at( i ).layerName );
	}
	_mTurnedOnDeviceLayers = Utilities::FindCommonCStrings( _mWantedInstanceAndDeviceLayers, avalableNames );

	// Get the available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties( _mPhysicalDevice, nullptr, &extensionCount, nullptr );
	cout << endl << "We have " << extensionCount << " extensions on the chosen graphics card. They are:" << endl;
	vector<VkExtensionProperties> vkDeviceExtensionProps( extensionCount );
	vkEnumerateDeviceExtensionProperties( _mPhysicalDevice, nullptr, &extensionCount, vkDeviceExtensionProps.data() );


	for( unsigned i = 0; i < vkDeviceExtensionProps.size(); i++ )
	{
		avalableNames.push_back( vkDeviceExtensionProps.at( i ).extensionName );
	}
	_mTurnedOnDeviceExtensions = Utilities::FindCommonCStrings( _mWantedDeviceExtensions, avalableNames );


	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = static_cast< uint32_t >( queuesForLogicalDevice.size() );
	deviceCreateInfo.pQueueCreateInfos = queuesForLogicalDevice.data();
	deviceCreateInfo.enabledLayerCount = Utilities::buildIsDebug ? _mTurnedOnDeviceLayers.size() : 0;
	deviceCreateInfo.ppEnabledLayerNames = Utilities::buildIsDebug ? _mTurnedOnDeviceLayers.data() : nullptr;
	deviceCreateInfo.enabledExtensionCount = _mTurnedOnDeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = _mTurnedOnDeviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &requiredFeatures;

	returnResult = vkCreateDevice( _mPhysicalDevice, &deviceCreateInfo, nullptr, &_mLogicalDevice );

	vkGetDeviceQueue( _mLogicalDevice, _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex, 0, &_mGraphicsQueue );
	vkGetDeviceQueue( _mLogicalDevice, _mPhysicalDeviceQueueFamilyIndexes._mPresentFamilyIndex, 0, &_mPresentQueue );

	return returnResult;
}


VkResult VK_Renderer::InitSwapChain()
{
	VkResult returnResult;

	SwapChainSupportDetails swapChainSupport = QueryDeviceSwapChainSupport( &_mPhysicalDevice );

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat( swapChainSupport._mAvailableFormats );
	VkPresentModeKHR presentMode = ChooseSwapChainPresentationMode( swapChainSupport._mAvailablePresentModes );
	VkExtent2D extent = ChooseSwapChainExtentionDimensions( swapChainSupport._mCapabilities );

	uint32_t imageCount = swapChainSupport._mCapabilities.minImageCount + 1;// 3 in most cases and we want to aim for triple buffering
	if( swapChainSupport._mCapabilities.maxImageCount > 0 && imageCount > swapChainSupport._mCapabilities.maxImageCount )
	{
		imageCount = swapChainSupport._mCapabilities.maxImageCount;// Use if we can't have more than the max
		Utilities::LogWarningIfDebug( "Can't do tripple buffering" );
	}
	Utilities::LogInfoIfDebug( "SwapChain length = " + imageCount );

	uint32_t queueFamilyIndices[] = {
		( uint32_t ) _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex,
		( uint32_t ) _mPhysicalDeviceQueueFamilyIndexes._mPresentFamilyIndex
	};

	// Building the Swapchain:
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _mWindowSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;// Depth: use 1 unless you're making stereoscopic 3d images
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Variants for the create info
	if( _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex != _mPhysicalDeviceQueueFamilyIndexes._mPresentFamilyIndex )
	{
		Utilities::LogInfoIfDebug( "QueueFamily IS NOT PresentFamily" );
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		Utilities::LogInfoIfDebug( "QueueFamily IS THE SAME AS PresentFamily" );
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	// Interesting points for the Swap Chain create info
	createInfo.preTransform = swapChainSupport._mCapabilities.currentTransform;// Flipping and rotating images
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;// making images transparent so you can see through the window
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;// don't draw pixels if another window is covering them up - performant
	createInfo.oldSwapchain = VK_NULL_HANDLE;// TODO: If the window is resized, this swap chain is invalid and will need to be referenced for the new one.


	// Creating the Swapchain
	returnResult = vkCreateSwapchainKHR( _mLogicalDevice, &createInfo, nullptr, &_mSwapChainHandle );


	// Retrive Swapchain images
	vkGetSwapchainImagesKHR( _mLogicalDevice, _mSwapChainHandle, &imageCount, nullptr );
	_mSwapChainImages = vector<VkImage>( imageCount );
	vkGetSwapchainImagesKHR( _mLogicalDevice, _mSwapChainHandle, &imageCount, _mSwapChainImages.data() );

	// store new Swapchain details for remaking
	_mSwapChainFormat = surfaceFormat.format;
	_mSwapChainExtent = extent;

	return returnResult;
}


// Utilities
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


DeviceQueueFamilyIndexes VK_Renderer::FindDeviceQueueFamilies( VkPhysicalDevice* physicalDevice )
{
	DeviceQueueFamilyIndexes indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( *physicalDevice, &queueFamilyCount, nullptr );
	vector<VkQueueFamilyProperties> queueFamilyProps( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( *physicalDevice, &queueFamilyCount, queueFamilyProps.data() );

	for( auto i = 0; i < queueFamilyCount; i++ )
	{
		if( queueFamilyProps.at( i ).queueCount > 0 && queueFamilyProps.at( i ).queueFlags & VK_QUEUE_GRAPHICS_BIT )
		{
			indices._mGraphicsFamilyIndex = i;
		}

		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( *physicalDevice, i, _mWindowSurface, &presentationSupport );

		if( queueFamilyProps.at( i ).queueCount > 0 && presentationSupport )
		{
			indices._mPresentFamilyIndex = i;
		}

		if( indices.HasAllNeededQueues() )
		{
			break;
		}
	}

	return indices;
}


SwapChainSupportDetails VK_Renderer::QueryDeviceSwapChainSupport( VkPhysicalDevice* physicalDevice )
{
	SwapChainSupportDetails queryResults;

	// Querying capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( *physicalDevice, _mWindowSurface, &queryResults._mCapabilities );

	// Querying the available formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR( *physicalDevice, _mWindowSurface, &formatCount, nullptr );
	if( formatCount > 0 )
	{
		queryResults._mAvailableFormats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( *physicalDevice, _mWindowSurface, &formatCount, queryResults._mAvailableFormats.data() );
	}

	// Querying the available presentation modes 
	uint32_t presentationModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( *physicalDevice, _mWindowSurface, &presentationModesCount, nullptr );
	if( presentationModesCount > 0 )
	{
		queryResults._mAvailablePresentModes.resize( presentationModesCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( *physicalDevice, _mWindowSurface, &presentationModesCount, queryResults._mAvailablePresentModes.data() );
	}

	return queryResults;
}


VkSurfaceFormatKHR VK_Renderer::ChooseSwapChainSurfaceFormat( vector<VkSurfaceFormatKHR> availableFormats )
{
	if( availableFormats.size() == 1 && availableFormats[ 0 ].format == VK_FORMAT_UNDEFINED )
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for( const auto& availableFormat : availableFormats )
	{
		if( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			return availableFormat;
		}
	}

	return availableFormats[ 0 ];
}


VkPresentModeKHR VK_Renderer::ChooseSwapChainPresentationMode( vector<VkPresentModeKHR> availableModes )
{
	/*
	VK_PRESENT_MODE_IMMEDIATE_KHR - images are transfered immediatly - may result in tearing
	VK_PRESENT_MODE_FIFO_KHR - images are queued and are only taken to the screen when the screen refreshes - similar to vsync
	VK_PRESENT_MODE_FIFO_RELAXED_KHR - just like the last one except images can be taken prematurley if the queue is empty resulting in tearing
	VK_PRESENT_MODE_MAILBOX_KHR - just like the second one except if the queue is full it overwrites the last image rather than blocking the application.
	*/

	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for( const auto& availablePresentMode : availableModes )
	{
		if( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
		{
			return availablePresentMode;
		}
		else if( availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}


VkExtent2D VK_Renderer::ChooseSwapChainExtentionDimensions( VkSurfaceCapabilitiesKHR capabilities )
{
	if( capabilities.currentExtent.width != UINT32_MAX )
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { _mWidth, _mHeight };

		actualExtent.width = max( capabilities.minImageExtent.width, min( capabilities.maxImageExtent.width, actualExtent.width ) );
		actualExtent.height = max( capabilities.minImageExtent.height, min( capabilities.maxImageExtent.height, actualExtent.height ) );

		return actualExtent;
	}
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


// Finale
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
}