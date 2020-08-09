#pragma once

#include <bitset>
#include <GL/glx.h>
#include <string>
#include <X11/Xlib.h>


class WindowManager
{
public:
	void Init(
		std::string const& windowTitle, unsigned int windowWidth, unsigned int windowHeight,
		unsigned int windowPositionX, unsigned int windowPositionY);

	void Update();

	void ProcessEvents();

	void Shutdown();

private:
	Display* mDisplay;
	Window mWindow;
	GLXContext mGlxContext;

	std::bitset<8> mButtons;
};
