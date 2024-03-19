-- Includes to add so Visual Studio can add them to Additional Include Directories
-- and then we can add them in source files
includedirs { "glad/include", "glfw/include", "glm", "imgui", "stb/include" }

-- GLAD
project "x-glad"
	kind "StaticLib"
	location "."

	includedirs { "glad/include" }

	filter "system:linux"
		defines { "_GLAD_X11=1" }

	filter "system:windows"
		defines { "_GLAD_WIN32=1" }

	files {
		"glad/src/*.c"
	}

-- GLFW
project "x-glfw"
	kind "StaticLib"
	location "."

	filter "system:linux"
		defines { "_GLFW_X11=1" }

	filter "system:windows"
		defines { "_GLFW_WIN32=1" }

	filter "toolset:msc-*"
		buildoptions {
			"/wd4100",
			"/wd4201",
			"/wd4204",
			"/wd4244",
			"/wd4706"
		}

	files {
		"glfw/src/context.c",
		"glfw/src/egl_context.c",
		"glfw/src/init.c",
		"glfw/src/input.c",
		"glfw/src/internal.h",
		"glfw/src/mappings.h",
		"glfw/src/monitor.c",
		"glfw/src/null_init.c",
		"glfw/src/null_joystick.c",
		"glfw/src/null_joystick.h",
		"glfw/src/null_monitor.c",
		"glfw/src/null_platform.h",
		"glfw/src/null_window.c",
		"glfw/src/platform.c",
		"glfw/src/platform.h",
		"glfw/src/vulkan.c",
		"glfw/src/window.c",
		"glfw/src/osmesa_context.c"
	}

	filter "system:linux"
		files {
			"glfw/src/posix_*",
			"glfw/src/x11_*", 
			"glfw/src/xkb_*",
			"glfw/src/glx_*",
			"glfw/src/linux_*"
		}

	filter "system:windows"
		files {
			"glfw/src/win32_*",
			"glfw/src/wgl_*"
		}

-- GLM
project "x-glm"
	kind "StaticLib"
	location "."

	filter "system:linux"
		defines { "_GLM_X11=1" }

	filter "system:windows"
		defines { "_GLM_WIN32=1" }

	files {
		"glm/glm/**"
	}

-- ImGui
project "x-imgui"
	kind "StaticLib"
	location "."

	filter "system:linux"
		defines { "_IMGUI_X11=1" }

	filter "system:windows"
		defines { "_IMGUI_WIN32=1" }

	defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD" }

	files {
		"imgui/*.cpp",
		"imgui/backends/imgui_impl_glfw.cpp",
		"imgui/backends/imgui_impl_opengl3.cpp"
	}

-- stb
project "x-stb"
	kind "StaticLib"
	location "."

	filter "toolset:msc-*"
		buildoptions { "/wd4204" }
	filter "*"

	files {
		"stb/src/*.c"
	}