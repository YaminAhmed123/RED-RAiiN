#include <VULKAN_PHYSICAL_DEVICE.hpp>
#include <VULKAN_QUEUE.hpp>
#include <cstdint>
#include <string.h>

#define DEBUG_ON
#include <ETL.hpp>

void VULKAN_PHYSICAL_DEVICE::INIT_META_DATA()
{
    VkPhysicalDeviceProperties PH_DEVICE_PROPS{};
    vkGetPhysicalDeviceProperties(this->PH_DEVICE, &PH_DEVICE_PROPS);
    uint32_t major = VK_API_VERSION_MAJOR(PH_DEVICE_PROPS.apiVersion);
    uint32_t minor = VK_API_VERSION_MINOR(PH_DEVICE_PROPS.apiVersion);
    uint32_t patch = VK_API_VERSION_PATCH(PH_DEVICE_PROPS.apiVersion);
    this->META_DATA.apiVersion = std::string{std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch)};
    this->META_DATA.deviceName = std::string(PH_DEVICE_PROPS.deviceName, strnlen(PH_DEVICE_PROPS.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE));

    VkPhysicalDeviceDriverProperties PROPERTIES{};
    PROPERTIES.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

    VkPhysicalDeviceProperties2 PROP_2{};
    PROP_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    PROP_2.pNext = &PROPERTIES;
    vkGetPhysicalDeviceProperties2(this->PH_DEVICE, &PROP_2);
    this->META_DATA.driverName = std::string(PROPERTIES.driverName, strnlen(PROPERTIES.driverName, VK_MAX_DRIVER_NAME_SIZE));
    this->META_DATA.driverInfo = std::string(PROPERTIES.driverInfo, strnlen(PROPERTIES.driverInfo, VK_MAX_DRIVER_INFO_SIZE));
}

static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR& surface)
{
    QueueFamilyIndices indices = QUEUE::findQueueFamilies(device, surface);

    bool DRIVER_API_SUPPRT = false;
    VkPhysicalDeviceProperties PH_DEVICE_PROPS{};
    vkGetPhysicalDeviceProperties(device, &PH_DEVICE_PROPS);
    if(PH_DEVICE_PROPS.apiVersion < VK_API_VERSION_1_3)
    {
        DRIVER_API_SUPPRT = false;
    } else{
        DRIVER_API_SUPPRT = true;
    }

    return indices.isComplete() && DRIVER_API_SUPPRT && true; // isSwapChainSuitable(device, surface);    WARNING SWAPCHAIN CHECK WILL COME IN FUTURE
}

void VULKAN_PHYSICAL_DEVICE::INIT_DEVICE(WINDOW& WIN, VULKAN_INSTANCE& INSTANCE)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(INSTANCE.getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find devices with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(INSTANCE.getInstance(), &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, WIN.getSurface())) {
            this->PH_DEVICE = device;
            break;
        }
    }

    if (this->PH_DEVICE == VK_NULL_HANDLE) {
        DEBUG_ERR("NO SUITABLE GPU WAS FOUND EITHER DUE TO MISSING DRIVERS OR SIMPLY OLD HARDWARE WITH NO SUPPORT FOR VULKAN_1.3");
        DEBUG_ERR("IN CASE OF OLD HARDWARE AND NO SUPPORTED DRIVERS BEING AVAILABLE TRY UPDATING DRIVERS");
        DEBUG_ERR("IF THIS DOESNT HELP THEN THE DEVICE IS TOO OLD AND THERE IS NO WAY OF RUNNING THE ENGINE !");
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    this->INIT_META_DATA();
}


VkPhysicalDevice VULKAN_PHYSICAL_DEVICE::GET_VK_HANDLE_TO_DEVICE()
{
    return this->PH_DEVICE;
}

void VULKAN_PHYSICAL_DEVICE_PRINTABLE_META_DATA::PRINT_META_INFO()
{
    std::cout << "Device Name: " << this->deviceName << std::endl;
    std::cout << "Driver Name: " << this->driverName << std::endl;
    std::cout << "Driver Info: " << this->driverInfo << std::endl;
    std::cout << "Api Version: " << this->apiVersion << std::endl;
}

void VULKAN_PHYSICAL_DEVICE_PRINTABLE_META_DATA::PRINT_META_INFO_DBG()
{
    #ifdef DEBUG_ON
    std::cout << std::endl;
    std::cout << "--------------DEVICE-INFORMATION--------------" << std::endl;
    DEBUG_LOG(std::string("Device Name: " + this->deviceName));
    DEBUG_LOG(std::string("Driver Name: " + this->driverName));
    DEBUG_LOG(std::string("Driver Info: " + this->driverInfo));
    DEBUG_LOG(std::string("Vulkan API Version: " + this->apiVersion));
    std::cout << "------------DEVICE-INFORMATION-END------------" << std::endl;
    std::cout << std::endl;
    #endif
}
