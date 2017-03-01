#include "VK_Renderer.h"


VK_Renderer::VK_Renderer()
{
	cout << "VK_Renderer constructor called" << endl;
	if (VkInit() == VK_SUCCESS)
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
	vkDestroyInstance(_mVkInstance, nullptr);

	cout << "VK_Renderer destructor completed" << endl;
}


VkResult VK_Renderer::VkInit()
{
	VkResult returnResult = VK_SUCCESS;

	_mAppInfo = {}; // information about your application and Vulkan compatibility
	_mInstanceCreateInfo = {}; // informations about the specific type of Vulkan instance you wish to create

	_mAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	_mAppInfo.pApplicationName = "Blank Vulkan Window";
	_mAppInfo.engineVersion = 1;
	_mAppInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	_mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_mInstanceCreateInfo.pApplicationInfo = &_mAppInfo;

	returnResult = vkCreateInstance(&_mInstanceCreateInfo, nullptr, &_mVkInstance);

	if (returnResult == VK_SUCCESS)
	{
		uint32_t physicalDeviceCount = 0;
		returnResult = vkEnumeratePhysicalDevices(_mVkInstance, &physicalDeviceCount, nullptr);

		// If there are any vulkan compatible devices
		if (returnResult == VK_SUCCESS)
		{
			// Size the device array appropriately and get the physical device handles
			_mPhysicalDevices.resize(physicalDeviceCount);
			returnResult = vkEnumeratePhysicalDevices(_mVkInstance, &physicalDeviceCount, &_mPhysicalDevices[0]);

			if (returnResult == VK_SUCCESS)
			{
				// #####################################################################################################
				// At this point we know that we can initialise Vulkan, now we need to know if it can be initialised
				// to a descrete GPU. This next section of code finds and sets _mGraphicsCard if a suitable 
				// PhysicalDevice is identified. If more than one qualify, it chooses the one with the most V-RAM
				// #####################################################################################################
				{
					cout << _mPhysicalDevices.size() << " Physical Device(s) have been found on this system." << endl;

					VkPhysicalDeviceProperties physicalDeviceProperties = VkPhysicalDeviceProperties();
					VkPhysicalDeviceProperties temp_PhysicalDeviceProperties = VkPhysicalDeviceProperties();
					int winningIndex = 0;

					for (int i = 0; i < _mPhysicalDevices.size(); i++)
					{
						vkGetPhysicalDeviceProperties(_mPhysicalDevices.at(i), &temp_PhysicalDeviceProperties);

						cout << i << "\nVkHandle\t" << _mPhysicalDevices.at(i) <<
							"\nDevice name:\t" << temp_PhysicalDeviceProperties.deviceName <<
							"\nDevice type:\t" << temp_PhysicalDeviceProperties.deviceType << // typedef enum VKPhysicalDeviceType {0-4} - 2 = Descrete GPU
							"\nDevice cpty:\t" << temp_PhysicalDeviceProperties.limits.maxMemoryAllocationCount << endl;

						if ((physicalDeviceProperties.deviceName == "" ||
							physicalDeviceProperties.limits.maxMemoryAllocationCount < temp_PhysicalDeviceProperties.limits.maxMemoryAllocationCount) &&
							temp_PhysicalDeviceProperties.deviceType == 2)
						{
							physicalDeviceProperties = temp_PhysicalDeviceProperties;
							winningIndex = i;
						}
					}

					if (physicalDeviceProperties.deviceName == "")
					{
						cout << "\nNo intergrated GPU device was found, exiting." << endl;
						return VK_ERROR_INITIALIZATION_FAILED;
					}

					_mGraphicsCard = _mPhysicalDevices.at(winningIndex);
					cout << "\nWe're using the " << physicalDeviceProperties.deviceName << " for graphics\n" << endl;
				}
				// #####################################################################################################
				// We have a GPU, now we can focus on initialising it with a logical Device.
				// #####################################################################################################
				{
					VkPhysicalDeviceFeatures supportedFeatures;
					VkPhysicalDeviceFeatures requiredFeatures = {};
					vkGetPhysicalDeviceFeatures(_mGraphicsCard, &supportedFeatures);

					requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
					requiredFeatures.tessellationShader = VK_TRUE;
					requiredFeatures.geometryShader = VK_TRUE;
					
					const VkDeviceQueueCreateInfo deviceQueueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	nullptr, 0, 0, 1, nullptr };

					const VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1,	&deviceQueueCreateInfo,
						0, nullptr,	0, nullptr,&requiredFeatures };

					returnResult = vkCreateDevice(_mGraphicsCard, &deviceCreateInfo, nullptr, &_mLogicalDevice);

					returnResult == VK_SUCCESS ? cout << "It worked\n\n" : cout << "It did not work\n\n";

					return returnResult;
				}
			}
		}
	}
	return returnResult;
}