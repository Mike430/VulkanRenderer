#pragma once
#include <iostream>
#include <vector>
#include "vulkan.h"

#pragma comment (lib, "vulkan-1.lib")

using namespace std;

class VK_Renderer
{
private:
	VkApplicationInfo					_mAppInfo;
	VkInstanceCreateInfo				_mInstanceCreateInfo;
	//const VkAllocationCallbacks			_mAllocator;

	VkInstance							_mVkInstance; // Used to track hardware's state (One instance can have many physical devices)
	VkPhysicalDevice					_mGraphicsCard; // of the potentially many devices, this renderer will only utilize one
	std::vector<VkPhysicalDevice>		_mPhysicalDevices; // Every device in the system Vulkan can put a handle on
	VkDevice							_mLogicalDevice;

	VkResult VkInit();
public:
	VK_Renderer();
	~VK_Renderer();
};

