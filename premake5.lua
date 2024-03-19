-- premake5.lua
workspace "WaterRendering"
	language "C++"
	cppdialect "C++20"
	
	platforms { "x64" }
	configurations { "Debug", "Release" }

	flags { "MultiProcessorCompile" }

	startproject "WaterRendering-main"

	objdir "objs"
	targetsuffix "-%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"

	filter "toolset:gcc"
		buildoptions { "-Wall" }

	filter "toolset:msc-*"
		warnings "extra"
		buildoptions { "/utf-8" }
		defines { "_CRT_SECURE_NO_WARNINGS=1" }

	filter "*"

	-- Default libraries
	filter "system:linux"
		links "dl"

	filter "system:windows"
		links "OpenGL32"

	filter "*"

	-- Default outputs
	filter "kind:StaticLib"
		targetdir "lib/"

	filter "kind:ConsoleApp"
		targetdir "bin/"
		targetextension ".exe"

	filter "*"

	-- Configuration filters
	filter "configurations:Debug"
		symbols "On"
		defines { "DEBUG=1" }

	filter "configurations:Release"
		defines { "NDEBUG=1" }
		optimize "On"

	filter "*"

-- Libraries
include "libs"

-- Projects
project "WaterRendering-main"
	local sources = {
		"main/**.cpp",
		"main/**.hpp",
		"main/**.h",
		"main/common/**.cpp",
		"main/common/**.hpp",
		"main/common/**.h"
	}

	kind "ConsoleApp"
	location "main"

	files(sources)

	links "x-stb"
	links "x-glad"
	links "x-glfw"
	links "x-glm"
	links "x-imgui"

	files(sources)

project "WaterRendering-shaders"
	local shaders = {
		"shaders/**.vert",
		"shaders/**.frag",
		"shaders/**.comp"
	}

	kind "Utility"
	location "shaders"

	files(shaders)