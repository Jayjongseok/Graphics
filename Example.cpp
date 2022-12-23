#include <gl/glew.h>
#include <GL/GL.h>
#include <GLFW/glfw3.h>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <vector>
#include "toys.h"
#include "j3a.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;

float cameraTheta = 0;
float cameraPhi = 0; // 라디안 
float scaleFactor = 1.0f;
bool isShrinking = true;
const float PI = 3.14159265358979f;
float cameraDistance = 5;
float fovy = 45 * PI / 180;

vec3 lightPosition = vec3(3, 10, 5);
vec3 lightColor = vec3(100); //밝기
vec3 ambientLight = vec3(0.0);

GLuint diffTex = 0;
GLuint bumpTex = 0;
GLuint VBO = 0; // gl unsigned int vertex buffer object
GLuint NBO = 0; // gl unsigned int normal buffer object  
GLuint textureCoordinateBuffer = 0;
GLuint VAO = 0; // gl unsigned int vertex ARRAY object
GLuint EBO = 0; // gl unsinged int elements buffer object
Program program;

void init();
void cursorPosCallback(GLFWwindow* win, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void render(GLFWwindow* window);
int main(void)
{
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Title", 0, 0);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwMakeContextCurrent(window);
	glewInit();
	init();

	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		render(window);
		glfwPollEvents();
	}

}
void render(GLFWwindow* window)
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0, 0, 0.5, 0);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 앞의 glclearcolor 색으로 버퍼가 초기화 된다.


	glUseProgram(program.programID); // gpu내 buffer id


	////theta += 5.0f*PI/180;
	//if (isShrinking) {
	//	scaleFactor -= 0.025;
	//	if (scaleFactor < 0.5) {
	//		isShrinking = false;
	//	}
	//}
	//else {
	//	scaleFactor += 0.025;
	//	if (scaleFactor > 1) {
	//		isShrinking = true;
	//	}
	//}

	vec3 initialCameraPosition = vec3(0, 0, cameraDistance);
	mat4 cameraRotationMatrix1 = glm::rotate(cameraPhi, vec3(1, 0, 0)); // 앵글 각도, 어느축 중심
	mat4 cameraRotationMatrix2 = glm::rotate(cameraTheta, vec3(0, 1, 0)); // 앵글 각도, 어느축 중심

	vec3 cameraPosition = cameraRotationMatrix2 * cameraRotationMatrix1 * vec4(initialCameraPosition, 1); // 카메라 위치

	mat4 viewMat = glm::lookAt(cameraPosition, vec3(0, 0, 0), vec3(0, 1, 0)); // 카메라 위치 ,center 볼 물체의 위치, upvector
	mat4 projMat = glm::perspective(fovy, w / (float)h, 0.01f, 1000.0f);
	//mat3 rotationMatrix = mat3(cos(theta), -sin(theta), 0, sin(theta), cos(theta), 0, 0, 0, 1);
	//mat4 rotationMatrix = rotate(theta, vec3(0, 1, 0));
	//mat3 scalingMatrix = mat3(scaleFactor, 0, 0, 0,scaleFactor, 0, 0, 0, 1);
	GLuint modelMatLocation = glGetUniformLocation(program.programID, "modelMat");
	glUniformMatrix4fv(modelMatLocation, 1, 0, value_ptr(mat4(1)));


	GLuint viewMatLocation = glGetUniformLocation(program.programID, "viewMat");
	glUniformMatrix4fv(viewMatLocation, 1, 0, value_ptr(viewMat));

	GLuint projMatLocation = glGetUniformLocation(program.programID, "projMat");
	glUniformMatrix4fv(projMatLocation, 1, 0, value_ptr(projMat));

	GLuint colorLocation = glGetUniformLocation(program.programID, "color");
	glUniform4fv(colorLocation, 1, value_ptr(diffuseColor[0]));

	GLuint shininessLocation = glGetUniformLocation(program.programID, "shininess");
	glUniform1f(shininessLocation, shininess[0]);

	GLuint lightPositionLocation = glGetUniformLocation(program.programID, "lightPosition");
	glUniform3fv(lightPositionLocation, 1, value_ptr(lightPosition));

	GLuint ambientLightLocation = glGetUniformLocation(program.programID, "ambientLight");
	glUniform3fv(ambientLightLocation, 1, value_ptr(ambientLight));

	GLuint cameraPositionLocation = glGetUniformLocation(program.programID, "cameraPosition");
	glUniform3fv(cameraPositionLocation, 1, value_ptr(cameraPosition));

	GLuint lightColorLocation = glGetUniformLocation(program.programID, "lightColor");
	glUniform3fv(lightColorLocation, 1, value_ptr(lightColor));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffTex);
	GLuint diffTexLocation = glGetUniformLocation(program.programID, "diffTex");
	glUniform1i(diffTexLocation, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bumpTex);
	GLuint bumpTexLocation = glGetUniformLocation(program.programID, "bumpTex");
	glUniform1i(bumpTexLocation, 1);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, nTriangles[0] * 3, GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(window);

}
void init()
{
	loadJ3A("trex_m.j3a");
	program.loadShaders("shader.vert", "shader.frag");

	glGenBuffers(1, &VBO);// vbo만들기 vertex 저장 객체
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // gl_array_Buffer에 바인딩 시키는 것 vertex들
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec3), vertices[0], GL_STATIC_DRAW);
	/* gl_array_buffer 전달 binding 되는 것들 알려주고
	buffer의 크기 알려주고, buffer에 저장할 데이터 넣어주고,
	buffer 이용목적 표시 - 버퍼 내용 한 번만 설정 */
	glGenBuffers(1, &NBO);// vbo만들기 vertex 저장 객체
	glBindBuffer(GL_ARRAY_BUFFER, NBO); // gl_array_Buffer에 바인딩 시키는 것 vertex들
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec3), normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &textureCoordinateBuffer);// vbo만들기 vertex 저장 객체
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateBuffer); // gl_array_Buffer에 바인딩 시키는 것 vertex들
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec2), texCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO); // vertex속성 및 위치 등 저장 array만 들고 오브젝트를 그림
	glBindVertexArray(VAO); // 이용 준비
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // gl_array_Buffer에 바인딩 시키는 것 vertex들
	glEnableVertexAttribArray(0); // 속성 이용할 수 있게 함
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0); // 속성들을 해석 위치속성

	glBindBuffer(GL_ARRAY_BUFFER, NBO); // gl_array_Buffer에 바인딩 시키는 것 vertex들
	glEnableVertexAttribArray(1); // 속성 이용할 수 있게 함
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0); // 속성들을 해석 위치속성

	glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateBuffer); // gl_array_Buffer에 바인딩 시키는 것 vertex들
	glEnableVertexAttribArray(2); // 속성 이용할 수 있게 함
	glVertexAttribPointer(2, 2, GL_FLOAT, 0, 0, 0); // 속성들을 해석 위치속성

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles[0] * sizeof(glm::u32vec3), triangles[0], GL_STATIC_DRAW);

	int w, h, n;
	stbi_set_flip_vertically_on_load(true);
	void* d = stbi_load(diffuseMap[0].c_str(), &w, &h, &n, 4);
	printf("image : %s %d x %d\n", diffuseMap[0].c_str(), w, h);
	glGenTextures(1, &diffTex);
	glBindTexture(GL_TEXTURE_2D, diffTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d);
	stbi_image_free(d);

	d = stbi_load(bumpMap[0].c_str(), &w, &h, &n, 4);
	printf("image : %s %d x %d\n", bumpMap[0].c_str(), w, h);
	glGenTextures(1, &bumpTex);
	glBindTexture(GL_TEXTURE_2D, bumpTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d);
	stbi_image_free(d);
}

void cursorPosCallback(GLFWwindow* win, double xpos, double ypos)
{
	static double lastX = 0;
	static double lastY = 0;
	if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1))
	{
		double dx = xpos - lastX;
		double dy = ypos - lastY;
		int w, h;
		glfwGetWindowSize(win, &w, &h);
		cameraTheta -= dx / (w * PI);
		cameraPhi -= dy / (h * PI);
		//printf("dx :%.3f dy :%.3f\n", cameraTheta, cameraPhi);
	}
	lastX = xpos;
	lastY = ypos;
}
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	//cameraDistance = cameraDistance * pow(1.1, yoffset); // yoffset 양수면 1보다 크게 음수면 1보다 작은 0.000... 
	fovy += yoffset / 30;
}