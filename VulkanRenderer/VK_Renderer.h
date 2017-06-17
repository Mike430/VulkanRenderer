#pragma once
// Project dependencies
#define GLFW_INCLUDE_VULKAN

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING
#endif


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

// Other dependencies
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "Utilities.h"
#include "GraphicalUtilities.h"

// namespaces
using namespace std;
using namespace Utilities;
using namespace GraphicalUtilities;


struct DeviceQueueFamilyIndexes
{
	int _mGraphicsFamilyIndex = -1;

	bool HasAllNeededQueues()
	{
		return _mGraphicsFamilyIndex != -1;
	}
};


class VK_Renderer
{
private:
#ifdef NDEBUG
	const bool _mIsDebugBuild = false;
#else
	const bool _mIsDebugBuild = true;
#endif
	const uint8_t _mSwapChainSize = 2;
	const string _mVkReportPrefix = "Vk_Renderer Report: ";

	// Windows
	HANDLE _mConsoleHandle;

	// GLFW Variables
	GLFWwindow*									_mWindow;

	// Shared Variables
	uint32_t									_mWidth = 1600;
	uint32_t									_mHeight = 900;
	uint32_t									_mChainNextImageIndex = 0;

	// Vulkan variables
	// C strings for searching in Vulkan.h
	vector<const char*>							_mWantedInstanceLayers;
	vector<const char*>							_mWantedInstanceExtensions;
	vector<const char*>							_mWantedDeviceExtensions;

	vector<const char*>							_mTurnedOnInstanceLayers;
	vector<const char*>							_mTurnedOnInstanceExtensions;
	vector<const char*>							_mTurnedOnDeviceExtensions;

	vector<vector<vertex>>						_mVertexBufferData;

	// Vulkan's core component objects
	VkDebugReportCallbackEXT					_mDebugCallbackHandle;

	VkApplicationInfo							_mAppInfo;
	VkInstanceCreateInfo						_mInstanceCreateInfo;

	VkInstance									_mVkInstance;						// Used to track hardware's state (One instance can have many physical devices I only want one)
	VkPhysicalDevice							_mPhysicalDevice;					// Of the potentially many devices, this renderer will only utilize one
	DeviceQueueFamilyIndexes					_mPhysicalDeviceQueueFamilyIndexes;	// Look uptable for important Device Queues when building logical device
	VkDevice									_mLogicalDevice;					// Abstraction of the hardware
	VkQueue										_mGraphicsQueue;					// The handle through which we'll send graphical instructions
	uint32_t									_mGraphicsQueueDeviceIndex;			// Where the Graphics queue is on the graphics card
	VkCommandPool								_mGraphicsQueueCmdPool;				// 
	VkCommandBuffer								_mGraphicsQueueCmdBuffer;			//
	VkRenderPass								_mRenderPass;						//
	VkSurfaceKHR								_mWindowSurface;					// The window "Surface" Vulkan will be drawing onto

	VkSwapchainKHR								_mSwapChainHandle;
	vector<VkImage>								_mSwapChainImages;					// An Image in the swap chain consists of an image, image view, frame buffer and optional fence
	vector<VkImageView>							_mSwapChainImageViews;
	vector<VkFramebuffer>						_mSwapChainFrameBuffers;
	vector<VkFence>								_mSwapChainRenderFences;
	VkExtent2D									_mSwapChainSurfaceResolution;
	VkFormat									_mSwapChainImageFormat;

	VkSurfaceFormatKHR							_mDeviceSurfaceFormats;
	VkPresentModeKHR							_mPresentModeKHR;
	VkSurfaceCapabilitiesKHR					_mSurfaceCapabilities;

	VkBuffer									_mVertexBuffer;
	VkDeviceMemory								_mVertexBufferMemory;


	// Methods
	// Vulkan
	VkResult									InitVulkanRenderer();
	VkResult									InitVulkanAndGLFW();
	static VKAPI_ATTR VkBool32 VKAPI_CALL		DebugCallback( VkDebugReportFlagsEXT msgTypesAsFlags, VkDebugReportObjectTypeEXT msgSubjectObj, uint64_t object, size_t location, int32_t code, const char* layerPrefix, const char* message, void* userData );
	VkResult									CreateDebugReportCallbackEXT( VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback );
	void										DestroyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator );

	VkResult									InitInstance();
	VkResult									SetUpDebugCallback();
	VkResult									ChooseAPhysicalDevice();
	VkResult									InitLogicalDevice();

	VkResult									InitVulkanGraphicalPipeline();
	VkResult									InitSwapChain();
	VkResult									InitGraphicsQueue();
	VkResult									InitFrameBuffers();
	VkResult									InitRenderFences();

	// Temporary
	VkResult									InitVertexBuffer();

	// GLFW
	void										CreateGLFWWindow();
	VkResult									InitialiseWindowSurface();

	// Custom
	bool										IfVKErrorPrintMSG( VkResult VkState, string errOutput );
	bool										IfVKErrorPrintMSG( VkResult VkState, string errOutput, string successOutput );

	uint64_t									RatePhysicalDeviceForGameGraphics(VkPhysicalDevice* physicalDevice);
	DeviceQueueFamilyIndexes					FindDeviceQueueFamilies( VkPhysicalDevice* physicalDevice );
public:
	VK_Renderer();
	~VK_Renderer();

	bool										isCorrectlyInitialised;
	void										GameLoop();
	void										RenderScene();
};