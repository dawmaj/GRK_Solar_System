#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>


#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

#define MOVEMENT_SPEED 2
#define SIZE 1
//shaders
GLuint programColor;
GLuint programTexture;
GLuint programSkyBox;

//textures
GLuint test_texture;
GLuint CubemapTexture;

//rotation angles
float y_rotation_angle;
float around_rotation_angle;

Core::Shader_Loader shaderLoader;

//models and Vertex array
obj::Model shipModel;
obj::Model sphereModel;

float cubemapVertices[] = {
	// positions          
	-SIZE,  SIZE, -SIZE,
	-SIZE, -SIZE, -SIZE,
	SIZE, -SIZE, -SIZE,
	SIZE, -SIZE, -SIZE,
	SIZE,  SIZE, -SIZE,
	-SIZE,  SIZE, -SIZE,

	-SIZE, -SIZE,  SIZE,
	-SIZE, -SIZE, -SIZE,
	-SIZE,  SIZE, -SIZE,
	-SIZE,  SIZE, -SIZE,
	-SIZE,  SIZE,  SIZE,
	-SIZE, -SIZE,  SIZE,

	SIZE, -SIZE, -SIZE,
	SIZE, -SIZE,  SIZE,
	SIZE,  SIZE,  SIZE,
	SIZE,  SIZE,  SIZE,
	SIZE,  SIZE, -SIZE,
	SIZE, -SIZE, -SIZE,

	-SIZE, -SIZE,  SIZE,
	-SIZE,  SIZE,  SIZE,
	SIZE,  SIZE,  SIZE,
	SIZE,  SIZE,  SIZE,
	SIZE, -SIZE,  SIZE,
	-SIZE, -SIZE,  SIZE,

	-SIZE,  SIZE, -SIZE,
	SIZE,  SIZE, -SIZE,
	SIZE,  SIZE,  SIZE,
	SIZE,  SIZE,  SIZE,
	-SIZE,  SIZE,  SIZE,
	-SIZE,  SIZE, -SIZE,

	-SIZE, -SIZE, -SIZE,
	-SIZE, -SIZE,  SIZE,
	SIZE, -SIZE, -SIZE,
	SIZE, -SIZE, -SIZE,
	-SIZE, -SIZE,  SIZE,
	SIZE, -SIZE,  SIZE
};

GLuint SkyboxVertexBuffer, SkyboxVertexAttributes;

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	}
}

glm::mat4 createCameraMatrix()
{
	// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0,1,0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

//draw things without texture
void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

//draw thing with texture
void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint id)
{
	GLuint program = programTexture;

	glUseProgram(program);
	Core::SetActiveTexture(id, "samp_tex", program, 0);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

//rendar all planets
void renderPlanets()
{
	y_rotation_angle += 0.001;
	around_rotation_angle += 0.0001;

	glm::vec3 myRotationAxis(0.0f, 1.0f, 0.0f);
	//Sun
	glm::mat4 SunModelMatrix = glm::scale(glm::vec3(4.0f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle/20, myRotationAxis);
	drawObjectTexture(&sphereModel, SunModelMatrix, test_texture);

	//Merkury
	glm::mat4 MercuryModelMatrix = glm::rotate(glm::mat4(1.0f), (glm::mediump_float)around_rotation_angle*2.0f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(0, 0, 7.0f)) * glm::scale(glm::vec3(0.3f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle*1.5f, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, MercuryModelMatrix, test_texture);

	//Venus
	glm::mat4 venusmodelmatrix = glm::rotate(glm::mat4(1.0f), (glm::mediump_float)around_rotation_angle*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(0, 0, 12.0f)) * glm::scale(glm::vec3(0.95f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myrotationaxis);
	drawObjectTexture(&sphereModel, venusmodelmatrix, test_texture);

	//Earth
	glm::mat4 EarthModelMatrix = glm::rotate(glm::mat4(1.0f), (glm::mediump_float)around_rotation_angle*0.6f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(-21.0f, 0, 0)) * glm::scale(glm::vec3(1.0f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle*2.0f, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, EarthModelMatrix, test_texture);

	//Earth Moon
	glm::mat4 MoonEarthModelMatrix = glm::rotate(EarthModelMatrix, (glm::mediump_float)around_rotation_angle*0.005f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(0, 0, 3.0f)) * glm::scale(glm::vec3(0.35f));
	drawObjectTexture(&sphereModel, MoonEarthModelMatrix, test_texture);

	//Mars
	glm::mat4 MarsModelMatrix = glm::rotate(glm::mat4(1.0f), (glm::mediump_float)around_rotation_angle*0.3f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(-3.0f, 0, 31.0f)) * glm::scale(glm::vec3(0.5f)) * glm::rotate(glm::mat4(0.8f), (glm::mediump_float)y_rotation_angle*3.0f, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, MarsModelMatrix, test_texture);

	//Jupiter
	glm::mat4 JupiterModelMatrix = glm::rotate(glm::mat4(0.1f), (glm::mediump_float)around_rotation_angle*0.1f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(3.0f, 0, 48.0f)) * glm::scale(glm::vec3(2.8f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle*2.0f, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, JupiterModelMatrix, test_texture);

	//Jupiter Moons
	//Io
	glm::mat4 MoonJupiterModelMatrix = glm::rotate(JupiterModelMatrix, (glm::mediump_float)around_rotation_angle*4*MOVEMENT_SPEED, -myRotationAxis) * glm::translate(glm::vec3(0, 0, 2.0f)) * glm::scale(glm::vec3(0.12f));
	drawObjectTexture(&sphereModel, MoonJupiterModelMatrix, test_texture);
	//Europa
	glm::mat4 Moon2JupiterModelMatrix = glm::rotate(JupiterModelMatrix, (glm::mediump_float)around_rotation_angle*0.25f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(-2.3f, 0, 0)) * glm::scale(glm::vec3(0.15f));
	drawObjectTexture(&sphereModel, Moon2JupiterModelMatrix, test_texture);

	//Saturn
	glm::mat4 SaturnModelMatrix = glm::rotate(glm::mat4(0.25f), (glm::mediump_float)around_rotation_angle*0.05f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(-3.0f, 0, -56.0f)) * glm::scale(glm::vec3(2.2f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle*2.2f, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, SaturnModelMatrix, test_texture);

	//Uranus
	glm::mat4 UraniumModelMatrix = glm::rotate(glm::mat4(0.75f), (glm::mediump_float)around_rotation_angle*0.04f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(-60.0f, 0, 0)) * glm::scale(glm::vec3(1.5f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle*1.5f, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, UraniumModelMatrix, test_texture);

	//Neptun
	glm::mat4 NeptuneModelMatrix = glm::rotate(glm::mat4(0.45f), (glm::mediump_float)around_rotation_angle*0.03f*MOVEMENT_SPEED, myRotationAxis) * glm::translate(glm::vec3(70.0f, 0, 0)) * glm::scale(glm::vec3(1.6f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, NeptuneModelMatrix, test_texture);
}

//render ship
void renderShip()
{
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0, -0.25f, 0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
}

//render skybox
void renderSkybox()
{
	glUseProgram(programSkyBox);
	glm::mat4 view = glm::mat4(glm::mat3(cameraMatrix));
	glUniformMatrix4fv(glGetUniformLocation(programSkyBox, "view"), 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(programSkyBox, "perspective"), 1, GL_FALSE, (float*)&perspectiveMatrix);

	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(SkyboxVertexAttributes);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	//glDepthMask(GL_TRUE);

	glUseProgram(0);

}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	renderShip();
	renderPlanets();
	renderSkybox();

	glutSwapBuffers();
}

void init()
{
	y_rotation_angle = around_rotation_angle = 0;
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSkyBox = shaderLoader.CreateProgram("shaders/shader_box.vert", "shaders/shader_box.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	test_texture = Core::LoadTexture("textures/grid.png");
	
	//load Cubemap texture
	std::vector<std::string> faces
	{
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/back.jpg",
		"textures/skybox/front.jpg"
	};
	CubemapTexture = Core::LoadCubemap(faces);

	//Skybox settings
	glGenBuffers(1, &SkyboxVertexBuffer);
	glGenVertexArrays(1, &SkyboxVertexAttributes);
	glBindVertexArray(SkyboxVertexAttributes);
	glBindBuffer(GL_ARRAY_BUFFER, SkyboxVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), cubemapVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Solar system!");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
