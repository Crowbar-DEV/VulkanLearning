#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>

//Global Constants
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


class TriangleApplication {
    public:
        void run(){
            initWindow();
			initVulkan();
			mainLoop();
			cleanup();
		}

    private:
        GLFWwindow* window;
        
        //holds the handle to the VkInstance
        VkInstance instance;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        VkDevice device;

        VkQueue graphicsQueue;

        VkSurfaceKHR surface;

        void initWindow() {
            //initializes glfw library
            glfwInit();

            //these two calls let glfw know we are not creating an opengl context,
            //and to not make the window resizable
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            
            //creating the actual window - width, height, title, monitor to open on, N/A
            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        }

        void initVulkan() {
            createInstance();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
        }

        void createSurface() {
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
        }

        void createLogicalDevice(){
            //creation of logical device involves specifying a bunch of details in structs.
            //much like the Instance creation.
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;

            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures{};

            VkDeviceCreateinfo createInfo{};

            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = &queueCreateInfo;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = 0;

            if(enableValidationLayers){
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
                throw std::runtime_error("logical device could not be created");
            }

            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

        }

        void pickPhysicalDevice(){
            uint32_t deviceCount;

            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if(deviceCount == 0){
                throw std::runtime_error("No physical Devices");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data() );
            
            for(const auto& device : devices){
                if(isDeviceSuitable(device)){
                    physicalDevice = device;
                    break;
                }
            }

            if(physicalDevice == VK_NULL_HANDLE){
                throw std::runtime_error("could not find suitable GPU");
            }
            
        }

        bool isDeviceSuitable(VkPhysicalDevice device){
            //vkphysicaldeviceproperties deviceproperties;
            //vkGetPhysicalDeviceProperties(device, &deviceProperties);

            //VkPhysicalDeviceFeatures deviceFeatures;
            //vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            
            QueueFamilyIndices indices = findQueueFamilies(device);
            return indices.isComplete();
        }

        struct QueueFamilyIndices{
            std::optional<uint32_t> graphicsFamily;

            bool isComplete(){
                return graphicsFamily.has_value();
            }
        };

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for(const auto& queueFamily : queueFamilies){
                if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                    indices.graphicsFamily = i;
                }

                if(indices.isComplete()){
                    break;
                }

                i++;
            }

            return indices;
        }

        void createInstance() {
            //VkApplicationInfo is a struct containing information about the application
            //so that the driver can have a little extra info about what is needed.
            //
            //Many structs in vulkan require you to define the type of struct in 
            //sType.
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); 
            appInfo.apiVersion = VK_API_VERSION_1_0;

            //VkInstanceCreateInfo is a struct that specifies the parameters of a newly
            //created VkInstance
            VkInstanceCreateInfo createInfo{};

            if(enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;
            createInfo.enabledLayerCount = 0;
            
            //Actually creating the vulkan instance, in the third param, we pass
            //the handle we defined as a private member. The actual instance does not
            //get stored in 'result' but the instance is stored in mem and then pointed to
            //by the passed handle. So if we want to use the instance, we must
            //use the handle we defined, in this case 'instance'
            //
            //Most Vulkan functions return a value of type VkResult which is either
            //a succes or an error code. This is stored in result.
            
            //VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);


            //however there is a way to do this without having to store the result.
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
                throw std::runtime_error("failed to create instance");
            }

            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            std::cout << "available extensions:\n";

            for (const auto& extension : extensions){
                std::cout << '\t' << extension.extensionName << '\n';
            }
            
            if(enableValidationLayers && !checkValidationLayerSupport()){
                throw std::runtime_error("validation layers requested but not supported");
            }



        }

        bool checkValidationLayerSupport(){

            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            
            //check wanted validation layers against the available validation layers
            for(const char* layerName : validationLayers){
                bool layerFound = false;
                for(const auto& layerProperties : availableLayers){
                    if(strcmp(layerName, layerProperties.layerName) == 0){
                        layerFound = true;
                        break;
                    }
                }
                if(!layerFound){
                    return false;
                }
            }
            
            return true;


        }

        bool areGlfwExtensionsSupported(std::vector<VkExtensionProperties> extensions){
            uint32_t count = 0;
            uint32_t extensionsSupported = 0;
            const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&count);
            
            for(const auto& e : extensions){
                for(int g = 0; g<count; g++){
                    if(!strcmp(e.extensionName, (const char*)glfwRequiredExtensions[g] ) ){
                        extensionsSupported++;
                    }
                }
            }

            if(extensionsSupported){
                return true;
            }
            return false;
        }

        void mainLoop() {
            while(!glfwWindowShouldClose(window)){
                //we poll the windowing system for events to provide input
                //to the application and also to prove to the window system
                //that the application has not locked up.
                //glfwPollEvents will process whatever events are in the
                //event queue at a given time, processing means that the associated
                //callback functions for each event are called.
                glfwPollEvents();
            }
        }

        void cleanup() {
            glfwDestroyWindow(window);
            glfwTerminate();
            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
            vkDestroyDevice(device, nullptr);
        }
};

int main() {
    TriangleApplication app;

    try{
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
