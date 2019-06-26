// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GLEW/glew.h>
// 引入GLFW库
#include <GLFW/glfw3.h>
// 引入SOIL库
#include <SOIL/SOIL.h>
// 引入GLM库
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cstdlib>

// 包含着色器加载库
#include "ResourceManager.h"
// 包含相机控制辅助类
#include "camera.h"
// 包含纹理加载类
#include "texture.h"
// 加载模型的类
#include "model.h"
//光源顶点
#include "vertics.h"
//运动控制类
#include "movementController.h"
// 键盘回调函数原型声明
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
// 鼠标移动回调函数原型声明
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
// 鼠标滚轮回调函数原型声明
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void renderObject(const unsigned int VAO, const unsigned int VBO, bool first);
// 场景中移动
void do_movement();

void collision(glm::vec4 wall);

// 定义程序常量
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
// 用于相机交互参数
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouseMove = true;
bool keyPressedStatus[1024]; // 按键情况记录
GLfloat deltaTime = 0.0f; // 当前帧和上一帧的时间差
GLfloat lastFrame = 0.0f; // 上一帧时间

GLfloat angle = 0.0, fontangle = 0.0, rightangle = 0.0;
glm::vec3 coord = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 offset = glm::vec3(740.072*0.001, 306.926*0.001, 107.156*0.001);

Camera camera(glm::vec3(0.0f, 1.0f, 0.0f));
moveController Move(0.02, glm::vec3(1.0, 0.0, 0.0));

enum ViewPort viewfrom = FREE;
glm::vec3 lightpos = glm::vec3(-2.0f, 8.0f, 0.0f);
Model objModel;

int main(int argc, char** argv)
{

	if (!glfwInit())	// 初始化glfw库
	{
		std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;
		return -1;
	}

	// 开启OpenGL 3.3 core profile
	std::cout << "Start OpenGL core profile version 3.3" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// 创建窗口
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
		"Demo of loading model with AssImp with light on", NULL, NULL);
	if (!window)
	{
		std::cout << "Error::GLFW could not create winddow!" << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	// 创建的窗口的context指定为当前context
	glfwMakeContextCurrent(window);

	// 注册窗口键盘事件回调函数
	glfwSetKeyCallback(window, key_callback);
	// 注册鼠标事件回调函数
	glfwSetCursorPosCallback(window, mouse_move_callback);
	// 注册鼠标滚轮事件回调函数
	glfwSetScrollCallback(window, mouse_scroll_callback);
	// 鼠标捕获 停留在程序内
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 初始化GLEW 获取OpenGL函数
	glewExperimental = GL_TRUE; // 让glew获取所有拓展函数
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cout << "Error::GLEW glew version:" << glewGetString(GLEW_VERSION)
			<< " error string:" << glewGetErrorString(status) << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	// 设置视口参数
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	//Section1 加载模型数据
	if (!objModel.loadModel("../../../resources/models/baolingqiu/666666.obj"))
	{
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Section2 准备着色器程序
	ResourceManager::LoadShader("model.vertex", "model.frag", nullptr, "objModel",true);
	ResourceManager::LoadShader("model.vertex", "model.frag", nullptr, "ballModel",true);
	ResourceManager::LoadShader("model.vertex", "model.frag", "model.geometry", "pinModel", true);
	ResourceManager::LoadShader("light.vertex", "light.frag", nullptr, "light", true);
	ResourceManager::LoadShader("shadow_getDepth_VS.txt", "shadow_getDepth_FS.txt", nullptr,"shadow" ,true);
	ResourceManager::GetShader("objModel").Use();
	glUniform1i(glGetUniformLocation(ResourceManager::GetShader("objModel").ID, "shadowMap"), 0);
	renderObject(VAO, VBO, true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);


	/*------------------------------阴影---------------------------------*/
	//创建帧缓冲对象
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//创建对应的纹理对对象作为帧缓冲的附件
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	//设置纹理的相关参数：纹理映射、环绕方式、数据等
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//将纹理对象添加到帧缓冲中
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	/*-----------------------------------End-----------------------------------------*/
	
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//cout << currentFrame << endl;
		glfwPollEvents(); // 处理例如鼠标 键盘等事件
		do_movement(); // 根据用户操作情况 更新相机属性

					   // 清除颜色缓冲区 重置为指定颜色
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 设置观察者位置
		if (viewfrom == FREE) {
			ResourceManager::GetShader("objModel").SetVector3f("viewPos", camera.position.x, camera.position.y, camera.position.z, true);
			ResourceManager::GetShader("ballModel").SetVector3f("viewPos", camera.position.x, camera.position.y, camera.position.z, true);
			ResourceManager::GetShader("pinModel").SetVector3f("viewPos", camera.position.x, camera.position.y, camera.position.z, true);
		}
		else if (viewfrom == BOWLING) {
			ResourceManager::GetShader("objModel").SetVector3f("viewPos", camera.bowlingposition.x, camera.bowlingposition.y, camera.bowlingposition.z, true);
			ResourceManager::GetShader("ballModel").SetVector3f("viewPos", camera.bowlingposition.x, camera.bowlingposition.y, camera.bowlingposition.z, true);
			ResourceManager::GetShader("pinModel").SetVector3f("viewPos", camera.bowlingposition.x, camera.bowlingposition.y, camera.bowlingposition.z, true);
		}
		//设置渲染时的投影矩阵
		glm::mat4 projection;
		if(viewfrom==FREE) projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.001f, 1000.0f); // 投影矩阵
		else if(viewfrom==BOWLING) projection = glm::perspective(30.0f,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.001f, 100.0f);
		ResourceManager::GetShader("objModel").SetMatrix4("projection", projection, true);
		ResourceManager::GetShader("ballModel").SetMatrix4("projection", projection, true);
		ResourceManager::GetShader("pinModel").SetMatrix4("projection", projection, true);
		//设置渲染时的观察矩阵
		glm::mat4 view = viewfrom==FREE?camera.getViewMatrix():camera.getViewMatrixB(); // 视变换矩阵
		ResourceManager::GetShader("objModel").SetMatrix4("view", view, true);
		ResourceManager::GetShader("ballModel").SetMatrix4("view", view, true);
		ResourceManager::GetShader("pinModel").SetMatrix4("view", view, true);
		//设置渲染时的模型矩阵
		//1.
		glm::mat4 objmodel=glm::mat4(1.0f),ballmodel=glm::mat4(1.0f);
		objmodel = glm::translate(objmodel, glm::vec3(0.0f, 0.0f, 0.0f)); // 适当下调位置
		objmodel = glm::scale(objmodel, glm::vec3(0.001f, 0.001f, 0.001f)); // 适当缩小模型
		ResourceManager::GetShader("objModel").SetMatrix4("model", objmodel, true);
		ResourceManager::GetShader("pinModel").SetMatrix4("model", objmodel, true);
		//2.
		ballmodel = glm::translate(ballmodel, offset);
		ballmodel = glm::rotate(ballmodel, glm::radians(angle), coord);
		ballmodel = glm::translate(ballmodel, glm::vec3(-740.072*0.001, -306.926*0.001,-107.156*0.001));
		ballmodel= glm::scale(ballmodel, glm::vec3(0.001f, 0.001f, 0.001f));
		ResourceManager::GetShader("ballModel").SetMatrix4("model", ballmodel, true);

		//进行渲染得到光的视角下的阴影贴图
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
		lightView = glm::lookAt(lightpos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		ResourceManager::GetShader("shadow").SetMatrix4("lightSpaceMatrix", lightSpaceMatrix,true);
		ResourceManager::GetShader("shadow").SetMatrix4("model", objmodel,true);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		objModel.draw(ResourceManager::GetShader("shadow"), ResourceManager::GetShader("ballModel"), ResourceManager::GetShader("pinModel"),true,false);
		
		ResourceManager::GetShader("shadow").SetMatrix4("model", ballmodel, true);
		objModel.draw(ResourceManager::GetShader("shadow"), ResourceManager::GetShader("ballModel"), ResourceManager::GetShader("pinModel"),true,true);
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//设置保龄球视角下的摄像机属性
		if (Move.move) {
			if (Move.speech > 0) {
				Move.updateSpeech(deltaTime);
				offset += Move.changeValue;
				objModel.move_ball(Move.changeValue*1000.0f);
				angle += Move.dirction.x > 0 ? 10*(Move.speech/0.03): -10 * (Move.speech / 0.03);
				coord = Move.coord;
				collision(Move.wall);
			}
		}
		camera.bowlingposition = offset+glm::vec3(-0.1,0.3,0.0);
		camera.bowlingup = glm::vec3(0.0f, -1.0f, 0.0f);
		camera.bowlingforward = glm::vec3(1.0, -1.0, 0.0);
		// 这里填写场景绘制代码
		renderObject(VAO, VBO, false);//渲染光源正方体

		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		ResourceManager::GetShader("objModel").SetMatrix4("lightSpaceMatrix", lightSpaceMatrix, true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		objModel.draw(ResourceManager::GetShader("objModel"),ResourceManager::GetShader("ballModel"), ResourceManager::GetShader("pinModel"),false,false); // 绘制物体
		
		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); // 交换缓存
	}
	// 释放资源
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keyPressedStatus[key] = true;
		else if (action == GLFW_RELEASE)
			keyPressedStatus[key] = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // 关闭窗口
	}
}
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (viewfrom==FREE) {
		if (firstMouseMove) // 首次鼠标移动
		{
			lastX = xpos;
			lastY = ypos;
			firstMouseMove = false;
		}

		GLfloat xoffset = xpos - lastX;
		GLfloat yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera.handleMouseMove(xoffset, yoffset);
	}
}
// 由相机辅助类处理鼠标滚轮控制
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(viewfrom == FREE) camera.handleMouseScroll(yoffset);
}
// 由相机辅助类处理键盘控制
void do_movement()
{
	if (keyPressedStatus[GLFW_KEY_P]) {
		viewfrom = BOWLING;
	}
	if (keyPressedStatus[GLFW_KEY_L]) {
		viewfrom = FREE;
	}
	if (keyPressedStatus[GLFW_KEY_O]) {
		viewfrom = HUMAN;
	}
	if (keyPressedStatus[GLFW_KEY_W] && viewfrom == FREE)
		camera.handleKeyPress(FORWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_S] && viewfrom == FREE)
		camera.handleKeyPress(BACKWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_A] && viewfrom == FREE)
		camera.handleKeyPress(LEFT, deltaTime);
	if (keyPressedStatus[GLFW_KEY_D] && viewfrom == FREE)
		camera.handleKeyPress(RIGHT, deltaTime);
	//控制小球的移动
	if (keyPressedStatus[GLFW_KEY_J])
	{
		/*float dis=objModel.move_ball(0);
		offset -= glm::vec3(dis*0.001, 0.0f, 0.0f);
		rightangle += dis == 0.0 ? 0 : 20*fabs(dis/30.0);
		angle = rightangle;
		coord = glm::vec3(0.0, 0.0, 1.0);
		cout << "qx: " << offset.x << endl;
		//camera.bowlingforward = glm::vec3(-1.0, -1.0, 0.0);*/
		Move.move = true;
	}
	if (keyPressedStatus[GLFW_KEY_U])
	{
		/*float dis=objModel.move_ball(1);
		offset += glm::vec3(0.001*dis, 0.0f, 0.0f);
		rightangle -= dis == 0.0 ? 0 : 20* fabs(dis / 30.0);
		angle = rightangle;
		coord = glm::vec3(0.0, 0.0, 1.0);*/
		
		if(Move.move) Move.addSpeech();

	}
	if (keyPressedStatus[GLFW_KEY_H])
	{
		/*float dis=objModel.move_ball(2);
		offset -= glm::vec3(0.0f, 0.0f, 0.001f*dis);
		fontangle += dis == 0.0 ? 0 : 20* fabs(dis / 30.0);
		angle = fontangle;
		coord = glm::vec3(1.0, 0.0, 0.0);*/
		
		if (!Move.move) Move.updateDirction(0);
		
	}
	if (keyPressedStatus[GLFW_KEY_K])
	{
		/*float dis=objModel.move_ball(3);
		offset += glm::vec3(0.0f, 0.0f, 0.001*dis);
		fontangle -= dis==0.0?0:20* fabs(dis / 30.0);
		angle = fontangle;
		coord = glm::vec3(1.0, 0.0, 0.0);
		cout << "qz: " << offset.z << endl;*/
		
		if(!Move.move) Move.updateDirction(1);
	}
}

void renderObject(const unsigned int VAO, const unsigned int VBO, bool first) {
	if (first) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vvertices), &vvertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	}
	else {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, lightpos);
		model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
		glm::mat4  projection = glm::perspective(camera.mouse_zoom,(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.001f, 1000.0f);
		glm::mat4 view = viewfrom == FREE ? camera.getViewMatrix() : camera.getViewMatrixB();
		ResourceManager::GetShader("light").SetMatrix4("model", model, true);
		ResourceManager::GetShader("light").SetMatrix4("view", view, true);
		ResourceManager::GetShader("light").SetMatrix4("projection", projection, true);
		glBindVertexArray(VAO);
		ResourceManager::GetShader("light").Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void collision(glm::vec4 wall) {
	if (offset.x >= wall.x || offset.x<= wall.y) Move.updateDirctionZ();
	if (offset.z >= wall.z || offset.z <= wall.w) Move.updateDirctionX();
}