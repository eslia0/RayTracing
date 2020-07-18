#pragma once
#include <GLFW/glfw3.h>
#include "RayTracingScene.h"

double cx, cy;
bool lbutton_down;
bool rbutton_down;
bool mbutton_down;
float m_lastMouseX;
float m_lastMouseY;
int m_width, m_height;

class Callback
{
public:
	RayTracingScene* scene;

	void InitializeCallbacks(int width, int height)
	{
		m_width = width;
		m_height = height;
	}

	static void window_size_callback(GLFWwindow* window, int width, int height)
	{
		glfwGetWindowSize(window, &width, &height);
		m_width = width;
		m_height = height;
	}

	static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
	{
		cx = xpos;
		cy = ypos;
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if (GLFW_PRESS == action)
			{
				rbutton_down = true;
			}
			else if (GLFW_RELEASE == action) {
				rbutton_down = false;
			}
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			if (GLFW_PRESS == action) {
				mbutton_down = true;
			}
			else if (GLFW_RELEASE == action) {
				mbutton_down = false;
			}
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (GLFW_PRESS == action) {
				lbutton_down = true;
			}
			else if (GLFW_RELEASE == action) {
				lbutton_down = false;
			}
		}
	}

	// 마우스 드래깅
	void mouseDragging(double width, double height)
	{
		if (lbutton_down)
		{
			float fractionChangeX = static_cast<float>(cx - m_lastMouseX) / static_cast<float>(width);
			float fractionChangeY = static_cast<float>(m_lastMouseY - cy) / static_cast<float>(height);
			scene->m_viewer->rotate(fractionChangeX, fractionChangeY);
		}
		else if (mbutton_down)
		{
			float fractionChangeX = static_cast<float>(cx - m_lastMouseX) / static_cast<float>(width);
			float fractionChangeY = static_cast<float>(m_lastMouseY - cy) / static_cast<float>(height);
			scene->m_viewer->zoom(fractionChangeY);
		}
		else if (rbutton_down)
		{
			float fractionChangeX = static_cast<float>(cx - m_lastMouseX) / static_cast<float>(width);
			float fractionChangeY = static_cast<float>(m_lastMouseY - cy) / static_cast<float>(height);
			scene->m_viewer->translate(-fractionChangeX, -fractionChangeY, 1);
		}

		m_lastMouseX = (float)cx;
		m_lastMouseY = (float)cy;
	}

	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error %d: %s\n", error, description);
	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
};