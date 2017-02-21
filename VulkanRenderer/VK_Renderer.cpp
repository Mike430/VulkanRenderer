#include "VK_Renderer.h"


VK_Renderer::VK_Renderer()
{
	cout << "VK_Renderer constructor called" << endl;
	if (Vk_RendererInit() == VK_SUCCESS)
	{
		cout << "VK_Renderer constructor complete" << endl;
	}
	else
	{
		cout << "VK_Renderer constructor failed" << endl;
		this->~VK_Renderer();
	}

	this->~VK_Renderer();
}


VK_Renderer::~VK_Renderer()
{
	cout << "VK_Renderer destructor called" << endl;
	vkDestroyInstance(_vkInstance, nullptr);

	cout << "VK_Renderer destructor completed" << endl;
}


VkResult VK_Renderer::Vk_RendererInit()
{
	VkResult returnResult = VK_SUCCESS;

	_appInfo = {}; // information about your application and Vulkan compatibility
	_createInfo = {}; // informations about the specific type of Vulkan instance you wish to create

	_appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	_appInfo.pApplicationName = "Blank Vulkan Window";
	_appInfo.engineVersion = 1;
	_appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	_createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_createInfo.pApplicationInfo = &_appInfo;

	returnResult = vkCreateInstance(&_createInfo, nullptr, &_vkInstance);

	if (returnResult == VK_SUCCESS)
	{
		uint32_t physicalDeviceCount = 0;
		returnResult = vkEnumeratePhysicalDevices(_vkInstance, &physicalDeviceCount, nullptr);

		// If there are any vulkan compatible devices
		if (returnResult == VK_SUCCESS)
		{
			// Size the device array appropriately and get the physical device handles
			_physicalDevices.resize(physicalDeviceCount);
			returnResult = vkEnumeratePhysicalDevices(_vkInstance, &physicalDeviceCount, &_physicalDevices[0]);

			if (returnResult == VK_SUCCESS)
			{
				cout << _physicalDevices.size() << " Physical Device(s) have been found on this system." << endl;
				for(int i = 0; i < _physicalDevices.size(); i++)
				{
					VkPhysicalDeviceProperties physicalDeviceProperties;
					VkPhysicalDeviceFeatures physicalDeviceFeatures;
					VkPhysicalDeviceMemoryProperties physicalDeviceMemProps;
					VkQueueFamilyProperties physicalDeviceQueueFamilyProps;
					uint32_t queueCount = 0;

					vkGetPhysicalDeviceProperties(_physicalDevices.at(i), &physicalDeviceProperties);
					vkGetPhysicalDeviceFeatures(_physicalDevices.at(i), &physicalDeviceFeatures);
					vkGetPhysicalDeviceMemoryProperties(_physicalDevices.at(i), &physicalDeviceMemProps);
					vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevices.at(i), &queueCount, &physicalDeviceQueueFamilyProps);

					cout << i << "\nVkHandle\t" << _physicalDevices.at(i) <<
						"\nDevice name:\t" << physicalDeviceProperties.deviceName <<
						"\nDevice type:\t" << physicalDeviceProperties.deviceType << // typedef enum VKPhysicalDeviceType {0-4} - 1 = intergrated GPU
						"\nDevice cpty:\t" << physicalDeviceProperties.limits.maxMemoryAllocationCount << // number of bytes in device local memory
						"\nDevice memT:\t" << physicalDeviceMemProps.memoryTypeCount <<
						"\nDevice memC:\t" << physicalDeviceMemProps.memoryHeapCount <<
						"\nDevice Ques:\t" << physicalDeviceQueueFamilyProps.queueCount << endl;
				}
			}
		}
	}
	return returnResult;
}