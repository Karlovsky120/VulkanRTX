#pragma once

#include "GlobalDefines.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

class Camera {
public:
	void translate(glm::vec3 offset);
	void translateOriented(glm::vec3 offset);

	void moveForwards(float amount);
	void moveBackwards(float amount);
	void strafeLeft(float amount);
	void strafeRight(float amount);

	void rotate(glm::vec3 rotation);
	void rotate(float pitch, float yaw);

	void updateProjectionMatrixAspectRatio(float aspect);
	void updateProjectionMatrixFov(float fov);
	void adjustFov(float step);

	glm::mat4 getCameraMatrix();

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	glm::vec3 getCameraPosition();

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		   glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f),
		   float fov = 90,
		   float aspect = 1.777f, //16:9
		   float zNear = 0.1f,
		   float zFar = 10.0f);

private:
	float m_fov;
	float m_aspect;
	float m_zNear;
	float m_zFar;

	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::mat4 m_projection;

	glm::vec3 m_forwardVector = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_rightVector = glm::vec3(-1.0f, 0.0f, 0.0f);

	glm::mat4 calculateProjectionMatrix(float fov, float aspect, float zNear, float zFar);
};

