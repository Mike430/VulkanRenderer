#pragma once
// Project definitions
#define GLFW_INCLUDE_VULKAN

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

// SDK deprendencies
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// C++ & System dependencies
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <algorithm>

// Custom dependencies
#include "Utilities.h"
#include "GraphicalUtilities.h"

// namespaces
using namespace std;
using namespace GraphicalUtilities;


struct DeviceQueueFamilyIndexes
{
	int _mGraphicsFamilyIndex = -1;
	int _mPresentFamilyIndex = -1;

	bool HasAllNeededQueues()
	{
		return _mGraphicsFamilyIndex > -1 && _mPresentFamilyIndex > -1;
	}
};


struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR _mCapabilities;
	vector<VkSurfaceFormatKHR> _mAvailableFormats;
	vector<VkPresentModeKHR> _mAvailablePresentModes;
};


class VK_Renderer
{
private:
	const uint8_t _mSwapChainSize = 2;
	static const string _mVkReportPrefix;

	// GLFW Variables
	GLFWwindow*									_mWindow;

	// Shared Variables
	uint32_t									_mWidth = 1600;
	uint32_t									_mHeight = 900;
	uint32_t									_mChainNextImageIndex = 0;

	// Vulkan variables
	// C strings for searching in Vulkan.h
	vector<const char*>							_mWantedInstanceAndDeviceLayers;
	vector<const char*>							_mWantedInstanceExtensions;
	vector<const char*>							_mWantedDeviceExtensions;

	vector<const char*>							_mTurnedOnInstanceLayers;
	vector<const char*>							_mTurnedOnInstanceExtensions;
	vector<const char*>							_mTurnedOnDeviceLayers;
	vector<const char*>							_mTurnedOnDeviceExtensions;

	vector<vector<vertex>>						_mVertexBufferData;

	// Vulkan's core component objects
	VkDebugReportCallbackEXT					_mDebugCallbackHandle;

	VkApplicationInfo							_mAppInfo;
	VkInstanceCreateInfo						_mInstanceCreateInfo;

	VkInstance									_mVkInstance;
	VkPhysicalDevice							_mPhysicalDevice;
	DeviceQueueFamilyIndexes					_mPhysicalDeviceQueueFamilyIndexes;
	VkDevice									_mLogicalDevice;
	VkQueue										_mGraphicsQueue;
	VkQueue										_mPresentQueue;
	VkSurfaceKHR								_mWindowSurface;

	VkSwapchainKHR								_mSwapChainHandle;
	vector<VkImage>								_mSwapChainImages;
	vector<VkImageView>							_mSwapChainImageViews;
	VkFormat									_mSwapChainImageFormat;
	VkExtent2D									_mSwapChainImageExtent;
	vector<VkFramebuffer>						_mSwapChainFrameBuffers;

	VkSurfaceFormatKHR							_mDeviceSurfaceFormats;
	VkPresentModeKHR							_mPresentModeKHR;
	VkSurfaceCapabilitiesKHR					_mSurfaceCapabilities;

	VkRenderPass								_mRenderPass;
	VkPipelineLayout							_mPipelineLayout;
	VkPipeline									_mGraphicalPipeline;
	VkCommandPool								_mCommandPool;
	vector<VkCommandBuffer>						_mCommandBuffers;

	VkSemaphore									_mImageAvailableSemaphore;
	VkSemaphore									_mRenderFinishedSemaphore;


	// Methods
	// Vulkan
	VkResult									InitVulkanRenderer();
	VkResult									InitVulkanAndGLFW();
	static VKAPI_ATTR VkBool32 VKAPI_CALL		DebugCallback( VkDebugReportFlagsEXT msgTypesAsFlags,
															   VkDebugReportObjectTypeEXT msgSubjectObj,
															   uint64_t object,
															   size_t location,
															   int32_t code,
															   const char* layerPrefix,
															   const char* message,
															   void* userData );
	VkResult									CreateDebugReportCallbackEXT( VkInstance instance,
																			  const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
																			  const VkAllocationCallbacks* pAllocator,
																			  VkDebugReportCallbackEXT* pCallback );
	void										DestroyDebugReportCallbackEXT( VkInstance instance,
																			   VkDebugReportCallbackEXT callback,
																			   const VkAllocationCallbacks* pAllocator );
	VkResult									InitInstance();
	VkResult									SetUpDebugCallback();
	VkResult									ChooseAPhysicalDevice();
	VkResult									InitLogicalDevice();
	VkResult									InitSwapChain();
	VkResult									InitImageViews();
	VkResult									InitRenderPasses();
	VkResult									InitGraphicsPipeline();
	VkResult									InitFrameBuffers();
	VkResult									InitCommandPool();
	VkResult									InitCommandBuffers();
	VkResult									InitSemaphores();

	// Vk Event Methods
	VkResult									BuildNewSwapChain();

	// Vk Helpers
	uint64_t									RatePhysicalDeviceForGameGraphics( VkPhysicalDevice* physicalDevice );
	DeviceQueueFamilyIndexes					FindDeviceQueueFamilies( VkPhysicalDevice* physicalDevice );
	SwapChainSupportDetails						QueryDeviceSwapChainSupport( VkPhysicalDevice* physicalDevice );
	VkSurfaceFormatKHR							ChooseSwapChainSurfaceFormat( vector<VkSurfaceFormatKHR> availableFormats );
	VkPresentModeKHR							ChooseSwapChainPresentationMode( vector<VkPresentModeKHR> availableModes );
	VkExtent2D									ChooseSwapChainExtentionDimensions( VkSurfaceCapabilitiesKHR capabilities );
	pair<VkResult, VkShaderModule>				BuildShaderModule(const vector<char>& byteCode);
	void										CleanSwapChainResources();

	// GLFW
	void										CreateGLFWWindow();
	VkResult									InitialiseWindowSurface();

	// GLFW Event Methods
	static void									OnWindowResize(GLFWwindow* window, int width, int height);

	// Custom
	static bool									IfVKErrorPrintMSG( VkResult VkState, string errOutput );
	static bool									IfVKErrorPrintMSG( VkResult VkState, string errOutput, string successOutput );
public:
	VK_Renderer();
	~VK_Renderer();

	bool										isCorrectlyInitialised;
	void										GameLoop();
	VkResult									RenderScene();
};