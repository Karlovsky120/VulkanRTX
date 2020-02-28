#include "Camera.h"

void Camera::translate(glm::vec3 offset) {
	m_position -= offset;
}

void Camera::rotate(glm::vec3 rotation) {
	m_rotation = m_rotation * glm::quat(rotation);
}

glm::mat4 Camera::getCameraMatrix() {
	return m_projection;
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4 identity(1.0f);

	glm::mat4 rotate = glm::mat4_cast(m_rotation);
	glm::mat4 translate = glm::translate(identity, m_position);

	return translate * rotate;
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

	// Perspective matrix flips the Z coordinate, so it needs to be flipped back.
	m_projection = glm::scale(glm::perspective(glm::radians(fov), aspect, zNear, zFar), glm::vec3(1.0f, 1.0f, -1.0f));
}