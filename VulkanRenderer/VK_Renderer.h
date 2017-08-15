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
	const string _mVkReportPrefix = "Vk_Renderer Report: ";

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

	VkInstance									_mVkInstance;						// Used to track hardware's state (One instance can have many physical devices I only want one)
	VkPhysicalDevice							_mPhysicalDevice;					// Of the potentially many devices, this renderer will only utilize one
	DeviceQueueFamilyIndexes					_mPhysicalDeviceQueueFamilyIndexes;	// Look uptable for important Device Queues when building logical device
	VkDevice									_mLogicalDevice;					// Abstraction of the hardware
	VkQueue										_mGraphicsQueue;					// The handle through which we'll send graphical instructions
	//uint32_t									_mGraphicsQueueDeviceIndex;			// Where the Graphics queue is on the graphics card
	VkQueue										_mPresentQueue;
	VkCommandPool								_mGraphicsQueueCmdPool;				// 
	VkCommandBuffer								_mGraphicsQueueCmdBuffer;			//
	VkRenderPass								_mRenderPass;						//
	VkSurfaceKHR								_mWindowSurface;					// The window "Surface" Vulkan will be drawing onto

	VkSwapchainKHR								_mSwapChainHandle;
	vector<VkImage>								_mSwapChainImages;
	VkFormat									_mSwapChainFormat;
	VkExtent2D									_mSwapChainExtent;

	VkSurfaceFormatKHR							_mDeviceSurfaceFormats;
	VkPresentModeKHR							_mPresentModeKHR;
	VkSurfaceCapabilitiesKHR					_mSurfaceCapabilities;


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
	VkResult									InitSwapChain();

	// GLFW
	void										CreateGLFWWindow();
	VkResult									InitialiseWindowSurface();

	// Custom
	bool										IfVKErrorPrintMSG( VkResult VkState, string errOutput );
	bool										IfVKErrorPrintMSG( VkResult VkState, string errOutput, string successOutput );

	uint64_t									RatePhysicalDeviceForGameGraphics(VkPhysicalDevice* physicalDevice);
	DeviceQueueFamilyIndexes					FindDeviceQueueFamilies( VkPhysicalDevice* physicalDevice );
	SwapChainSupportDetails						QueryDeviceSwapChainSupport(VkPhysicalDevice* physicalDevice);
	VkSurfaceFormatKHR							ChooseSwapChainSurfaceFormat( vector<VkSurfaceFormatKHR> availableFormats);
	VkPresentModeKHR							ChooseSwapChainPresentationMode( vector<VkPresentModeKHR> availableModes);
	VkExtent2D									ChooseSwapChainExtentionDimensions( VkSurfaceCapabilitiesKHR capabilities );
public:
	VK_Renderer();
	~VK_Renderer();

	bool										isCorrectlyInitialised;
	void										GameLoop();
	void										RenderScene();
};