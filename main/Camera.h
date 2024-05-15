#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	float _lastX = 0.0f;
	float _lastY = 0.0f;
	float _yaw = 0.0f;
	float _pitch = 0.0f;

	bool _isEnabled = false;
	bool _firstMouse = true;
};
