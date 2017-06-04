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

// namespaces
using namespace std;
using namespace Utilities;

class VK_Renderer
{
private:
#ifdef NDEBUG
	const bool _mValidationLayerOn = false;
#else
	const bool _mValidationLayerOn = true;
#endif

	// C strings for searching in Vulkan.h
	vector<const char*>							_mWantedInstanceLayers;
	vector<const char*>							_mWantedInstanceExtensions;
	vector<const char*>							_mWantedDeviceExtensions;

	vector<const char*>							_mTurnedOnInstanceLayers;
	vector<const char*>							_mTurnedOnInstanceExtensions;
	vector<const char*>							_mTurnedOnDeviceExtensions;

	// GLFW Variables
	GLFWwindow*									_mWindow;

	// Shared Variables
	uint32_t									_mWidth = 1600;
	uint32_t									_mHeight = 900;
	uint32_t										_mChainNextImageIndex = 0;

	// Vulkan variables
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

	// pipeline
	//VkFence										_mRenderFence;						// Fences off the rest of the CPU program until the GPU has finished rendering


	VkSwapchainKHR								_mSwapChainHandle;
	vector<VkImage>								_mSwapChainImages;					// Every immage needs an image view, conside a struct
	vector<VkImageView>							_mSwapChainImageViews;
	vector<VkFramebuffer>						_mSwapChainFrameBuffers;
	VkExtent2D									_mSwapChainSurfaceResolution;
	VkFormat									_mSwapChainImageFormat;


	// Methods
	// Vulkan
	VkResult									InitVulkanDevicesAndRenderer();
	VkResult									InitInstance();
	VkResult									ChooseAPhysicalDevice();
	VkResult									InitLogicalDevice();

	VkResult									InitVulkanGraphicalPipeline();
	VkResult									InitSwapChain();
	VkResult									InitGraphicsQueue();
	VkResult									InitFrameBuffers();

	// GLFW
	void										CreateGLFWWindow();
	VkResult									InitialiseWindowSurface();
public:
	VK_Renderer();
	~VK_Renderer();

	bool										isCorrectlyInitialised;
	void										GameLoop();
	void										RenderScene();
};

/*
struct VkGraphicsCard
{
	VkPhysicalDevice							_mPhysicalDevice;
	VkQueue										_mGraphicsQueue;
	VkCommandBuffer								_mCommandBuffer;
	VkCommandPool								_mCommandPool;
	uint32_t									_graphicsQueueIndex;
};

struct VkImageSet
{
	VkImage										_mImage;
	VkImageView									_mImageView;
	VkFramebuffer								_mFrameBuffer;
};
*/