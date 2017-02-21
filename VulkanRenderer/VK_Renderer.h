#pragma once
#include <iostream>
#include <vector>
#include "vulkan.h"

#pragma comment (lib, "vulkan-1.lib")

using namespace std;

class VK_Renderer
{
private:
	VkApplicationInfo					_appInfo;
	VkInstanceCreateInfo				_createInfo;
	//const VkAllocationCallbacks			_allocator;
	VkInstance							_vkInstance; // Used to track hardware's state (One instance can have many physical devices)
	std::vector<VkPhysicalDevice>		_physicalDevices; // Every device in the system Vulkan can put a handle on

	VkResult Vk_RendererInit();
public:
	VK_Renderer();
	~VK_Renderer();
};

