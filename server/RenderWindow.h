#pragma once
#include <stdexcept>
#include <GLFW\glfw3.h>

class RendererWindow
{
private:
	int width, height;
	const char * title;
	GLFWwindow * window;
	bool exit = false;
	int code;

	static void WindowCloseCallback(GLFWwindow * window);
	static void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
	void CloseAndExit(int code);
public:
	RendererWindow(int width, int height, const char * title) :
		width(width), height(height), title(title)
	{
		//	Creating a window
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (window == nullptr) {
			throw std::runtime_error("Could not create window");
		}

		//	Setting up context
		glfwMakeContextCurrent(window);

		//	Set RendererWindow as user pointer of window, so we can retrieve it in callbacks
		glfwSetWindowUserPointer(window, this);

		//	Setting up callbacks
		glfwSetWindowCloseCallback(window, &WindowCloseCallback);
		glfwSetKeyCallback(window, &KeyCallback);

		//	Disable Vsync
		glfwSwapInterval(0);
	};

	~RendererWindow()
	{
		glfwDestroyWindow(window);
	};

	int Render();
};

