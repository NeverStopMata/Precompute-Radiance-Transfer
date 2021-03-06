﻿// Include standard headers
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>
#include "GL\freeglut.h"
// Include GLFW
#include <glfw/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "common/objloader.hpp"
#include "common/vboindexer.hpp"
#include "Scene.h"
#include "Light.h"
#include "Controller.h"
#include "OpenCL_Math.h"
#include "BRDF_Manager.h"
int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "PRT_SH", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// We would expect width and height to be 1024 and 768
	int windowWidth = 1024;
	int windowHeight = 768;
	// But on MacOS X with a retina screen it'll be 1024*2 and 768*2, so we get the actual framebuffer size:
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	
	GLuint Texture = loadBMP_custom("blue.bmp");


	SampleSet Ss(64, 4);
	Scene scene("test.obj", Ss,false);
	
	Light simpleLight(0.8f, Ss);
	BRDF_Manager brdfMnger(Ss);
	float* verTransMats = new float[scene.numIndices * 256];
	float* transLightCoes = new float[scene.numIndices * 16];
	scene.ExportAllTransMats(verTransMats);
	OpenCL_Math CLtool;

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, scene.indexed_vertices.size() * sizeof(glm::vec3), &scene.indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, scene.indexed_uvs.size() * sizeof(glm::vec2), &scene.indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, scene.indexed_normals.size() * sizeof(glm::vec3), &scene.indexed_normals[0], GL_STATIC_DRAW);
	
	GLuint specBritnessbuffer;
	glGenBuffers(1, &specBritnessbuffer);
	/*glBindBuffer(GL_ARRAY_BUFFER, specBritnessbuffer);
	glBufferData(GL_ARRAY_BUFFER, scene.indexed_normals.size() * sizeof(float), &scene.indexed_normals[0], GL_STATIC_DRAW);*/

	vector<float>DefuseCoeffsVecs;
	vector<float>DefuseCoeffsVecs_simple;
	for (int i = 0; i < scene.indexed_coeffsVecList.size(); i++)
	{
		for (int j = 0; j < 16; j++)
		{
			DefuseCoeffsVecs.push_back(scene.indexed_coeffsVecList[i].Array[j] + scene.indexed_IRcoesVecList[i].Array[j]);
			DefuseCoeffsVecs_simple.push_back(scene.indexed_coeffsVecList[i].Array[j]);
		}
	}



	GLuint defuseCoeffbuffer;
	glGenBuffers(1, &defuseCoeffbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer);
	glBufferData(GL_ARRAY_BUFFER, DefuseCoeffsVecs.size() * sizeof(float), &DefuseCoeffsVecs[0], GL_STATIC_DRAW);

	GLuint defuseCoeffbuffer_simple;
	glGenBuffers(1, &defuseCoeffbuffer_simple);
	glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer_simple);
	glBufferData(GL_ARRAY_BUFFER, DefuseCoeffsVecs_simple.size() * sizeof(float), &DefuseCoeffsVecs_simple[0], GL_STATIC_DRAW);

	GLuint transLightCoesbuffer;
	glGenBuffers(1, &transLightCoesbuffer);


	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene.indices.size() * sizeof(unsigned short), &scene.indices[0], GL_STATIC_DRAW);





	
	//// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("vertex.glsl", "fragment.glsl");
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint lightCoeffsID = glGetUniformLocation(programID, "lightCoeffs");
	GLuint brdfCoeffsID = glGetUniformLocation(programID, "brdfCoeffs");
	GLuint eyePosWorldID = glGetUniformLocation(programID, "EyePos_worldspace"); 
	GLuint isInterRefID = glGetUniformLocation(programID, "isInterReflect"); 


	
	float lastTime = glfwGetTime();
	float currentTime = 0.0, fps = 100.0;
	int nbFrames = 0;

	Controller controller;
	

	
	do {
		
		glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

									  // We don't use bias in the shader, but instead we draw back faces, 
									  // which are already separated from the front faces by a small distance 
									  // (if your geometry is made this way)
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

							 // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);

		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowWidth, windowHeight); // Render on the whole framebuffer, complete from the lower left corner to the upper right

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

							 // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		// Compute the MVP matrix from keyboard and mouse input
		
		controller.computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = controller.getProjectionMatrix();
		glm::mat4 ViewMatrix = controller.getViewMatrix();
		//ViewMatrix = glm::lookAt(glm::vec3(14,6,4), glm::vec3(0,1,0), glm::vec3(0,1,0));
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		//simpleLight.RotateLight(90.0f + 30.0f * sin(1.0 * currentTime), 90.0f + 30.0f * cos(1.0 * currentTime));
		simpleLight.RotateLight(60.0f, 120.0f);
		glm::mat4 lightCoeffs = simpleLight.getRotatedCoeffsMatrix();
		glm::mat4 brdfCoeffs = brdfMnger.getUnRotCoeffsMatrix();
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(lightCoeffsID, 1, GL_FALSE, &lightCoeffs[0][0]);
		glUniformMatrix4fv(brdfCoeffsID, 1, GL_FALSE, &brdfCoeffs[0][0]); 
		glUniform1i(isInterRefID, controller.getIsInterReflect());
		vec3 tempEyePosWorld = controller.position;
		glUniform3f(eyePosWorldID, tempEyePosWorld.x, tempEyePosWorld.y, tempEyePosWorld.z);
		
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);
		CLtool.GetTransLightCoes(scene.numIndices, simpleLight.rotatedCoeffs.Array, verTransMats, transLightCoes);
		glBindBuffer(GL_ARRAY_BUFFER, transLightCoesbuffer);
		glBufferData(GL_ARRAY_BUFFER, scene.indexed_normals.size() * 16 * sizeof(float), transLightCoes, GL_STATIC_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, specBritnessbuffer);
		//glBufferData(GL_ARRAY_BUFFER, scene.indexed_normals.size() * sizeof(float), specBrightness, GL_STATIC_DRAW);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer);
		glVertexAttribPointer(
			3,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16*sizeof(float),                                // stride
			(void*)0                          // array buffer offset
			);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer);
		glVertexAttribPointer(
			4,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(4 * sizeof(float))                         // array buffer offset
			);

		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer);
		glVertexAttribPointer(
			5,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(8 * sizeof(float))                          // array buffer offset
			);

		glEnableVertexAttribArray(6);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer);
		glVertexAttribPointer(
			6,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(12* sizeof(float))                         // array buffer offset
			);


		glEnableVertexAttribArray(7);
		glBindBuffer(GL_ARRAY_BUFFER, transLightCoesbuffer);
		glVertexAttribPointer(
			7,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)0                          // array buffer offset
			);

		glEnableVertexAttribArray(8);
		glBindBuffer(GL_ARRAY_BUFFER, transLightCoesbuffer);
		glVertexAttribPointer(
			8,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(4 * sizeof(float))                         // array buffer offset
			);

		glEnableVertexAttribArray(9);
		glBindBuffer(GL_ARRAY_BUFFER, transLightCoesbuffer);
		glVertexAttribPointer(
			9,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(8 * sizeof(float))                          // array buffer offset
			);

		glEnableVertexAttribArray(10);
		glBindBuffer(GL_ARRAY_BUFFER, transLightCoesbuffer);
		glVertexAttribPointer(
			10,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(12 * sizeof(float))                         // array buffer offset
			);

		glEnableVertexAttribArray(11);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer_simple);
		glVertexAttribPointer(
			11,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)0                          // array buffer offset
			);

		glEnableVertexAttribArray(12);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer_simple);
		glVertexAttribPointer(
			12,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(4 * sizeof(float))                         // array buffer offset
			);

		glEnableVertexAttribArray(13);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer_simple);
		glVertexAttribPointer(
			13,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(8 * sizeof(float))                          // array buffer offset
			);

		glEnableVertexAttribArray(14);
		glBindBuffer(GL_ARRAY_BUFFER, defuseCoeffbuffer_simple);
		glVertexAttribPointer(
			14,                                // attribute
			4,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			16 * sizeof(float),                                // stride
			(void*)(12 * sizeof(float))                         // array buffer offset
			);

		
		//glEnableVertexAttribArray(7);
		//glBindBuffer(GL_ARRAY_BUFFER, specBritnessbuffer);
		//glVertexAttribPointer(
		//	7,                                // attribute
		//	1,                                // size
		//	GL_FLOAT,                         // type
		//	GL_FALSE,                         // normalized?
		//	0,                                // stride
		//	(void*)0                          // array buffer offset
		//	);
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			scene.indices.size(),    // count
			GL_UNSIGNED_SHORT, // type
			(void*)0           // element array buffer offset
			);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);


		// Draw the triangle !
		// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
		//glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		glDisableVertexAttribArray(0);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		nbFrames++;
		currentTime = glfwGetTime();
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
											 // printf and reset
			fps = (nbFrames);
			nbFrames = 0;
			lastTime += 1.0;
			cout << "fps:" << fps << endl;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	//for (int i = 0; i < 4; i++)
	//{
	//	glDeleteBuffers(1, &coeffsbuffer[i]);
	//}
	glDeleteBuffers(1, &defuseCoeffbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	delete[] verTransMats;
	delete[] transLightCoes;
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

