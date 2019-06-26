#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>       
#include <vector>

class moveController {
private:
	float speech_u;
public:
	glm::vec3 dirction;
	glm::vec3 coord;
	glm::vec3 changeValue;
	float speech;
	glm::vec4 wall;
	bool move;
	moveController(float spech_u,glm::vec3 dir) {
		speech_u = spech_u;
		dirction = glm::normalize(dir);
		coord.z = dirction.x;
		coord.x = -dirction.z;
		move = false;
		wall = glm::vec4(10.127, -9.07, 0.627, -0.4028);
	}
	void updateSpeech(float timechanged) {
		float speechnow = speech;
		speech -= speech_u * timechanged;
		if (speech < 0) speech = 0;
		changeValue = dirction * ((speech+speechnow)*timechanged/2);
	}
	void updateDirctionX() {
		dirction.z = -dirction.z;
		coord.z = dirction.x;
		coord.x = -dirction.z;
		coord = glm::normalize(coord);
	}
	void updateDirctionZ() {
		dirction.x = -dirction.x;
		coord.z = dirction.x;
		coord.x = -dirction.z;
		coord = glm::normalize(coord);
	}
	void updateDirction(int code) {
		dirction.z += code == 1 ? 0.01 : -0.01;
		dirction = glm::normalize(dirction);
	}
	void addSpeech() {
		speech += 0.03;
	}
};