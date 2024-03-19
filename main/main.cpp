// STD libraries
#include <stdexcept>
#include <format>

// 3rd Party Libraries
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Local classes / files
#include "GlobalState.h" // Includes OceanMesh.h, Waves.h, glm.hpp, glad.h
#include <GLFW/glfw3.h>
#include "ShaderManager.h"
#include "Skybox.h" // Inclues stb_image.h, error.h
#include "debug_output.h"
#include "checkpoint.h"

bool wireframe = false;
const int gridSizes[8] = {16, 32, 64, 128, 256, 512, 1024, 2048};
const char* gridSizesLabels[] = {"16", "32", "64", "128", "256", "512", "1024", "2048"};
int gridSize = 4;

namespace {
	float lightColor[3] = { 0.132f, 0.335f, 0.995f };
	float lightPos[3] = {256.f, 50.f, 256.f};

	void renderScene(GlobalState, float, float, Waves, Skybox);
	void processKeys(GLFWwindow*);

	void onWindowSizeChange(GLFWwindow* window, int width, int height);
	void onMouseButtonClick(GLFWwindow* window, int key, int action, int mods);
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

	GlobalState globalState{};
	glfwSetWindowUserPointer(_window, &globalState);

	globalState.camera = Camera(glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	globalState.camera._pitch = -90.0f;
	globalState.camera._yaw = 0.0f;

	// Set context
	glfwMakeContextCurrent(_window);

	// Set callbacks
	glfwSetWindowSizeCallback(_window, &onWindowSizeChange);
	glfwSetMouseButtonCallback(_window, &onMouseButtonClick);
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
	onWindowSizeChange(_window, width, height);

	// setup debug output here
#if defined (DEBUG)
	setup_gl_debug_output();
#endif

	// Enable some OpenGL flags
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

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

	// Initialise ocean waves generation
	Waves wave = initialise(gridSizes[gridSize]);
	OceanMesh::initialiseMesh(wave);
	OceanMesh::createVAO();
	globalState.meshVAO = OceanMesh::getMeshVAO();
	globalState.mesh = OceanMesh::getMesh();

	OGL_CHECKPOINT_ALWAYS();

	// Setup viewport
	int fbwidth, fbheight;
	glfwGetFramebufferSize(_window, &fbwidth, &fbheight);
	glViewport(0, 0, fbwidth, fbheight);

	// Timings for time delta and FPS
	auto last = std::chrono::steady_clock::now();
	float totalTime = 0.f;

	double prevTime = glfwGetTime();
	int frames = 0;
	int fps = 0;

	// Initialise shaders
	ShaderManager::initialiseShaders();

	// Run loop
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();

		// ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug");
		ImGui::ColorEdit3("Light color", lightColor);
		ImGui::DragFloat3("Light Pos", lightPos);

		ImGui::Combo("Grid Size", &gridSize, gridSizesLabels, 8);
		ImGui::SliderInt("Length Scale", &wave._lengthScale, 0, 512);
		ImGui::SliderFloat("Depth", &wave._depth, 0.0f, 1000.0f);
		ImGui::SliderFloat("Wind Speed", &wave._windSpeed, 0.0f, 30.0f);
		ImGui::SliderFloat("Gravity", &wave._gravity, 0.0f, 20.0f);
		ImGui::SliderFloat("Fetch", &wave._fetch, 0.0f, 1000000.0f);

		if (ImGui::Button("Recalculate Initials")) {
			wave._peakOmega = wave.jonswapPeakFrequency(wave._gravity, wave._fetch, wave._windSpeed);
			wave._alpha = wave.jonswapAlpha(wave._gravity, wave._fetch, wave._windSpeed);
			wave.calculateWaveSpectra();
			wave.calculateConjugateSpectra();
		}

		if (ImGui::Button("Recalculate Grid (Resets values)")) {
			wave = initialise(gridSizes[gridSize]);
			OceanMesh::initialiseMesh(wave);
			OceanMesh::createVAO();
			globalState.meshVAO = OceanMesh::getMeshVAO();
			globalState.mesh = OceanMesh::getMesh();
		}

		if (ImGui::Button("Reset Values")) {
			lightColor[0] = 0.132f;
			lightColor[1] = 0.335f;
			lightColor[2] = 0.995f;
			lightPos[0] = 256.0f;
			lightPos[1] = 50.0f;
			lightPos[2] = 256.0f;

			wave._lengthScale = 256;
			wave._depth = 500.0f;
			wave._windSpeed = 7.29f;
			wave._gravity = 9.81f;
			wave._fetch = 100000.0f;
		}

		ImGui::Checkbox("Wireframe", &wireframe);

		ImGui::End();

		ImGui::Begin("Stats");
		ImGui::Text("FPS: %d", fps);
		ImGui::Text("Vertices: %d", globalState.mesh.positions.size());
		ImGui::Text("Triangles: %d", globalState.mesh.positions.size() / 3);
		ImGui::Text("Camera Position: %f %f %f", globalState.camera._position.x, globalState.camera._position.y, globalState.camera._position.z);
		ImGui::Text("Peak Omega: %f", wave._peakOmega);
		ImGui::End();

		ImGui::ShowDemoWindow();

		// Update waves
		wave.calculateWavesAtTime(totalTime);

		// Timings and FPS
		auto const now = std::chrono::steady_clock::now();
		float timeDelta = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1>>>(now - last).count();
		totalTime += timeDelta;
		globalState.timeDelta = timeDelta;
		
		double currentTime = glfwGetTime();
		frames++;
		if (currentTime - prevTime >= 1.0) {
			fps = frames;
			frames = 0;
			prevTime = currentTime;
		}

		last = now;

		// Check for resizing
		int fbwidth, fbheight;
		glfwGetFramebufferSize(_window, &fbwidth, &fbheight);
		glViewport(0, 0, fbwidth, fbheight);

		// Render Scene
		renderScene(globalState, fbwidth, fbheight, wave, skybox);

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

namespace {
	void renderScene(GlobalState globalState, float width, float height, Waves wave, Skybox skybox) {
		// Matrices
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = globalState.camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(100.0f), width / height, 0.1f, 1000.f);

		glm::mat4 mvpMatrix = projection * view * model;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Water mesh
		ShaderManager::enableShader("Default");
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wave._displacementTexture);

		glUniform1i(1, wave.getSize());
		// Sets uniform displacement sampler to texture 0
		glUniform1i(2, 0);

		glUniform3fv(3, 1, &globalState.camera._position.x);
		glUniform3fv(4, 1, lightPos);
		glUniform3fv(5, 1, lightColor);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wave._derivativesTexture);

		glUniform1i(6, 1);

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
					float distance = globalState->camera._moveSpeed * globalState->timeDelta;

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

	void onWindowSizeChange(GLFWwindow* window, int width, int height) {

	}

	void onMouseButtonClick(GLFWwindow* window, int key, int action, int mods) {
		
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