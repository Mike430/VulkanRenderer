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

// namespaces
using namespace std;

class VK_Renderer
{
private:
	// C strings for searching in Vulkan.h
	vector<const char*>							_mWantedInstanceExtensions;
	vector<const char*>							_mWantedDeviceExtensions;
	vector<const char*>							_mTurnedOnInstanceExtensions;
	vector<const char*>							_mTurnedOnDeviceExtensions;

	// GLFW Variables
	GLFWwindow*									_mWindow;

	// Vulkan variables
	VkApplicationInfo							_mAppInfo;
	VkInstanceCreateInfo						_mInstanceCreateInfo;

	VkInstance									_mVkInstance;						// Used to track hardware's state (One instance can have many physical devices I only want one)
	VkPhysicalDevice							_mPhysicalDevice;					// Of the potentially many devices, this renderer will only utilize one
	VkDevice									_mLogicalDevice;					// Abstraction of the hardware
	VkQueue										_mGraphicsQueue;					// The handle through which we'll send graphical instructions
	VkSurfaceKHR								_mWindowSurface;					// The window "Surface" Vulkan will be drawing onto


	VkSwapchainKHR								_mSwapChainHandle;
	vector<VkImage>								_mSwapChainImages;					// Every immage needs an image view, conside a struct
	VkExtent2D									_mSwapChainSurfaceResolution;
	VkFormat									_mSwapChainImageFormat;
	vector<VkImageView>							_mSwapChainImageViews;


	// Methods
	// Vulkan
	VkResult									InitVulkanGraphicsPipeline();
	VkResult									InitInstance();
	VkResult									ChooseAPhysicalDevice();
	VkResult									InitLogicalDevice();
	VkResult									InitSwapChain();
	VkResult									CreateGraphicalPipeline();

	// GLFW
	VkResult									CreateVulkanWindowSurface();
public:
	VK_Renderer();
	~VK_Renderer();

	bool										isCorrectlyInitialised;
	void										GameLoop();
};