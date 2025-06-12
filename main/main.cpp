// STD libraries
#include <stdexcept>
#include <format>
#include <numeric>
#include <iostream>
#include <chrono>

// 3rd Party Libraries
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Local classes / files
#include "Globals.h" // Includes OceanMesh.h, Waves.h, glm.hpp, glad.h
#include <GLFW/glfw3.h>
#include "shaders/ShaderManager.h"
#include "utils/Skybox.h" // Inclues stb_image.h, error.h
#include "utils/debug_output.h"

// Some global variables
const float PI = 3.14159274f;

bool wireframe = false;
bool vsync = false;
const int gridSizes[8] = {16, 32, 64, 128, 256, 512, 1024, 2048};
const char* gridSizesLabels[] = {"16", "32", "64", "128", "256", "512", "1024", "2048"};
int gridSize = 4;
struct WaveData waveData;

namespace {
	std::vector<double> frameTimes;
	double frameTimeAvg = 0.0;
	int fpsAvg = 0;

	float lightColorPBR[3] = { 300.0f, 300.0f, 300.0f };
	float lightPosPBR[3] = { 512.0f, 300.0f, 512.0f };

	float albedo[3] = { 0.016f, 0.118f, 0.745f };
	float metallic = 0.0f;
	float roughness = 0.1f;
	float ao = 0.25f;

	float foamStrength = 1.9f;

	bool recalculate = false;

	void renderScene(GlobalState, float, float, std::array<Waves, 3>, Skybox);
	void processKeys(GLFWwindow*);

	void onCursorPosChange(GLFWwindow* window, double x, double y);
	void onKeyPressed(GLFWwindow* window, int key, int, int action, int mods);

	void programShutdown();
}

/////////////////
// Main method //
/////////////////
int main() try {

	// Try initialise GLFW
	if (glfwInit() != GLFW_TRUE) {
		// GLFW failed to initialise
		char const* errorMessage = nullptr;
		int errorCode = glfwGetError(&errorMessage);
		throw Error("glfwInit() failed to initialise GLFW. Error: %s (%d)", errorMessage, errorCode);
	}

	// Set GLFW Settings
	// Request OpenGL version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	glfwWindowHint(GLFW_DEPTH_BITS, 24);

#if defined(DEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	// Get primary monitor and video mode to automatically set size
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

	// Try to create GLFW window
	GLFWwindow* _window = glfwCreateWindow(videoMode->width, videoMode->height, "Water Rendering", false ? primaryMonitor : nullptr, nullptr);

	if (_window == NULL) {
		// Window creation failed
		char const* errorMessage = nullptr;
		int errorCode = glfwGetError(&errorMessage);
		throw Error("glfwCreateWindow() failed to initialise a window. Error: %s (%d)", errorMessage, errorCode);
	}

	// A struct to hold some values needed in GLFW callback functions
	GlobalState globalState{};
	glfwSetWindowUserPointer(_window, &globalState);

	globalState.camera = Camera(glm::vec3(50.0f, 6.0f, 50.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	globalState.camera._pitch = -90.0f;
	globalState.camera._yaw = 0.0f;

	// Set context
	glfwMakeContextCurrent(_window);

	// Set callbacks
	glfwSetCursorPosCallback(_window, &onCursorPosChange);
	glfwSetKeyCallback(_window, &onKeyPressed);

	// Initialise GLAD
	if (!gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress))
		throw Error("gladLoadGLLoader() failed - could not load OpenGL API");

	// Some debug info
	std::printf("OPENGL_RENDERER: %s\n", glGetString(GL_RENDERER));
	std::printf("OPENGL_VENDOR: %s\n", glGetString(GL_VENDOR));
	std::printf("OPENGL_VERSION: %s\n", glGetString(GL_VERSION));
	std::printf("SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// Setup platform/renderer backends
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();

	// Maximise window (not fullscreen)
	glfwMaximizeWindow(_window);
	int width, height;
	glfwGetWindowSize(_window, &width, &height);

	glfwSwapInterval(vsync);

	// setup debug output here
#if defined (DEBUG)
	setup_gl_debug_output();
#endif

	// Enable some OpenGL flags
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// Initialise shaders
	ShaderManager::initialiseShaders();

	// Create skybox
	std::vector<std::string> faces = {
		"../assets/skybox/right.bmp",
		"../assets/skybox/left.bmp",
		"../assets/skybox/top.bmp",
		"../assets/skybox/bottom.bmp",
		"../assets/skybox/front.bmp",
		"../assets/skybox/back.bmp"
	};
	Skybox skybox = Skybox(faces);

	// Initialise ocean waves generation (using 3 iterations of different scale)
	std::array<Waves, 3> waves = initialise(waveData, gridSizes[gridSize]);

	OceanMesh::initialiseMesh(gridSizes[gridSize]);
	OceanMesh::createVAO();

	globalState.meshVAO = OceanMesh::getMeshVAO();
	globalState.mesh = OceanMesh::getMesh();

	// Setup viewport
	int fbwidth, fbheight;
	glfwGetFramebufferSize(_window, &fbwidth, &fbheight);
	glViewport(0, 0, fbwidth, fbheight);

	// Timings for time delta and FPS
	auto last = std::chrono::steady_clock::now();
	float totalTime = 0.f;
	double prevTime = glfwGetTime();
	double lastSecondTime = 0.0;
	double frameTime = 0.0;
	int frames = 0;
	int fps = 0;

	// Run loop
	while (!glfwWindowShouldClose(_window)) {
		glfwSwapInterval(vsync);
		glfwPollEvents();

		// ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		/**/
		ImGui::Begin("Debug");
		ImGui::Combo("Grid Size", &gridSize, gridSizesLabels, 8);
		ImGui::SliderInt("Scale 1", &waveData.scale1, 1, 300);
		ImGui::SliderInt("Scale 2", &waveData.scale2, 1, 50);
		ImGui::SliderInt("Scale 3", &waveData.scale3, 1, 50);
		ImGui::SliderFloat("Depth", &waveData.depth, 0.1f, 1000.0f);
		ImGui::SliderFloat("Wind Speed", &waveData.windSpeed, 0.1f, 30.0f);
		ImGui::SliderFloat("Gravity", &waveData.gravity, 0.1f, 20.0f);
		ImGui::SliderFloat("Fetch", &waveData.fetch, 0.1f, 1000000.0f);
		ImGui::SliderFloat("Wave Direction", &waveData.angle, 0.0f, 360.0f);
		ImGui::Checkbox("Recalculate Parameters", &recalculate);

		ImGui::Text("Water Material Properties - PBR");
		ImGui::DragFloat3("Light Position (PBR)", lightPosPBR);
		ImGui::ColorEdit3("Albedo", albedo);
		ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
		ImGui::SliderFloat("Ambient Occlusion", &ao, 0.0f, 1.0f);
		ImGui::SliderFloat("Foam Strength", &foamStrength, 1.0f, 4.0f);

		if (ImGui::Button("Reset Values")) {
			waveData.scale1 = 250;
			waveData.scale2 = 19;
			waveData.scale3 = 4;
			waveData.depth = 500.0f;
			waveData.windSpeed = 7.29f;
			waveData.gravity = 9.81f;
			waveData.fetch = 100000.0f;
			waveData.angle = 29.81f;

			lightPosPBR[0] = 512.0f;
			lightPosPBR[1] = 300.0f;
			lightPosPBR[2] = 512.0f;
			albedo[0] = 0.016f;
			albedo[1] = 0.118;
			albedo[2] = 0.745f;
			metallic = 0.0f;
			roughness = 0.1f;
			ao = 0.25f;

			foamStrength = 1.9f;
		}

		ImGui::Checkbox("Wireframe", &wireframe);
		ImGui::SameLine();
		ImGui::Checkbox("VSync", &vsync);

		ImGui::End();

		ImGui::Begin("Stats");
		ImGui::Text("Frame time: %.3fms (Avg: %.3fms)", frameTime * 1000, frameTimeAvg * 1000);
		ImGui::Text("FPS: %d (Avg: %d)", fps, fpsAvg);
		ImGui::Text("Vertices: %d", globalState.mesh.positions.size());
		ImGui::Text("Triangles: %d", globalState.mesh.positions.size() / 3);
		ImGui::Text("Camera Position: %f %f %f", globalState.camera._position.x, globalState.camera._position.y, globalState.camera._position.z);
		ImGui::End();

		if (recalculate) {
			float boundary1 = 2 * PI / waveData.scale2 * 6.f;
			float boundary2 = 2 * PI / waveData.scale3 * 6.f;

			waves[0].recalculateInitials(waveData, waveData.scale1, 0.0001f, boundary1);
			waves[1].recalculateInitials(waveData, waveData.scale2, boundary1, boundary2);
			waves[2].recalculateInitials(waveData, waveData.scale3, boundary2, 9999.9f);
		}

		// Update waves
		waves[0].calculateWavesAtTime(totalTime, globalState.timeDelta);
		waves[1].calculateWavesAtTime(totalTime, globalState.timeDelta);
		waves[2].calculateWavesAtTime(totalTime, globalState.timeDelta);

		// Timings and FPS
		auto const now = std::chrono::steady_clock::now();
		float timeDelta = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1>>>(now - last).count();
		totalTime += timeDelta;
		globalState.timeDelta = timeDelta;
		
		double currentTime = glfwGetTime();
		frameTime = currentTime - prevTime;
		fps = 1.0 / frameTime;
		frames++;
		frameTimes.push_back(frameTime);
		if (currentTime - lastSecondTime >= 1.0) {
			frameTimeAvg = std::reduce(frameTimes.begin(), frameTimes.end()) / frames;
			fpsAvg = 1 / frameTimeAvg;
			frameTimes.clear();
			frames = 0;
			lastSecondTime = currentTime;
		}
		prevTime = currentTime;

		last = now;

		// Check for resizing
		int fbwidth, fbheight;
		glfwGetFramebufferSize(_window, &fbwidth, &fbheight);
		glViewport(0, 0, fbwidth, fbheight);

		// Render Scene
		renderScene(globalState, fbwidth, fbheight, waves, skybox);

		// Render ImGui frame
		ImGui::Render();
		ImGui::EndFrame();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(_window);

		processKeys(_window);
	}

	programShutdown();

	return 0;
}
catch (std::exception const& error) {
	std::fprintf(stderr, "Thrown Exception (%s):\n", typeid(error).name());
	std::fprintf(stderr, "%s\n", error.what());
	return 1;
}

std::vector<GLuint64> timings;

namespace {
	void renderScene(GlobalState globalState, float width, float height, std::array<Waves, 3> waves, Skybox skybox) {
		// Matrices
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = globalState.camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(120.0f), width / height, 0.1f, 1000.f);

		glm::mat4 mvpMatrix = projection * view;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Water rendering and shading
		ShaderManager::enableShader("PBR");
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
		glUniform1i(1, gridSizes[gridSize]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waves[0]._displacementTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, waves[1]._displacementTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, waves[2]._displacementTexture);

		glUniform1i(2, waves[0]._scale);
		glUniform1i(3, waves[1]._scale);
		glUniform1i(4, waves[2]._scale);

		glUniform3fv(5, 1, &globalState.camera._position.x);
		glUniform3fv(6, 1, lightPosPBR);
		glUniform3fv(7, 1, lightColorPBR);

		glUniform3fv(8, 1, albedo);
		glUniform1f(9, metallic);
		glUniform1f(10, roughness);
		glUniform1f(11, ao);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, waves[0]._derivativesTexture);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, waves[1]._derivativesTexture);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, waves[2]._derivativesTexture);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, waves[0]._foamTexture);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, waves[1]._foamTexture);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, waves[2]._foamTexture);

		glUniform1f(12, foamStrength);

		// Pass wireframe state so we can color the wireframe in black if enabled
		glUniform1i(13, wireframe);

		glBindVertexArray(globalState.meshVAO);
		// Wireframe should only alter water mesh
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

		glDrawElements(GL_TRIANGLES, globalState.mesh.indices.size(), GL_UNSIGNED_INT, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, 0);
		ShaderManager::disableShader();

		// Skybox
		glDepthFunc(GL_LEQUAL);
		ShaderManager::enableShader("Skybox");

		glm::mat4 mvpMatrixMod = projection * glm::mat4(glm::mat3(view)) * model;
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixMod));
		glBindVertexArray(skybox._skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox._skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		ShaderManager::disableShader();
		glDepthFunc(GL_LESS);
	}

	void processKeys(GLFWwindow* window) {
		if (GlobalState* globalState = static_cast<GlobalState*>(glfwGetWindowUserPointer(window)))
		{
			for (auto const& [key, value] : globalState->keys)
			{
				if (value.first)
				{
					// Shift needs to be held down before pressing a movement key
					float speedMod = 1.0f;
					if (value.second == GLFW_MOD_SHIFT) speedMod = 3.0f;
					float distance = globalState->camera._moveSpeed * speedMod * globalState->timeDelta;

					switch (key)
					{
					case GLFW_KEY_W:
						globalState->camera._position += distance * globalState->camera._frontDirection;
						break;
					case GLFW_KEY_S:
						globalState->camera._position -= distance * globalState->camera._frontDirection;
						break;
					case GLFW_KEY_D:
						globalState->camera._position += glm::normalize(glm::cross(globalState->camera._frontDirection, globalState->camera._upVector)) * distance;
						break;
					case GLFW_KEY_A:
						globalState->camera._position -= glm::normalize(glm::cross(globalState->camera._frontDirection, globalState->camera._upVector)) * distance;
						break;
					}
				}
			}
		}
	}

	void onCursorPosChange(GLFWwindow* window, double x, double y) {
		if (GlobalState* globalState = static_cast<GlobalState*>(glfwGetWindowUserPointer(window))) {

			if (globalState->camera.isEnabled()) {

				if (globalState->camera.isFirstMouse()) {
					int winX, winY;
					glfwGetFramebufferSize(window, &winX, &winY);

					globalState->camera._lastX = (float)winX/2;
					globalState->camera._lastY = (float)winY/2;
					globalState->camera.setFirstMouse(false);
				}

				float xOffset = (float)x - globalState->camera._lastX;
				float yOffset = globalState->camera._lastY - (float)y;
				globalState->camera._lastX = (float)x;
				globalState->camera._lastY = (float)y;

				xOffset *= globalState->camera._mouseSensitivity;
				yOffset *= globalState->camera._mouseSensitivity;

				globalState->camera._yaw += xOffset;
				globalState->camera._pitch += yOffset;

				if (globalState->camera._pitch > 89.f)
					globalState->camera._pitch = 89.f;
				if (globalState->camera._pitch < -89.f)
					globalState->camera._pitch = -89.f;

				glm::vec3 newDirection;
				newDirection.x = std::cos(glm::radians(globalState->camera._yaw)) * std::cos(glm::radians(globalState->camera._pitch));
				newDirection.y = std::sin(glm::radians(globalState->camera._pitch));
				newDirection.z = std::sin(glm::radians(globalState->camera._yaw)) * std::cos(glm::radians(globalState->camera._pitch));
				globalState->camera._frontDirection = glm::normalize(newDirection);
			}
		}
	}

	void onKeyPressed(GLFWwindow* window, int key, int, int action, int mods) {
		if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			return;
		}

		if (GlobalState* globalState = static_cast<GlobalState*>(glfwGetWindowUserPointer(window))) {
			if (GLFW_KEY_SPACE == key && GLFW_PRESS == action) {

				globalState->camera.setEnabled(!globalState->camera.isEnabled());

				if (globalState->camera.isEnabled()) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}

			if (GLFW_PRESS == action) {
				globalState->keys[key] = std::pair(true, mods);
			}
			else if (GLFW_RELEASE == action) {
				globalState->keys[key] = std::pair(false, mods);
			}
		}
	}

	void programShutdown() {
		glfwTerminate();
		
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}