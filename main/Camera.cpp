#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 position, glm::vec3 frontDirection, glm::vec3 upVector, float moveSpeed, float mouseSensitivity) :
	_position(position), 
	_frontDirection(frontDirection), 
	_upVector(upVector), 
	_moveSpeed(moveSpeed), 
	_mouseSensitivity(mouseSensitivity),
	_isEnabled(false) {


}

bool Camera::isEnabled() {
	return _isEnabled;
}

bool Camera::isFirstMouse() {
	return _firstMouse;
}

void Camera::setEnabled(bool enable) {
	_isEnabled = enable;
}

void Camera::setFirstMouse(bool enable) {
	_firstMouse = enable;
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(_position, _position + _frontDirection, _upVector);
}