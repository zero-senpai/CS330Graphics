#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
    // Macro for window title
    const char* const WINDOW_TITLE = "5-3 Assignment Jake B";

    // Main GLFW window
    GLFWwindow* g_Window = nullptr;

    // Scene manager object for managing the 3D scene prepare and render
    SceneManager* g_SceneManager = nullptr;
    // Shader manager object for dynamic interaction with the shader code
    ShaderManager* g_ShaderManager = nullptr;
    // View manager object for managing the 3D view setup and projection to 2D
    ViewManager* g_ViewManager = nullptr;
}

// Function declarations - all functions that are called manually
bool InitializeGLFW();
bool InitializeGLEW();
void initLighting();  // Declare the lighting initialization function

/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
    // if GLFW fails initialization, then terminate the application
    if (InitializeGLFW() == false)
    {
        return(EXIT_FAILURE);
    }

    // try to create a new shader manager object
    g_ShaderManager = new ShaderManager();
    // try to create a new view manager object
    g_ViewManager = new ViewManager(g_ShaderManager);

    // try to create the main display window
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

    // if GLEW fails initialization, then terminate the application
    if (InitializeGLEW() == false)
    {
        return(EXIT_FAILURE);
    }

    // load the shader code from the external GLSL files
    g_ShaderManager->LoadShaders(
        "../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    // Initialize lighting setup
    initLighting();  // Call the lighting initialization function

    // try to create a new scene manager object and prepare the 3D scene
    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    // loop will keep running until the application is closed 
    // or until an error has occurred
    while (!glfwWindowShouldClose(g_Window))
    {
        // Enable z-depth
        glEnable(GL_DEPTH_TEST);

        // Clear the frame and z buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // convert from 3D object space to 2D view
        g_ViewManager->PrepareSceneView();

        // refresh the 3D scene
        g_SceneManager->RenderScene();

        // Flips the the back buffer with the front buffer every frame.
        glfwSwapBuffers(g_Window);

        // query the latest GLFW events
        glfwPollEvents();
    }

    // clear the allocated manager objects from memory
    if (NULL != g_SceneManager)
    {
        delete g_SceneManager;
        g_SceneManager = NULL;
    }
    if (NULL != g_ViewManager)
    {
        delete g_ViewManager;
        g_ViewManager = NULL;
    }
    if (NULL != g_ShaderManager)
    {
        delete g_ShaderManager;
        g_ShaderManager = NULL;
    }

    // Terminates the program successfully
    exit(EXIT_SUCCESS);
}

/***********************************************************
 *	InitializeGLFW()
 *
 *  This function is used to initialize the GLFW library.
 ***********************************************************/
bool InitializeGLFW()
{
    // GLFW: initialize and configure library
    glfwInit();

#ifdef __APPLE__
    // set the version of OpenGL and profile to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // set the version of OpenGL and profile to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // GLFW: end -------------------------------

    return(true);
}

/***********************************************************
 *	InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
    // GLEW: initialize
    GLenum GLEWInitResult = GLEW_OK;

    // try to initialize the GLEW library
    GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }
    // GLEW: end -------------------------------

    // Displays a successful OpenGL initialization message
    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

    return(true);
}

/***********************************************************
 *  initLighting()
 *
 *  This function sets up the lighting parameters for the scene.
 ***********************************************************/
void initLighting() {
    glEnable(GL_LIGHTING);  // Enable the lighting system
    glEnable(GL_LIGHT0);    // Enable the first light source (GL_LIGHT0)
    glEnable(GL_DEPTH_TEST); // Enable depth testing for correct rendering order

    // Light properties
    GLfloat lightPos[] = { 1.0f, 1.0f, 1.0f, 0.0f };  // Directional light
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };  // Ambient color
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };  // Diffuse color
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Specular color

    // Apply properties to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // Optional: Set material properties for objects being drawn
    GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat materialShininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);
}
