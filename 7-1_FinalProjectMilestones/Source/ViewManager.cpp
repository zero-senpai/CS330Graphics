///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// if orthographic projection is on, this value will be
	// true
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager* pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.5f, 5.5f, 10.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	glfwSetScrollCallback(window, &ViewManager::Scroll_Callback);


	// tell GLFW to capture all mouse events
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// when the first mouse move event is received, this needs to be recorded so that
	// all subsequent mouse moves can correctly calculate the X position offset and Y
	// position offset for proper operation
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// calculate the X offset and Y offset values for moving the 3D camera accordingly
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // reversed since y-coordinates go from bottom to top

	// set the current positions into the last position variables
	gLastX = xMousePos;
	gLastY = yMousePos;

	// Change value of the sensitivity variable
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Adjust the camera's yaw and pitch
	g_pCamera->Yaw += xOffset;
	g_pCamera->Pitch += yOffset;

	//gimbal lock prevention
	if (g_pCamera->Pitch > 89.0f)
		g_pCamera->Pitch = 89.0f;
	if (g_pCamera->Pitch < -89.0f)
		g_pCamera->Pitch = -89.0f;

	// Update the camera's direction
	glm::vec3 front;
	front.x = cos(glm::radians(g_pCamera->Yaw)) * cos(glm::radians(g_pCamera->Pitch));
	front.y = sin(glm::radians(g_pCamera->Pitch));
	front.z = sin(glm::radians(g_pCamera->Yaw)) * cos(glm::radians(g_pCamera->Pitch));
	g_pCamera->Front = glm::normalize(front);
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	float velocity = gDeltaTime * g_pCamera->Zoom;

	// Close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// If the camera object is null, then exit this method
	if (NULL == g_pCamera)
	{
		return;
	}

	// Process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->Position += g_pCamera->Front * gDeltaTime * g_pCamera->Zoom;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->Position -= g_pCamera->Front * gDeltaTime * g_pCamera->Zoom;
	}

	// Process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->Position -= glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * gDeltaTime * g_pCamera->Zoom;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->Position += glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * gDeltaTime * g_pCamera->Zoom;
	}

	// Up and Down movement (vertical panning)
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->Position += g_pCamera->Up * gDeltaTime * g_pCamera->Zoom;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->Position -= g_pCamera->Up * gDeltaTime * g_pCamera->Zoom;
	}

	// Switch to perspective projection
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		bOrthographicProjection = false;
	}

	// Switch to orthographic projection
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		bOrthographicProjection = true;
	}
}

/**********************************************************
*	Scroll_Callback()
*
*	@params: GLFWindow pointer, double xOffset, double yOffeset
*	Handles any events in queue for the scroll wheel input
***********************************************************/
void ViewManager::Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// Adjust zoom or movement speed based on mouse scroll
	if (g_pCamera->Zoom >= 1.0f && g_pCamera->Zoom <= 45.0f)
		g_pCamera->Zoom -= yOffset; // zoom out with scroll down
	if (g_pCamera->Zoom <= 1.0f)
		g_pCamera->Zoom = 1.0f;
	if (g_pCamera->Zoom >= 45.0f)
		g_pCamera->Zoom = 45.0f;
}




/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// Per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// Process any keyboard events that may be waiting in the event queue
	ProcessKeyboardEvents();

	// Get the current view matrix from the camera
	view = glm::lookAt(g_pCamera->Position, g_pCamera->Position + g_pCamera->Front, g_pCamera->Up);

	// Switch between perspective and orthographic projections
	if (bOrthographicProjection)
	{
		float orthoSize = 10.0f;
		projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
	}
	else
	{
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}

	// If the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// Set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// Set the projection matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// Set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}