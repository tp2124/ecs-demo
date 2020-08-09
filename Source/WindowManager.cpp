#include "WindowManager.hpp"

#include "Core/Coordinator.hpp"
#include <bitset>
#include <iostream>
#include <X11/XKBlib.h>


extern Coordinator gCoordinator;


// TODO: Return error to caller
void WindowManager::Init(
	std::string const& windowTitle, unsigned int windowWidth, unsigned int windowHeight, unsigned int windowPositionX,
	unsigned int windowPositionY)
{
	// Open the mDisplay
	{
		mDisplay = XOpenDisplay(nullptr);

		if (!mDisplay)
		{
			std::cerr << "Error: XOpenDisplay()\n";
			return;
		}
	}

	// Get the proper frame buffer config
	GLXFBConfig frameBufferConfig{};
	{
		// Frame buffer configs were added in GLX version 1.3.
		if (int glxMajor, glxMinor;
			!glXQueryVersion(mDisplay, &glxMajor, &glxMinor)
			|| ((glxMajor == 1) && (glxMinor < 3)) || (glxMajor < 1))
		{
			std::cerr << "Invalid GLX version.\n";
			return;
		}

		static int visualAttributes[] =
			{
				GLX_X_RENDERABLE, True,
				GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
				GLX_RENDER_TYPE, GLX_RGBA_BIT,
				GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
				GLX_RED_SIZE, 8,
				GLX_GREEN_SIZE, 8,
				GLX_BLUE_SIZE, 8,
				GLX_ALPHA_SIZE, 8,
				GLX_DEPTH_SIZE, 24,
				GLX_STENCIL_SIZE, 8,
				GLX_DOUBLEBUFFER, True,
				None
			};


		int frameBufferCount;
		GLXFBConfig* frameBufferConfigList = glXChooseFBConfig(
			mDisplay, DefaultScreen(mDisplay), visualAttributes, &frameBufferCount);

		if (!frameBufferConfigList)
		{
			std::cerr << "Error: glXChooseFBConfig()\n";
			return;
		}

		frameBufferConfig = frameBufferConfigList[0];

		XFree(frameBufferConfigList);
	}


	// Create the window
	{
		XVisualInfo* visualInfo = glXGetVisualFromFBConfig(mDisplay, frameBufferConfig);

		XSetWindowAttributes swa;
		swa.colormap = XCreateColormap(
			mDisplay,
			RootWindow(mDisplay, visualInfo->screen),
			visualInfo->visual, AllocNone);

		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask; // NOLINT(hicpp-signed-bitwise)

		mWindow = XCreateWindow(
			mDisplay, RootWindow(mDisplay, visualInfo->screen),
			windowPositionX, windowPositionY,
			windowWidth, windowHeight,
			0, visualInfo->depth, InputOutput,
			visualInfo->visual,
			CWColormap | CWEventMask, &swa); // NOLINT (hicpp-signed-bitwise)

		if (!mWindow)
		{
			std::cerr << "Error: XCreateWindow()\n";
			return;
		}

		XFree(visualInfo);
		XStoreName(mDisplay, mWindow, windowTitle.c_str());
		XMapWindow(mDisplay, mWindow);
	}


	// Create the context
	{
		typedef GLXContext (* glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

		auto glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress(
			(const GLubyte*)"glXCreateContextAttribsARB");

		if (!glXCreateContextAttribsARB)
		{
			std::cerr << "glXCreateContextAttribsARB() not found\n";
			return;
		}

		int contextAttributes[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
				GLX_CONTEXT_MINOR_VERSION_ARB, 0,
				None
			};

		mGlxContext = glXCreateContextAttribsARB(
			mDisplay, frameBufferConfig, nullptr,
			True, contextAttributes);

		if (!mGlxContext)
		{
			std::cerr << "Failed to create an OpenGL context\n";
			return;
		}
	}

	// Generate KeyRelease event only when physical key is actually released
	{
		bool detectableSet = XkbSetDetectableAutoRepeat(mDisplay, true, nullptr);

		if (!detectableSet)
		{
			std::cerr << "Detectable auto repeat not set - holding a key down will cause event spamming and delays.\n";
		}
	}

	glXMakeCurrent(mDisplay, mWindow, mGlxContext);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glXSwapBuffers(mDisplay, mWindow);
	glEnable(GL_DEPTH_TEST);
}


void WindowManager::Update()
{
	glXSwapBuffers(mDisplay, mWindow);
}


void WindowManager::Shutdown()
{
	glXMakeCurrent(mDisplay, 0, nullptr);
	glXDestroyContext(mDisplay, mGlxContext);
	XDestroyWindow(mDisplay, mWindow);
	XCloseDisplay(mDisplay);
}

void WindowManager::ProcessEvents()
{
	XEvent xEvent;

	if (XCheckWindowEvent(mDisplay, mWindow, ExposureMask | KeyPressMask | KeyReleaseMask, &xEvent)) // NOLINT (hicpp-signed-bitwise)
	{
		if (xEvent.type == Expose)
		{
			XWindowAttributes gwa;
			XGetWindowAttributes(mDisplay, mWindow, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);

			Event event(Events::Window::RESIZED);
			event.SetParam<unsigned int>(Events::Window::Resized::WIDTH, gwa.width);
			event.SetParam<unsigned int>(Events::Window::Resized::HEIGHT, gwa.height);

			gCoordinator.SendEvent(event);
		}
		else if (xEvent.type == KeyPress)
		{
			KeySym key = XkbKeycodeToKeysym(
				mDisplay,
				static_cast<KeyCode>(xEvent.xkey.keycode),
				0, 0);

			bool buttonStateChanged = true;

			if (key == XK_Escape)
			{
				gCoordinator.SendEvent(Events::Window::QUIT);
			}

			else if (key == XK_w)
			{
				mButtons.set(static_cast<std::size_t>(InputButtons::W));
			}

			else if (key == XK_a)
			{
				mButtons.set(static_cast<std::size_t>(InputButtons::A));
			}

			else if (key == XK_s)
			{
				mButtons.set(static_cast<std::size_t>(InputButtons::S));
			}

			else if (key == XK_d)
			{
				mButtons.set(static_cast<std::size_t>(InputButtons::D));
			}

			else if (key == XK_q)
			{
				mButtons.set(static_cast<std::size_t>(InputButtons::Q));
			}

			else if (key == XK_e)
			{
				mButtons.set(static_cast<std::size_t>(InputButtons::E));
			}
			else
			{
				buttonStateChanged = false;
				XFlush(mDisplay);
			}

			if (buttonStateChanged)
			{
				Event event(Events::Window::INPUT);
				event.SetParam(Events::Window::Input::INPUT, mButtons);
				gCoordinator.SendEvent(event);
			}
		}
		else if (xEvent.type == KeyRelease)
		{
			KeySym key = XkbKeycodeToKeysym(
				mDisplay,
				static_cast<KeyCode>(xEvent.xkey.keycode),
				0, 0);

			bool buttonStateChanged = true;

			if (key == XK_Escape)
			{
				gCoordinator.SendEvent(Events::Window::QUIT);
			}

			else if (key == XK_w)
			{
				mButtons.reset(static_cast<std::size_t>(InputButtons::W));
			}

			else if (key == XK_a)
			{
				mButtons.reset(static_cast<std::size_t>(InputButtons::A));
			}

			else if (key == XK_s)
			{
				mButtons.reset(static_cast<std::size_t>(InputButtons::S));
			}

			else if (key == XK_d)
			{
				mButtons.reset(static_cast<std::size_t>(InputButtons::D));
			}
			else if (key == XK_q)
			{
				mButtons.reset(static_cast<std::size_t>(InputButtons::Q));
			}

			else if (key == XK_e)
			{
				mButtons.reset(static_cast<std::size_t>(InputButtons::E));
			}
			else
			{
				buttonStateChanged = false;
				XFlush(mDisplay);
			}

			if (buttonStateChanged)
			{
				Event event(Events::Window::INPUT);
				event.SetParam(Events::Window::Input::INPUT, mButtons);
				gCoordinator.SendEvent(event);
			}
		}
		else
		{
			XFlush(mDisplay);
		}
	}
}
