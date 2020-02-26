#include "Camera.h"

void Camera::translate(glm::vec3 offset) {
	m_position += offset;
}

void Camera::rotate(glm::vec3 rotation) {
	m_rotation *= glm::quat(rotation);
}

glm::mat4 Camera::getCameraMatrix() {
	return m_projection * glm::lookAt(m_position, glm::vec3(0.0f, 0.0f, 1.0f) * m_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Camera::getViewMatrix() {
	return glm::lookAt(m_position, glm::vec3(0.0f, 0.0f, 0.0f) * m_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Camera::getProjectionMatrix() {
	return m_projection;
}

Camera::Camera(glm::vec3 position,
			   glm::vec3 rotation,
			   float fov,
			   float aspect,
			   float zNear,
			   float zFar) :
	m_position(position),
	m_rotation(rotation) {

	m_projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	//Flip for Vulkan
	m_projection[1][1] *= -1;
}