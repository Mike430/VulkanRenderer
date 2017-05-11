#pragma once
// Project dependencies
#define GLFW_INCLUDE_VULKAN
#include "vulkan.h"
#include "GLFW/glfw3.h"

// Other dependencies
#include <iostream>
#include <string>
#include <vector>

// namespaces
using namespace std;

class VK_Renderer
{
private:
	// Vulkan variables
	VkApplicationInfo							_mAppInfo;
	VkInstanceCreateInfo						_mInstanceCreateInfo;

	//GLFW Variables
	GLFWwindow*									_mWindow;

	VkInstance									_mVkInstance;		// Used to track hardware's state (One instance can have many physical devices)
	VkPhysicalDevice							_mGraphicsCard;		// Of the potentially many devices, this renderer will only utilize one
	vector<VkPhysicalDevice>					_mPhysicalDevices;	// Every device in the system Vulkan can put a handle on
	VkDevice									_mLogicalDevice;	// Abstraction of the hardware
	VkSurfaceKHR								_mWindowSurface;	// The window "Surface" Vulkan will be drawing onto

	VkResult									initVulkanGraphicsPipeline();
	VkResult									initInstance();
	VkResult									chooseAPhysicalDevice();
	VkResult									initPhysicalDevice();
	VkResult									initLogicalDevice();

	void										CreateVulkanWindowSurface();
public:
	VK_Renderer();
	~VK_Renderer();

	void										GameLoop();
};