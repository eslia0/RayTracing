#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Callback.h"

class Source : public Callback
{
public:
	GLFWwindow* window;

	bool Init(int width, int height)
	{
		// Create a windowed mode window and its OpenGL context
		window = glfwCreateWindow(width, height, "OpenGL FrameWork", NULL, NULL);
		if (!window)
		{
			return false;
		}

		glfwMakeContextCurrent(window);
		glfwSetWindowTitle(window, "RayTracing");
		glfwSwapInterval(1);

		glfwSetErrorCallback(error_callback);
		glfwSetKeyCallback(window, key_callback);
		glfwSetWindowSizeCallback(window, window_size_callback);
		glfwSetCursorPosCallback(window, cursor_pos_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);

		return true;
	}

	void Render()
	{
		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene->draw();

		glfwSwapBuffers(window);
		glfwPollEvents();

		mouseDragging(display_w, display_h);
	}
};

int main(void)
{
	if (!glfwInit())
	{
		std::cout << "GLFW Initialization has failed" << std::endl;
		return -1;
	}

	// Initialize the library
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
	int width = 800;
	int height = 800;

	Source* source = new Source();
	if (!source->Init(width, height)) {
		glfwTerminate();
		std::cout << "GLFW window create failed" << std::endl;
		return 0;
	}

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "glewInit failed" << std::endl;
		return 0;
	}
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	source->scene = new RayTracingScene(width, height);

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(source->window))
	{
		source->Render();
	}

	glfwDestroyWindow(source->window);

	glfwTerminate();
	return 0;
}