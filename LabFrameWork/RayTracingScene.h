#pragma once

#pragma warning(push)
#pragma warning(disable:4311)		// convert void* to long
#pragma warning(disable:4312)		// convert long to void*

#include <iostream>
#include <string>
#include <vector>

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include "Viewer.h"
#include "ModelView.h"
#include "Loader.h"

#pragma warning(pop)

struct Object {
	float type;
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec4 color;
	glm::vec3 vert1;
	glm::vec3 vert2;
	glm::vec3 vert3;
	float radius;
	float diffuse;
	float specular;
	float shininess;
	float reflect;
};

struct Light {
	glm::vec3 pos;
	glm::vec4 color;
};

class RayTracingScene {
public:
	RayTracingScene(int w, int h);
	~RayTracingScene();
	void draw();
	void setSize(int w, int h) { m_width = w; m_height = h; }
	void setAspect(float r) { m_viewer->setAspectRatio(r); }
	Viewer* m_viewer;
	float m_rotate;

private:
	int m_width;
	int m_height;

	glm::vec3 eye, look, up;
	glm::mat4 model, view, projection, mview, imvp;
	glm::mat3 nmat;

	GLuint VAO, vert, texCoord, texture;

	void initialize();
	void setupRayTracing();
	void initShader();
	void initTexture();
	
	Model m_model;
	Object* objects;
	Light* lights;
	unsigned int objectNum;
	unsigned int lightNum;

	ShaderProgram* m_RayTracingShader;
	ShaderProgram* m_RayTracingComputeShader;
};
