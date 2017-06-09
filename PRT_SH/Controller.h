#pragma once
#include <glfw/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

class Controller
{
public:
	Controller();
	~Controller();
	mat4 Controller::getViewMatrix();
	mat4 Controller::getProjectionMatrix();
	void Controller::computeMatricesFromInputs();
public:
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;

	// Initial position : on +Z
	vec3 position;
	// Initial horizontal angle : toward -Z
	float horizontalAngle;
	// Initial vertical angle : none
	float verticalAngle;
	// Initial Field of View
	float initialFoV;

	float speed; // 3 units / second
	float mouseSpeed;
};

