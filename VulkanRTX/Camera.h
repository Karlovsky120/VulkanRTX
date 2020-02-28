#pragma once

#include "glm/gtc/quaternion.hpp"

class Camera {
public:
	void translate(glm::vec3 offset);
	void rotate(glm::vec3 rotation);

	glm::mat4 getCameraMatrix();

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		   glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f),
		   float fov = 90,
		   float aspect = 1.777f, //16:9
		   float zNear = 0.1f,
		   float zFar = 10.0f);

private:
	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::mat4 m_projection;

	//const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

};

