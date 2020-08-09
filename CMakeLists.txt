cmake_minimum_required(VERSION 3.14)
project(nexus)


find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(nexus)

target_compile_options(
	nexus
	PRIVATE
	-fno-exceptions
	-Wall)

target_sources(
	nexus
	PRIVATE
	Source/Graphics/GlLoader.cpp
	Source/Graphics/Shader.cpp
	Source/Main.cpp
	Source/Systems/CameraControlSystem.cpp
	Source/Systems/PhysicsSystem.cpp
	Source/Systems/PlayerControlSystem.cpp
	Source/Systems/RenderSystem.cpp
	Source/WindowManager.cpp)

target_sources(
	nexus
	PRIVATE
	Source/Components/Camera.hpp
	Source/Components/Gravity.hpp
	Source/Components/Player.hpp
	Source/Components/Renderable.hpp
	Source/Components/RigidBody.hpp
	Source/Components/Thrust.hpp
	Source/Components/Transform.hpp
	Source/Core/ComponentArray.hpp
	Source/Core/ComponentManager.hpp
	Source/Core/Coordinator.hpp
	Source/Core/EntityManager.hpp
	Source/Core/EventManager.hpp
	Source/Core/System.hpp
	Source/Core/SystemManager.hpp
	Source/Core/Types.hpp
	Source/Math/Vec2.hpp
	Source/Math/Vec3.hpp
	Source/Math/Vec4.hpp)

target_include_directories(
	nexus
	PRIVATE
	Source)

target_link_libraries(
	nexus
	PRIVATE
	OpenGL::GLX
	X11::X11)
