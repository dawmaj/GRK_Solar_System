#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>


#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
//#include "glm/gtc/matrix_transform.hpp"

GLuint programColor;
GLuint programTexture;
GLuint tex_id;
GLuint earth;

float y_rotation_angle;
float around_rotation_angle;
float moon_rotation_angle;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;
glm::vec3 MercuryPos = glm::vec3(0.0f, 0.0f, 4.0f);

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

void renderScene()
{
	y_rotation_angle += 0.001;
	around_rotation_angle += 0.0003;
	moon_rotation_angle += 0.002;
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	// (Bardziej elegancko byloby przekazac je jako argumenty do funkcji, ale robimy tak dla uproszczenia kodu.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	
	glm::vec3 myRotationAxis(0.0f, 1.0f, 0.0f);
	//Sun - skalowanie * rotacja (wzgledem osi y)
	glm::mat4 SunModelMatrix = glm::scale(glm::vec3(3.0f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, SunModelMatrix, tex_id);
	   
	////Merkury - (rotacja * translacja)(obrot wzgledem centrum) * translacja(przesuniecie od srodka) * skalowanie * rotacja(rotacja wokol osi y)
	//glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	glm::mat4 MercuryModelMatrix = glm::rotate(glm::mat4(1.2f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(0, 0, 6.0f)) * glm::scale(glm::vec3(0.3f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, MercuryModelMatrix, tex_id);
	//get mercury position vector
	glm::vec3 mercury_center(MercuryModelMatrix * glm::vec4(1.0f));
	//Moon
	glm::mat4 MoonModelMatrix = glm::rotate(MercuryModelMatrix, (glm::mediump_float)moon_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(0, 0, 2.0f)) * glm::scale(glm::vec3(0.2f));
	drawObjectTexture(&sphereModel, MoonModelMatrix, tex_id);
	////Wenus
	glm::mat4 venusmodelmatrix = glm::rotate(glm::mat4(0.75f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(4.5f, 0, 10.0f)) * glm::scale(glm::vec3(0.95f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myrotationaxis);
	drawObjectTexture(&sphereModel, venusmodelmatrix, tex_id);
	//Ziemia
	glm::mat4 EarthModelMatrix = glm::rotate(glm::mat4(0.8f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(-3.0f, 0, 16.5f)) * glm::scale(glm::vec3(1.0f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, EarthModelMatrix, earth);

	glm::vec3 earth_center(EarthModelMatrix * glm::vec4(1.0f));
	//Ksiê¿yc
	glm::mat4 MoonEarthModelMatrix = glm::rotate(EarthModelMatrix, (glm::mediump_float)moon_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(0, 0, 1.0f)) * glm::scale(glm::vec3(0.25f));
	drawObjectTexture(&sphereModel, MoonModelMatrix, tex_id); 
	//Mars
	glm::mat4 MarsModelMatrix = glm::rotate(glm::mat4(0.8f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(-3.0f, 0, 21.5f)) * glm::scale(glm::vec3(0.5f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, MarsModelMatrix, tex_id);
	//Jowisz
	glm::mat4 JupiterModelMatrix = glm::rotate(glm::mat4(0.1f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(8.0f, 0, 26.5f)) * glm::scale(glm::vec3(2.5f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, JupiterModelMatrix, tex_id);

	glm::vec3 jupiter_center(EarthModelMatrix * glm::vec4(1.0f));
	//Ksiê¿yc
	glm::mat4 MoonJupiterModelMatrix = glm::rotate(JupiterModelMatrix, (glm::mediump_float)moon_rotation_angle, -myRotationAxis) * glm::translate(glm::vec3(0, 0, 1.4f)) * glm::scale(glm::vec3(0.25f));
	drawObjectTexture(&sphereModel, MoonJupiterModelMatrix, tex_id);
	glm::mat4 Moon2JupiterModelMatrix = glm::rotate(JupiterModelMatrix, (glm::mediump_float)moon_rotation_angle, myRotationAxis+0.2f) * glm::translate(glm::vec3(0, 0, 1.9f)) * glm::scale(glm::vec3(0.45f));
	drawObjectTexture(&sphereModel, Moon2JupiterModelMatrix, tex_id);
	//Saturn
	glm::mat4 SaturnModelMatrix = glm::rotate(glm::mat4(0.25f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(-6.0f, 0, 35.0f)) * glm::scale(glm::vec3(2.0f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, SaturnModelMatrix, tex_id);
	//Uran
	glm::mat4 UraniumModelMatrix = glm::rotate(glm::mat4(0.75f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(4.0f, 0, 47.5f)) * glm::scale(glm::vec3(1.8f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, UraniumModelMatrix, tex_id);
	//Neptun
	glm::mat4 NeptuneModelMatrix = glm::rotate(glm::mat4(0.45f), (glm::mediump_float)around_rotation_angle, myRotationAxis) * glm::translate(glm::vec3(1.5f, 0, 54.5f)) * glm::scale(glm::vec3(1.6f)) * glm::rotate(glm::mat4(1.0f), (glm::mediump_float)y_rotation_angle, myRotationAxis);//glm::rotate(glm::mat4(1.0f), (glm::mediump_float)rotation_angle, myRotationAxis);
	drawObjectTexture(&sphereModel, NeptuneModelMatrix, tex_id);
	glutSwapBuffers();
}

void init()
{
	y_rotation_angle = 0;
	around_rotation_angle = 0;
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	tex_id = Core::LoadTexture("textures/grid.png");
	earth = Core::LoadTexture("textures/earth.png");
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
