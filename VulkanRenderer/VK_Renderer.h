#pragma once
// Project dependencies
#define GLFW_INCLUDE_VULKAN

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

// Other dependencies
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "Utilities.h"
#include "GraphicalUtilities.h"

// namespaces
using namespace std;
using namespace Utilities;
using namespace GraphicalUtilities;

class VK_Renderer
{
private:
#ifdef NDEBUG
	const bool _mIsDebugBuild = false;
#else
	const bool _mIsDebugBuild = true;
#endif
	const uint8_t _mSwapChainSize = 2;

	// C strings for searching in Vulkan.h
	vector<const char*>							_mWantedInstanceLayers;
	vector<const char*>							_mWantedInstanceExtensions;
	vector<const char*>							_mWantedDeviceExtensions;

	vector<const char*>							_mTurnedOnInstanceLayers;
	vector<const char*>							_mTurnedOnInstanceExtensions;
	vector<const char*>							_mTurnedOnDeviceExtensions;

	vector<vector<vertex>>						_mVertexBufferData;

	// GLFW Variables
	GLFWwindow*									_mWindow;

	// Shared Variables
	uint32_t									_mWidth = 1600;
	uint32_t									_mHeight = 900;
	uint32_t									_mChainNextImageIndex = 0;

	// Vulkan variables
	VkDebugReportCallbackEXT					_mDebugCallbackHandle;

	VkApplicationInfo							_mAppInfo;
	VkInstanceCreateInfo						_mInstanceCreateInfo;

	VkInstance									_mVkInstance;						// Used to track hardware's state (One instance can have many physical devices I only want one)
	VkPhysicalDevice							_mPhysicalDevice;					// Of the potentially many devices, this renderer will only utilize one
	VkDevice									_mLogicalDevice;					// Abstraction of the hardware
	VkQueue										_mGraphicsQueue;					// The handle through which we'll send graphical instructions
	uint32_t									_mGraphicsQueueDeviceIndex;			// Where the Graphics queue is on the graphics card
	VkCommandPool								_mGraphicsQueueCmdPool;				// 
	VkCommandBuffer								_mGraphicsQueueCmdBuffer;			//
	VkRenderPass								_mRenderPass;						//
	VkSurfaceKHR								_mWindowSurface;					// The window "Surface" Vulkan will be drawing onto

	VkSwapchainKHR								_mSwapChainHandle;
	vector<VkImage>								_mSwapChainImages;					// Every immage needs an image view, conside a struct
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

	VkResult									InitVertexBuffer();

	// GLFW
	void										CreateGLFWWindow();
	VkResult									InitialiseWindowSurface();

	// Custom
	bool										IfVKErrorPrintMSG( VkResult VkState, string output );
public:
	VK_Renderer();
	~VK_Renderer();

	bool										isCorrectlyInitialised;
	void										GameLoop();
	void										RenderScene();
};