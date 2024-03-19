#pragma once

#include <glm/glm.hpp>

class Camera {
public:
	Camera();
	Camera(
		glm::vec3 position, 
		glm::vec3 viewPoint, 
		glm::vec3 upVector, 
		float moveSpeed = 30.0f, 
		float mouseSensitivity = 0.1f);

	bool isEnabled();
	bool isFirstMouse();
	void setEnabled(bool enable);
	void setFirstMouse(bool enable);

	glm::mat4 getViewMatrix() const;

	glm::vec3 _position;
	glm::vec3 _frontDirection;
	glm::vec3 _upVector;

	float _moveSpeed;
	float _mouseSensitivity;

	float _lastX;
	float _lastY;
	float _yaw;
	float _pitch;

	bool _isEnabled = false;
	bool _firstMouse = true;
};
