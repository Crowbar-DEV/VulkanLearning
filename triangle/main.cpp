//following two lines let GLFW load the vulkan header
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

//Global Constants
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
