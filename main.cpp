#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <algorithm>

gps::Window myWindow;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
gps::Shader myBasicShader;

gps::Camera myCamera(
	glm::vec3(3000.0f, 5000.0f, 2000.0f),
	glm::vec3(40.0f, 10.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLuint cameraPositionLoc;

GLfloat cameraSpeed = 10.0f;
double pitch = 0.0f, yaw = 0.0f;

GLboolean pressedKeys[1024];

gps::Model3D sun;
gps::Model3D mercury;
gps::Model3D venus;
gps::Model3D earth;
gps::Model3D moon;
gps::Model3D mars;
gps::Model3D jupyter;
gps::Model3D saturn;
gps::Model3D saturn_ring;
gps::Model3D uranus;
gps::Model3D neptun;

GLfloat angle = 0.0f;

float delta = 0.0f;
float speed = 1.0f;
float movementSpeed = 0.09f;
double lastTimeStamp = glfwGetTime();

void updateDelta(double elapsedSeconds) {
	delta += movementSpeed * elapsedSeconds;
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 1.0f, far_plane = 5.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	double yawMouse, pitchMouse;
	glfwGetCursorPos(myWindow.getWindow(), &yawMouse, &pitchMouse);
	myCamera.rotate(pitchMouse / 4, yawMouse / 4);
}

void processMovement() {
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_1)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_LINE_SMOOTH);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_2)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_3)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_4)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LINE_SMOOTH);
	}
	if (pressedKeys[GLFW_KEY_UP]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 0.01f;
	}
	if (pressedKeys[GLFW_KEY_E]) {
		angle += 0.01f;
	}
	if (pressedKeys[GLFW_KEY_H]) {
		pitch -= 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_K]) {
		pitch += 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_J]) {
		yaw -= 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_U]) {
		yaw += 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_0]) {
		speed *= 1.2;
	}
	if (pressedKeys[GLFW_KEY_9]) {
		speed /= 1.2;
	}
}

void initOpenGLWindow() {
	myWindow.Create(1920, 1080, "Galaxy");

}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);	
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(myWindow.getWindow(), 960, 700);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void initModels() {
	sun.LoadModel("models/planet/sun.obj");
	mercury.LoadModel("models/planet/mercury.obj");
	venus.LoadModel("models/planet/venus.obj");
	earth.LoadModel("models/planet/earth.obj");
	moon.LoadModel("models/planet/moon.obj");
	mars.LoadModel("models/planet/mars.obj");
	jupyter.LoadModel("models/planet/jupyter.obj");
	saturn.LoadModel("models/planet/saturn.obj");
	saturn_ring.LoadModel("models/planet/saturn_circle.obj");
	uranus.LoadModel("models/planet/uranus.obj");
	neptun.LoadModel("models/planet/neptun.obj");
}

void initShaders() {
	myBasicShader.loadShader(
		"shaders/basic.vert",
		"shaders/basic.frag");
	
	faces.push_back("skybox/stars.jpg");
	faces.push_back("skybox/stars.jpg");
	faces.push_back("skybox/stars.jpg");
	faces.push_back("skybox/stars.jpg");
	faces.push_back("skybox/stars.jpg");
	faces.push_back("skybox/stars.jpg");

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	mySkyBox.Load(faces);
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 10000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	cameraPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "viewPos");
	glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));
}

void renderSun(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0))
		* glm::rotate(glm::mat4(1.0f), 1997.0f*0.001f*delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	sun.Draw(shader);
}

void renderMercury(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 47.87f * 0.001f * speed*delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(46+200, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 10.8f* 0.001f * speed*delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	mercury.Draw(shader);
}

void renderVenus(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 35.02f * 0.001f * speed*delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(107+200, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 6.5f * 0.001f * speed*delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	venus.Draw(shader);
}

void renderEarthAndMoon(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 29.78f * 0.001f * speed*delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(147+200, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 1574.0f * 0.001f * speed*delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	earth.Draw(shader);

	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) * 
		glm::rotate(glm::mat4(1.0f), 29.78f * 0.001f * speed*delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(147+200, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 3683.0f*0.001f * speed*delta, glm::vec3(0, 1, 1)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(10, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	moon.Draw(shader);
}

void renderMars(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 24.07f * 0.001f * speed*delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(205+200, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 866.0f * 0.001f * speed * delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	mars.Draw(shader);
}

void renderJupyter(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 13.07f * 0.001f * speed * delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(741+200, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 45583.0f * 0.001f * speed * delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	jupyter.Draw(shader);
}

void renderSaturn(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 13.07f * 0.001f * speed * delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(1350, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 36840.0f * 0.001f * speed * delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	saturn.Draw(shader);

	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 13.07f * 0.001f * speed * delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(1350, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 36840.0f * 0.001f * delta, glm::vec3(0, 1, 0));;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	saturn_ring.Draw(shader);
}

void renderUranus(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 6.81f * 0.001f * speed * delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(2250, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 14794.0f * 0.001f * speed * delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	uranus.Draw(shader);
}

void renderNeptun(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 5.43f * 0.001f * speed * delta, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(3950, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 9719.0f * 0.001f * speed * delta, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	neptun.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.8, 0.8, 0.8, 1.0);

	view = myCamera.getViewMatrix();
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	
	myBasicShader.useShaderProgram();	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	view = glm::lookAt(glm::vec3(0.0f, 10.0f, 2.5f),
		glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	renderSun(myBasicShader);
	renderMercury(myBasicShader);
	renderVenus(myBasicShader);
	renderEarthAndMoon(myBasicShader);
	renderMars(myBasicShader);
	renderJupyter(myBasicShader);
	renderSaturn(myBasicShader);
	renderUranus(myBasicShader);
	renderNeptun(myBasicShader);

	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	myWindow.Delete();
}

int main(int argc, const char* argv[]) {
	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
	setWindowCallbacks();

	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		double currentTimeStamp = glfwGetTime();
		updateDelta(currentTimeStamp - lastTimeStamp);
		lastTimeStamp = currentTimeStamp;

		processMovement();
		renderScene();
		
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}
	cleanup();

	return EXIT_SUCCESS;
}