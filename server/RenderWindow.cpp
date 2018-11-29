#include "stdafx.h"
#include "RenderWindow.h"

void RendererWindow::WindowCloseCallback(GLFWwindow * window)
{
	auto userPtr = (RendererWindow *)glfwGetWindowUserPointer(window);
	userPtr->CloseAndExit(0);
}

void RendererWindow::KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	//LOG(INFO) << "Key: " << key << ". Scancode: " << scancode << ". Action: " << action << ". Mods: " << mods;
}

void RendererWindow::CloseAndExit(int code)
{
	exit = true;
	code = code;
}

int RendererWindow::Render()
{
	while (!exit && !glfwWindowShouldClose(window)) {
		int paramFlip = 1;
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}
	return code;
}

