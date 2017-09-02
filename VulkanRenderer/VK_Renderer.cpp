#include "VK_Renderer.h"



const string VK_Renderer::_mVkReportPrefix = "Vk_Renderer Report: ";


VK_Renderer::VK_Renderer( vector<Vertex> renderableMeshVerts )
{
	Utilities::LogInfoIfDebug( "VK_Renderer constructor called" );

	_mVertices = renderableMeshVerts;

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
	CleanSwapChainResources();
	vkDestroyBuffer( _mLogicalDevice, _mVertexBuffer, nullptr );
	vkFreeMemory( _mLogicalDevice, _mVertexBufferMemory, nullptr );
	vkDestroySemaphore( _mLogicalDevice, _mImageAvailableSemaphore, nullptr );
	vkDestroySemaphore( _mLogicalDevice, _mRenderFinishedSemaphore, nullptr );
	vkDestroyCommandPool( _mLogicalDevice, _mCommandPool, nullptr );
	vkDestroySurfaceKHR( _mVkInstance, _mWindowSurface, nullptr );
	vkDestroyDevice( _mLogicalDevice, nullptr );
	DestroyDebugReportCallbackEXT( _mVkInstance, _mDebugCallbackHandle, nullptr );
	vkDestroyInstance( _mVkInstance, nullptr );

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
	Utilities::LogErrorIfDebug( "Debug Callback was called, message reads: " + ( ( string ) layerPrefix ) + " -> " + ( ( string ) message ) );
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
	_mWinManager = WindowManager::GetInstance();
	_mWinManager->CreateNewWindow( this, VK_TRUE, _mWidth, _mHeight, VK_Renderer::OnWindowResize );
	_mWindow = _mWinManager->GetWindow();

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

	returnResult = InitImageViews();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise image views.",
						   "Image views initialised." ) )					return returnResult;

	// Before the image views can become render targets (FrameBuffers) we need to set up the pipeline

	returnResult = InitRenderPasses();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise the render passes.",
						   "Render passes initialised." ) )					return returnResult;

	returnResult = InitGraphicsPipeline();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise graphics pipeline.",
						   "Graphics pipeline initialised." ) )				return returnResult;

	returnResult = InitFrameBuffers();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise frame buffers.",
						   "Frame buffers initialised." ) )					return returnResult;

	returnResult = InitCommandPool();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise command pool.",
						   "Command pool initialised." ) )					return returnResult;

	returnResult = InitVertexBuffer();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise vertex buffer.",
						   "Vertex Buffer initialised." ) )					return returnResult;

	returnResult = InitCommandBuffers();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise command buffers.",
						   "Command buffers initialised." ) )				return returnResult;

	returnResult = InitSemaphores();
	if( IfVKErrorPrintMSG( returnResult,
						   "Failed to initialise semaphores.",
						   "Semaphores initialised." ) )					return returnResult;

	return returnResult;
}


VkResult VK_Renderer::InitInstance()
{
	VkResult returnResult;

	// Layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
	Utilities::LogIfDebug( "We have " + to_string( layerCount ) + " layers available to the instance. They are:", COLOUR_WHITE );
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
	Utilities::LogIfDebug( "We have " + to_string( extensionCount ) + " extensions available to the instance. They are:", COLOUR_WHITE );
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

	Utilities::LogIfDebug( "There are " + to_string( physicalDeviceCount ) + " secondary processor(s) available, they are:", COLOUR_WHITE );

	for( unsigned i = 0; i < physicalDevices.size(); i++ )
	{
		indicies = FindDeviceQueueFamilies( &physicalDevices.at( i ) );
		bool isSuitable = indicies.HasAllNeededQueues();
		uint64_t currentScore = RatePhysicalDeviceForGameGraphics( &physicalDevices.at( i ) );
		vkGetPhysicalDeviceProperties( physicalDevices.at( i ), &tempDeviceProps );
		Utilities::LogIfDebug( to_string( i + 1 ) + " : " + tempDeviceProps.deviceName + " - score: " + to_string( currentScore ) + " - Has req queues: " + ( isSuitable ? "true" : "false" ), COLOUR_WHITE );

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

	Utilities::LogIfDebug( "Chosen GPU: " + ( string ) winningDeviceProps.deviceName, COLOUR_WHITE );
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

	for( size_t i = 0; i < deviceQueueIndexes.size(); i++ )
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
	Utilities::LogIfDebug( "We have " + to_string( layerCount ) + " layers on the chose graphics card. They are:", COLOUR_WHITE );
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
	Utilities::LogIfDebug( "We have " + to_string( extensionCount ) + " extensions on the chosen graphics card. They are:", COLOUR_WHITE );
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
		//Utilities::LogWarningIfDebug( "Can't do tripple buffering" );
	}
	//Utilities::LogInfoIfDebug( "SwapChain length = " + to_string( imageCount ) );

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
		//Utilities::LogInfoIfDebug( "The GPU's QueueFamily IS NOT PresentFamily" );
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		//Utilities::LogInfoIfDebug( "The GPU's QueueFamily IS THE SAME AS PresentFamily" );
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
	_mSwapChainImageFormat = surfaceFormat.format;
	_mSwapChainImageExtent = extent;

	return returnResult;
}


VkResult VK_Renderer::InitImageViews()
{
	VkResult returnResult;

	_mSwapChainImageViews = vector<VkImageView>( _mSwapChainImages.size() );

	for( size_t i = 0; i < _mSwapChainImages.size(); i++ )
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _mSwapChainImages[ i ];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _mSwapChainImageFormat;

		// alter for monochrome images
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Describing the image's purpose for external accessing
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		returnResult = vkCreateImageView( _mLogicalDevice,
										  &createInfo,
										  nullptr,
										  &_mSwapChainImageViews[ i ] );
		if( returnResult != VK_SUCCESS )
		{
			Utilities::LogErrorIfDebug( "Image view " + to_string( i + 1 ) + " could not be created successfully!" );
			break;
		}
	}


	return returnResult;
}


VkResult VK_Renderer::InitRenderPasses()
{
	VkResult returnResult;

	// The main render pass's create info
	VkAttachmentDescription colorAttachmentInfo = {};
	colorAttachmentInfo.format = _mSwapChainImageFormat;
	colorAttachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	// What to do with the image data before drawing
	colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// What to do with the image data after drawing
	colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// What to do with the Stencil data before drawing
	colorAttachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	// What to do with the Stencil data before drawing
	colorAttachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// What to do with the data on the fresh canvas
	colorAttachmentInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// What to do with the data on the finished canvas
	colorAttachmentInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Info for the subpass - one renderpass may have many subpasses, we'll only use one for now.
	VkAttachmentReference colorAttachmentRefInfo = {};
	colorAttachmentRefInfo.attachment = 0; // Array index reference to the above
	colorAttachmentRefInfo.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassCreateInfo = {};
	subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassCreateInfo.colorAttachmentCount = 1;// The index of the array containing our fragment shader
	subpassCreateInfo.pColorAttachments = &colorAttachmentRefInfo;


	VkSubpassDependency renderPassDependency = {};
	renderPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	renderPassDependency.dstSubpass = 0;
	renderPassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	renderPassDependency.dstSubpass = 0;
	renderPassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	renderPassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachmentInfo;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassCreateInfo;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &renderPassDependency;

	returnResult = vkCreateRenderPass( _mLogicalDevice, &renderPassCreateInfo, nullptr, &_mRenderPass );

	return returnResult;
}


VkResult VK_Renderer::InitGraphicsPipeline()
{
	VkResult returnResult;
	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	vector<char> vertShaderCode = Utilities::ReadFileAsCharVec( "Assets/Shaders/vert.spv" );
	vector<char> fragShaderCode = Utilities::ReadFileAsCharVec( "Assets/Shaders/frag.spv" );

	auto vertShaderData = BuildShaderModule( vertShaderCode );
	auto fragShaderData = BuildShaderModule( fragShaderCode );

	if( vertShaderData.first != VK_SUCCESS || fragShaderData.first != VK_SUCCESS )
	{
		vkDestroyShaderModule( _mLogicalDevice, vertShaderData.second, nullptr );
		vkDestroyShaderModule( _mLogicalDevice, fragShaderData.second, nullptr );

		if( vertShaderData.first != VK_SUCCESS )
		{
			return vertShaderData.first;
		}
		else
		{
			return fragShaderData.first;
		}
	}

	vertexShaderModule = vertShaderData.second;
	fragmentShaderModule = fragShaderData.second;

	// Assigning the shader byte code modules (wrappers) to the pipeline
	VkPipelineShaderStageCreateInfo vertPipelineShaderCreateInfo = {};
	vertPipelineShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertPipelineShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertPipelineShaderCreateInfo.module = vertexShaderModule;
	vertPipelineShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragPipelineShaderCreateInfo = {};
	fragPipelineShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragPipelineShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragPipelineShaderCreateInfo.module = fragmentShaderModule;
	fragPipelineShaderCreateInfo.pName = "main";// <- method to invoke in the shader byte code
	/*
	"...That means that it's possible to combine multiple fragment shaders into
	a single shader module and use different entry points to differentiate
	between their behaviors..."
	Source - https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
	*/

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertPipelineShaderCreateInfo,
		fragPipelineShaderCreateInfo
	};


	// Creating Vertex input
	// For the Vertex buffers.
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size() );
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Creating the input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;


	// Creating the viewport and scissor region - viewing filters for the rasteriser
	VkViewport targetViewPort = {};
	targetViewPort.x = 0.0f;
	targetViewPort.y = 0.0f;
	targetViewPort.width = ( float ) _mSwapChainImageExtent.width;
	targetViewPort.height = ( float ) _mSwapChainImageExtent.height;
	targetViewPort.minDepth = 0.0f; // near plane
	targetViewPort.maxDepth = 1.0f; // far plane
	// you may render with reversed depth buffers, but they cannot be greater than 1 or less than 0

	VkRect2D scissorRegion = {};
	scissorRegion.offset = { 0, 0 };
	scissorRegion.extent = _mSwapChainImageExtent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = &targetViewPort;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = &scissorRegion;


	// Creating the Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	// If depthClampEnable were set to true, this renders everything past the far plane with a depth of the far plane. Useful for shadow mapping but requires a GPU feature.
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	// If rasterizerDiscardEnable were set to true, Geometry data would be discarded at the rasterizer, basically disabling all visual data for the framebuffer.
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	// All modes other than VK_POLYGON_MODE_FILL require GPU features to be enabled
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;


	// Creating our Sampler:
	// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkPipelineMultisampleStateCreateInfo.html
	VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo = {};
	multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampleCreateInfo.minSampleShading = 1.0f;
	multiSampleCreateInfo.pSampleMask = nullptr;
	multiSampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multiSampleCreateInfo.alphaToOneEnable = VK_FALSE;


	// Dictating the color blending
	VkPipelineColorBlendAttachmentState colorBlendingAttachement = {};
	colorBlendingAttachement.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendingAttachement.blendEnable = VK_FALSE;
	colorBlendingAttachement.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendingAttachement.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendingAttachement.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendingAttachement.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendingAttachement.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendingAttachement.alphaBlendOp = VK_BLEND_OP_ADD;
	// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBlendFactor.html
	// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBlendOp.html
	colorBlendingAttachement.blendEnable = VK_TRUE;
	colorBlendingAttachement.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendingAttachement.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendingAttachement.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendingAttachement.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendingAttachement.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendingAttachement.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &colorBlendingAttachement;
	colorBlendingCreateInfo.blendConstants[ 0 ] = 0.0f;
	colorBlendingCreateInfo.blendConstants[ 1 ] = 0.0f;
	colorBlendingCreateInfo.blendConstants[ 2 ] = 0.0f;
	colorBlendingCreateInfo.blendConstants[ 3 ] = 0.0f;

	_mPipelineLayout = {};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = 0;

	returnResult = vkCreatePipelineLayout( _mLogicalDevice,
										   &pipelineLayoutCreateInfo,
										   nullptr,
										   &_mPipelineLayout );

	if( returnResult != VK_SUCCESS ) return returnResult;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multiSampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = _mPipelineLayout;
	pipelineCreateInfo.renderPass = _mRenderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	returnResult = vkCreateGraphicsPipelines( _mLogicalDevice,
											  VK_NULL_HANDLE,
											  1,
											  &pipelineCreateInfo,
											  nullptr,
											  &_mGraphicalPipeline );


	vkDestroyShaderModule( _mLogicalDevice, vertexShaderModule, nullptr );
	vkDestroyShaderModule( _mLogicalDevice, fragmentShaderModule, nullptr );

	return returnResult;
}


VkResult VK_Renderer::InitFrameBuffers()
{
	VkResult returnResult;
	_mSwapChainFrameBuffers = {};
	_mSwapChainFrameBuffers.resize( _mSwapChainImages.size() );

	for( size_t i = 0; i < _mSwapChainImageViews.size(); i++ )
	{
		VkImageView* attachedImageView = &_mSwapChainImageViews[ i ];

		VkFramebufferCreateInfo frameBufferInfo = {};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = _mRenderPass;
		frameBufferInfo.attachmentCount = 1;
		frameBufferInfo.pAttachments = attachedImageView;
		frameBufferInfo.width = _mSwapChainImageExtent.width;
		frameBufferInfo.height = _mSwapChainImageExtent.height;
		frameBufferInfo.layers = 1;

		returnResult = vkCreateFramebuffer( _mLogicalDevice, &frameBufferInfo, nullptr, &_mSwapChainFrameBuffers[ i ] );

		if( returnResult != VK_SUCCESS ) return returnResult;
	}

	return returnResult;
}


VkResult VK_Renderer::InitCommandPool()
{
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = 0;
	poolCreateInfo.queueFamilyIndex = _mPhysicalDeviceQueueFamilyIndexes._mGraphicsFamilyIndex;

	return vkCreateCommandPool( _mLogicalDevice, &poolCreateInfo, nullptr, &_mCommandPool );
}


VkResult VK_Renderer::InitVertexBuffer()
{
	VkResult returnResult;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof( _mVertices[ 0 ] ) * _mVertices.size();
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	returnResult = vkCreateBuffer( _mLogicalDevice, &bufferCreateInfo, nullptr, &_mVertexBuffer );

	if( IfVKErrorPrintMSG( returnResult, "Cannot create buffer for verticies." ) ) return returnResult;

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements( _mLogicalDevice, _mVertexBuffer, &memRequirements );

	auto memType = FindMemoryType( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

	if( memType.first == false ) return VK_ERROR_MEMORY_MAP_FAILED;

	VkMemoryAllocateInfo vertexMemAllocationInfo = {};
	vertexMemAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vertexMemAllocationInfo.allocationSize = memRequirements.size;
	vertexMemAllocationInfo.memoryTypeIndex = memType.second;

	returnResult = vkAllocateMemory( _mLogicalDevice, &vertexMemAllocationInfo, nullptr, &_mVertexBufferMemory );
	if( IfVKErrorPrintMSG( returnResult, "Cannot allocate memory on GPU for vertex buffer." ) ) return returnResult;

	returnResult = vkBindBufferMemory( _mLogicalDevice, _mVertexBuffer, _mVertexBufferMemory, 0 ); // final param is mem offset and must be divisible by memRequirements.alignment
	if( IfVKErrorPrintMSG( returnResult, "Could not bind buffer to GPU memory." ) ) return returnResult;

	void* destination;
	returnResult = vkMapMemory( _mLogicalDevice, _mVertexBufferMemory, 0, bufferCreateInfo.size, 0, &destination );
	if( IfVKErrorPrintMSG( returnResult, "Could not map GPU memory for vertex buffer memcpy." ) ) return returnResult;
	memcpy( destination, _mVertices.data(), ( size_t ) bufferCreateInfo.size );

	vkUnmapMemory( _mLogicalDevice, _mVertexBufferMemory );

	return returnResult;
}


VkResult VK_Renderer::InitCommandBuffers()
{
	VkResult returnResult = VK_SUCCESS;

	_mCommandBuffers = {};
	_mCommandBuffers.resize( _mSwapChainFrameBuffers.size() );

	VkCommandBufferAllocateInfo allocationInfo = {};
	allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocationInfo.commandPool = _mCommandPool;
	allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocationInfo.commandBufferCount = ( uint32_t ) _mCommandBuffers.size();

	returnResult = vkAllocateCommandBuffers( _mLogicalDevice, &allocationInfo, _mCommandBuffers.data() );

	if( returnResult != VK_SUCCESS ) return returnResult;

	for( size_t i = 0; i < _mCommandBuffers.size(); i++ )
	{
		VkCommandBufferBeginInfo startinUpInfo = {};
		startinUpInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		startinUpInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		startinUpInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer( _mCommandBuffers[ i ], &startinUpInfo );

		VkClearValue screenClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderPassBeginInfo startingRenderPassInfo = {};
		startingRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		startingRenderPassInfo.renderPass = _mRenderPass;
		startingRenderPassInfo.framebuffer = _mSwapChainFrameBuffers[ i ];
		startingRenderPassInfo.renderArea.offset = { 0,0 };
		startingRenderPassInfo.renderArea.extent = _mSwapChainImageExtent;
		startingRenderPassInfo.clearValueCount = 1;
		startingRenderPassInfo.pClearValues = &screenClearColor;

		vkCmdBeginRenderPass( _mCommandBuffers[ i ], &startingRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

		// Draw commands
		vkCmdBindPipeline( _mCommandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, _mGraphicalPipeline );

		VkBuffer vertexBuffers[] = { _mVertexBuffer };
		VkDeviceSize offsets[] = { 0 }; // an array of buffer offsets - presumably the starting points of each object's vertex data
		vkCmdBindVertexBuffers( _mCommandBuffers[ i ], 0, 1, vertexBuffers, offsets );

		vkCmdDraw( _mCommandBuffers[ i ], static_cast< uint32_t >( _mVertices.size() ), 1, 0, 0 );
		vkCmdEndRenderPass( _mCommandBuffers[ i ] );

		returnResult = vkEndCommandBuffer( _mCommandBuffers[ i ] );

		if( returnResult != VK_SUCCESS ) return returnResult;
	}

	return returnResult;
}


VkResult VK_Renderer::InitSemaphores()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult returnResult = vkCreateSemaphore( _mLogicalDevice,
											   &semaphoreCreateInfo,
											   nullptr,
											   &_mImageAvailableSemaphore );

	if( returnResult != VK_SUCCESS ) return returnResult;

	returnResult = vkCreateSemaphore( _mLogicalDevice,
									  &semaphoreCreateInfo,
									  nullptr,
									  &_mRenderFinishedSemaphore );

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

	for( uint32_t i = 0; i < queueFamilyCount; i++ )
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

		actualExtent.width = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, actualExtent.width ) );
		actualExtent.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, actualExtent.height ) );

		return actualExtent;
	}
}


pair<VkResult, VkShaderModule> VK_Renderer::BuildShaderModule( const vector<char>& byteCode )
{
	pair<VkResult, VkShaderModule> returnData;

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.codeSize = byteCode.size();
	createInfo.pCode = reinterpret_cast< const uint32_t* >( byteCode.data() );

	VkShaderModule shaderModule;
	VkResult vkResult = vkCreateShaderModule( _mLogicalDevice, &createInfo, nullptr, &shaderModule );

	returnData.first = vkResult;
	returnData.second = shaderModule;

	IfVKErrorPrintMSG( vkResult, "Could not create shader module!" );

	return returnData;
}


pair<bool, uint32_t> VK_Renderer::FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags props )
{
	bool success = false;
	uint32_t result = UINT32_MAX;

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties( _mPhysicalDevice, &memProperties );

	for( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
	{
		// I dislike the non self documenting binary operations in this if statement
		// This works and it's what's recommended, but I need to rewrite this when I
		// better understand what's going on
		if( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[ i ].propertyFlags & props ) == props )
		{
			success = true;
			result = i;
		}
	}

	return pair<bool, uint32_t>( success, result );
}


void VK_Renderer::CleanSwapChainResources()
{
	for( VkFramebuffer frameBuffer : _mSwapChainFrameBuffers )
	{
		vkDestroyFramebuffer( _mLogicalDevice, frameBuffer, nullptr );
	}

	vkFreeCommandBuffers( _mLogicalDevice, _mCommandPool, _mCommandBuffers.size(), _mCommandBuffers.data() );
	vkDestroyPipeline( _mLogicalDevice, _mGraphicalPipeline, nullptr );
	vkDestroyPipelineLayout( _mLogicalDevice, _mPipelineLayout, nullptr );
	vkDestroyRenderPass( _mLogicalDevice, _mRenderPass, nullptr );

	for( VkImageView imageView : _mSwapChainImageViews )
	{
		vkDestroyImageView( _mLogicalDevice, imageView, nullptr );
	}

	vkDestroySwapchainKHR( _mLogicalDevice, _mSwapChainHandle, nullptr );
}

// Event triggered
void VK_Renderer::OnWindowResize( GLFWwindow* window, int width, int height )
{
	if( width == 0 || height == 0 ) return;
	bool success = false;
	VK_Renderer* tempRef = nullptr;

	try
	{
		tempRef = reinterpret_cast< VK_Renderer* >( glfwGetWindowUserPointer( window ) );

		if( window != tempRef->_mWindow )
		{
			Utilities::LogErrorIfDebug( "WindowManager window and local window pointer Are not the same!" );
		}

		tempRef->_mWidth = width;
		tempRef->_mHeight = height;
		success = !IfVKErrorPrintMSG( tempRef->BuildNewSwapChain(), "Failed to build new swap chain" );
	}
	catch( exception e )
	{
		Utilities::LogErrorIfDebug( e.what() );
	}

	if( !success )
	{
		delete tempRef;
		tempRef = nullptr;
	}
}


VkResult VK_Renderer::BuildNewSwapChain()
{
	VkResult returnResult;
	vkDeviceWaitIdle( _mLogicalDevice );

	CleanSwapChainResources();

	// Call same methods as used on initialisation
	returnResult = InitSwapChain();
	if( returnResult != VK_SUCCESS ) return returnResult;

	returnResult = InitImageViews();
	if( returnResult != VK_SUCCESS ) return returnResult;

	returnResult = InitRenderPasses();
	if( returnResult != VK_SUCCESS ) return returnResult;

	returnResult = InitGraphicsPipeline();
	if( returnResult != VK_SUCCESS ) return returnResult;

	returnResult = InitFrameBuffers();
	if( returnResult != VK_SUCCESS ) return returnResult;

	returnResult = InitCommandBuffers();

	return returnResult;
}


VkResult VK_Renderer::InitialiseWindowSurface()
{
	return glfwCreateWindowSurface( _mVkInstance, _mWindow, nullptr, &_mWindowSurface );
}


VkResult VK_Renderer::RenderScene()
{
	/*
	1: Acquire an image from the swap chain
	2: Execute the command buffer with that image as attachment in the framebuffer
	3: Return the image to the swap chain for presentation
	*/
	VkResult returnResult;
	uint32_t imageIndex = 1;
	returnResult = vkAcquireNextImageKHR( _mLogicalDevice,
										  _mSwapChainHandle,
										  UINT64_MAX,
										  _mImageAvailableSemaphore,
										  VK_NULL_HANDLE,
										  &imageIndex );

	if( returnResult == VK_ERROR_OUT_OF_DATE_KHR )
	{
		BuildNewSwapChain();
		return returnResult;
	}
	else if( returnResult != VK_SUCCESS )
	{
		Utilities::LogWarningIfDebug( "Couldn't get the next image in the swap chain!" );
	}


	VkSemaphore waitSemaphores[] = { _mImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemphores[] = { _mRenderFinishedSemaphore };
	VkSwapchainKHR swapChains[] = { _mSwapChainHandle };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_mCommandBuffers[ imageIndex ];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemphores;

	returnResult = vkQueueSubmit( _mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	if( IfVKErrorPrintMSG( returnResult, "ERROR AT RENDER: Could not submit queue." ) )
	{
		return returnResult;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = _mSwapChainImages.size();
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemphores;
	presentInfo.pSwapchains = swapChains;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	returnResult = vkQueuePresentKHR( _mPresentQueue, &presentInfo );

	if( returnResult == VK_ERROR_OUT_OF_DATE_KHR || returnResult == VK_SUBOPTIMAL_KHR )
	{
		BuildNewSwapChain();
	}
	else if( returnResult != VK_SUCCESS )
	{
		Utilities::LogWarningIfDebug( "Failed to present the next swap chain image." );
	}

	vkQueueWaitIdle( _mPresentQueue );
	return returnResult;
}