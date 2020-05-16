#include "Camera.h"

#include <algorithm>

void Camera::translate(glm::vec3 offset) {
	m_position -= offset;
}

void Camera::translateOriented(glm::vec3 offset) {
	m_position += offset.z * m_forwardVector + offset.y * glm::vec3(0.0f, -1.0f, 0.0f) + offset.x * m_rightVector;
}

void Camera::moveForwards(float amount) {
	m_position += amount * m_forwardVector;
}

void Camera::moveBackwards(float amount) {
	m_position -= amount * m_forwardVector;
}

void Camera::strafeLeft(float amount) {
	m_position -= amount * m_rightVector;
}

void Camera::strafeRight(float amount) {
	m_position += amount * m_rightVector;
}

void Camera::rotate(glm::vec3 rotation) {
	m_rotation = glm::normalize(m_rotation * glm::quat(rotation));
}

void Camera::rotate(float pitch, float yaw) {

	if (pitch != 0) {
		if (pitch > 0) {
			pitch = std::min(pitch, acosf(glm::dot(m_forwardVector, glm::vec3(0.0f, -1.0f, 0.0f))) - 0.001f);
		} else if (pitch < 0) {
			pitch = -std::min(-pitch, acosf(glm::dot(m_forwardVector, glm::vec3(0.0f, 1.0f, 0.0f))) - 0.001f);
		}
	}

	glm::quat pitchQuat(glm::vec3(pitch, 0.0f, 0.0f));
	glm::quat yawQuat(glm::vec3(0.0f, yaw, 0.0f));

	m_rotation = pitchQuat * m_rotation * yawQuat;

	m_forwardVector = glm::normalize(glm::inverse(m_rotation) * glm::vec3(0.0f, 0.0f, -1.0f));
	m_rightVector = glm::normalize(glm::cross(m_forwardVector, glm::vec3(0.0f, -1.0f, 0.0f)));
}

void Camera::updateProjectionMatrixAspectRatio(float aspect) {
	m_aspect = aspect;
#ifdef IN_HOUSE_PROJECTION
	m_projection = glm::scale(calculateProjectionMatrix(glm::radians(m_fov), m_aspect, m_zNear, m_zFar), glm::vec3(1.0f, -1.0f, -1.0f));
#else
	m_projection = glm::scale(glm::perspective(glm::radians(m_fov), m_aspect, m_zNear, m_zFar), glm::vec3(1.0f, -1.0f, -1.0f));
#endif
}

void Camera::updateProjectionMatrixFov(float fov) {
	m_fov = fov;
	m_projection = glm::scale(glm::perspective(glm::radians(m_fov), m_aspect, m_zNear, m_zFar), glm::vec3(1.0f, -1.0f, -1.0f));
}

void Camera::adjustFov(float step) {
	updateProjectionMatrixFov(m_fov + step);
}

glm::mat4 Camera::getCameraMatrix() {
	return m_projection * getViewMatrix();
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4 identity(1.0f);

	glm::mat4 rotate = glm::mat4_cast(m_rotation);
	glm::mat4 translate = glm::translate(identity, m_position);

	return rotate * translate;
}

glm::mat4 Camera::getProjectionMatrix() {
	return m_projection;
}

glm::vec3 Camera::getCameraPosition() {
	return -m_position;
}

Camera::Camera(glm::vec3 position,
			   glm::vec3 rotation,
			   float fov,
			   float aspect,
			   float zNear,
			   float zFar) :
	m_position(position),
	m_rotation(rotation),
	m_fov(fov),
	m_aspect(aspect),
	m_zNear(zNear),
	m_zFar(zFar),
	m_projection(
		glm::scale(
			glm::perspective(
				glm::radians(m_fov),
				m_aspect,
				m_zNear,
				m_zFar),
		glm::vec3(1.0f, -1.0f, -1.0f))) {}