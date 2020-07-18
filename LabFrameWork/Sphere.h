#pragma once
#ifndef __SPHERE
#define __SPHERE

#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include "Loader.h"
#include <vector>


class Sphere
{
public:
	Sphere();
	void setup();
	void draw();

	int elementSize;

	GLuint vaoHandle;
	GLuint vbo_cube_vertices, vbo_cube_colors, ibo_cube_elements;
	ShaderProgram* shaderProgram;
};

#endif