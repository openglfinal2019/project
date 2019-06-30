#pragma once
#include <GLEW/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>       
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
using namespace std;

// ��ʾһ��Բ������
struct Circle
{
	float x;
	float y;
	float r;
};

//����Բ���ϵ������㣬����Բ�ĺͰ뾶
Circle Calculate_circle(glm::vec2 px1, glm::vec2 px2, glm::vec2 px3) {
	float x1, y1, x2, y2, x3, y3;
	float a, b, c, g, e, f;
	x1 = px1.x;
	y1 = px1.y;
	x2 = px2.x;
	y2 = px2.y;
	x3 = px3.x;
	y3 = px3.y;
	e = 2 * (x2 - x1);
	f = 2 * (y2 - y1);
	g = x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1;
	a = 2 * (x3 - x2);
	b = 2 * (y3 - y2);
	c = x3 * x3 - x2 * x2 + y3 * y3 - y2 * y2;
	float X = (g*b - c * f) / (e*b - a * f);
    float Y = (a*g - c * e) / (a*f - b * e);
	float R = sqrt((X - x1)*(X - x1) + (Y - y1)*(Y - y1));
	Circle cir;
	cir.x = X;
	cir.y = Y;
	cir.r = R;
	return cir;
}

//�ж�����Բ�Ƿ�������ײ
bool if_collision(Circle c1, Circle c2) {
	float distance = sqrt(pow((c1.x - c2.x), 2) + pow((c1.y-c2.y),2));
	float rsum = c1.r + c2.r;
	return distance <= rsum;
}
 
//�ж�Բ�Ƿ��һ������ײ��
bool if_collision(Circle c1, GLfloat line) {
	return (fabs(c1.x - line) <= c1.r || fabs(c1.y - line) <= c1.r);
}