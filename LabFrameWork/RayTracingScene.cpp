#include <glm/gtc/type_ptr.hpp>

#include "RayTracingScene.h"

RayTracingScene::RayTracingScene(int w, int h)
{
	m_width = w;
	m_height = h;

	glm::vec3 viewPoint(5, 5, 0);
	glm::vec3 viewCenter(0, 0, 0);
	glm::vec3 upVector(0, 1, 0);

	float aspect = (w / (float)h);
	m_viewer = new Viewer(viewPoint, viewCenter, upVector, 45.0f, aspect);

	glClearDepth(1);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_FRONT_AND_BACK);

	glActiveTexture(GL_TEXTURE0);

	initialize();
	initShader();
	initTexture();
	setupRayTracing();
}

void RayTracingScene::initialize()
{
	objectNum = 7;
	objects = new Object[objectNum];

	objects[0].type = 1; // plane
	objects[0].pos = glm::vec3(0, -3, 0);
	objects[0].rot = glm::vec3(0, 0, 0);
	objects[0].color = glm::vec4(1, 1, 1, 1);
	objects[0].diffuse = 1;
	objects[0].specular = 1;
	objects[0].shininess = 30;
	objects[0].reflect = 0.05;

	objects[1].type = 0; // Sphere
	objects[1].pos = glm::vec3(0, 0, 0);
	objects[1].rot = glm::vec3(0, 0, 0);
	objects[1].color = glm::vec4(1, 0.2, 0.2, 1);
	objects[1].radius = 2;
	objects[1].diffuse = 1;
	objects[1].specular = 3;
	objects[1].shininess = 50;
	objects[1].reflect = 0.1;

	objects[2].type = 0; // Sphere
	objects[2].pos = glm::vec3(7, 0, 0);
	objects[2].rot = glm::vec3(0, 0, 0);
	objects[2].color = glm::vec4(0.2, 0.2, 1, 1);
	objects[2].radius = 2;
	objects[2].diffuse = 1;
	objects[2].specular = 3;
	objects[2].shininess = 50;
	objects[2].reflect = 0.1;

	objects[3].type = 0; // Sphere
	objects[3].pos = glm::vec3(4, 0, 6);
	objects[3].rot = glm::vec3(0, 0, 0);
	objects[3].color = glm::vec4(0.2, 1, 0.2, 1);
	objects[3].radius = 3;
	objects[3].diffuse = 1;
	objects[3].specular = 2;
	objects[3].shininess = 20;
	objects[3].reflect = 0.1;

	objects[4].type = 0; // Sphere
	objects[4].pos = glm::vec3(-4, 0, -5);
	objects[4].rot = glm::vec3(0, 0, 0);
	objects[4].color = glm::vec4(0.2, 1, 1, 1);
	objects[4].radius = 2;
	objects[4].diffuse = 1;
	objects[4].specular = 2;
	objects[4].shininess = 20;
	objects[4].reflect = 0.1;

	objects[5].type = 2; // Triangle
	objects[5].pos = glm::vec3(5, -2, 2);
	objects[5].rot = glm::vec3(0, 0, 0);
	objects[5].color = glm::vec4(1, 1, 0.2, 1);
	objects[5].vert1 = glm::vec3(0, 0, 0);
	objects[5].vert2 = glm::vec3(0, 4, 0);
	objects[5].vert3 = glm::vec3(4, 0, 0);
	objects[5].diffuse = 1;
	objects[5].specular = 2;
	objects[5].shininess = 20;
	objects[5].reflect = 0.1;

	objects[6].type = 2; // Triangle
	objects[6].pos = glm::vec3(5, -2, 2);
	objects[6].rot = glm::vec3(0, 0, 0);
	objects[6].color = glm::vec4(1, 1, 0.2, 1);
	objects[6].vert1 = glm::vec3(4, 4, 0);
	objects[6].vert2 = glm::vec3(4, 0, 0);
	objects[6].vert3 = glm::vec3(0, 4, 0);
	objects[6].diffuse = 1;
	objects[6].specular = 2;
	objects[6].shininess = 20;
	objects[6].reflect = 0.1;

	lightNum = 2;
	lights = new Light[lightNum];

	lights[0].pos = glm::vec3(10, 10, 0);
	lights[0].color = glm::vec4(1, 1, 1, 1);

	lights[1].pos = glm::vec3(-10, 10, 10);
	lights[1].color = glm::vec4(0, 0.5, 1, 1);
}

void RayTracingScene::initShader()
{
	GLfloat vertices[] = {
		-1.f, -1.f,
		-1.f, 1.f,
		1.f, -1.f,
		-1.f, 1.f,
		1.f, -1.f,
		1.f, 1.f,
	};

	GLfloat textCoord[] = {
		0.f, 1.f,
		0.f, 0.f,
		1.f, 1.f,
		0.f, 0.f,
		1.f, 1.f,
		1.f, 0.f
	};

	m_RayTracingShader = new ShaderProgram();
	m_RayTracingShader->initFromFiles("shaders/RayTracing.vert", "shaders/RayTracing.frag");
	m_RayTracingShader->addAttribute("aPosition");
	m_RayTracingShader->addAttribute("aTexCoord");

	// Bind vertices
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &vert);
	glBindBuffer(GL_ARRAY_BUFFER, vert);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &texCoord);
	glBindBuffer(GL_ARRAY_BUFFER, texCoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textCoord), textCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void RayTracingScene::initTexture()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void RayTracingScene::setupRayTracing()
{
	m_RayTracingComputeShader = new ShaderProgram();
	m_RayTracingComputeShader->initFromFiles("shaders/RayTracing.comp");

	m_RayTracingComputeShader->addUniform("uSize");
	m_RayTracingComputeShader->addUniform("uCamera.pos");
	m_RayTracingComputeShader->addUniform("uCamera.rot");
	m_RayTracingComputeShader->addUniform("uCamera.fov");
	m_RayTracingComputeShader->addUniform("uCamera.reflectDepth");

	m_RayTracingComputeShader->addUniform("uObjectNum");

	for (int i = 0; i < objectNum; i++) {
		std::ostringstream os;
		os << "uObjects[" << i << "]";
		m_RayTracingComputeShader->addUniform(os.str().append(".type").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".pos").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".rot").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".color").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".vert1").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".vert2").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".vert3").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".radius").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".diffuse").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".specular").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".shininess").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".reflect").c_str());
	}

	m_RayTracingComputeShader->addUniform("uLightNum");

	for (int i = 0; i < lightNum; i++) {
		std::ostringstream os;
		os << "uLights[" << i << "]";
		m_RayTracingComputeShader->addUniform(os.str().append(".pos").c_str());
		m_RayTracingComputeShader->addUniform(os.str().append(".color").c_str());
	}
}

void RayTracingScene::draw ()
{
	glViewport(0, 0, m_width, m_height);

	m_RayTracingComputeShader->use();

	glDispatchCompute(m_width, m_height, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUniform2fv(m_RayTracingComputeShader->uniform("uSize"), 1, glm::value_ptr(glm::vec2(m_width, m_height)));
	glUniform3fv(m_RayTracingComputeShader->uniform("uCamera.pos"), 1, glm::value_ptr(m_viewer->getViewPoint()));
	glUniform3fv(m_RayTracingComputeShader->uniform("uCamera.rot"), 1, glm::value_ptr(m_viewer->getViewDir()));
	glUniform1f(m_RayTracingComputeShader->uniform("uCamera.fov"), m_viewer->getFieldOfView());
	glUniform1f(m_RayTracingComputeShader->uniform("uCamera.reflectDepth"), 10);

	glUniform1i(m_RayTracingComputeShader->uniform("uObjectNum"), objectNum);
	glUniform1i(m_RayTracingComputeShader->uniform("uLightNum"), lightNum);

	for (int i = 0; i < objectNum; i++) {
		std::ostringstream os;
		os << "uObjects[" << i << "]";
		glUniform1f(m_RayTracingComputeShader->uniform(os.str().append(".type")), objects[i].type);
		glUniform3fv(m_RayTracingComputeShader->uniform(os.str().append(".pos")), 1, glm::value_ptr(objects[i].pos));
		glUniform3fv(m_RayTracingComputeShader->uniform(os.str().append(".rot")), 1, glm::value_ptr(objects[i].rot));
		glUniform4fv(m_RayTracingComputeShader->uniform(os.str().append(".color")), 1, glm::value_ptr(objects[i].color));
		glUniform3fv(m_RayTracingComputeShader->uniform(os.str().append(".vert1")), 1, glm::value_ptr(objects[i].vert1));
		glUniform3fv(m_RayTracingComputeShader->uniform(os.str().append(".vert2")), 1, glm::value_ptr(objects[i].vert2));
		glUniform3fv(m_RayTracingComputeShader->uniform(os.str().append(".vert3")), 1, glm::value_ptr(objects[i].vert3));
		glUniform1f(m_RayTracingComputeShader->uniform(os.str().append(".radius")), objects[i].radius);
		glUniform1f(m_RayTracingComputeShader->uniform(os.str().append(".diffuse")), objects[i].diffuse);
		glUniform1f(m_RayTracingComputeShader->uniform(os.str().append(".specular")), objects[i].specular);
		glUniform1f(m_RayTracingComputeShader->uniform(os.str().append(".shininess")), objects[i].shininess);
		glUniform1f(m_RayTracingComputeShader->uniform(os.str().append(".reflect")), objects[i].reflect);
	}

	for (int i = 0; i < lightNum; i++) {
		std::ostringstream os;
		os << "uLights[" << i << "]";
		glUniform3fv(m_RayTracingComputeShader->uniform(os.str().append(".pos")), 1, glm::value_ptr(lights[i].pos));
		glUniform4fv(m_RayTracingComputeShader->uniform(os.str().append(".color")), 1, glm::value_ptr(lights[i].color));
	}
	m_RayTracingComputeShader->disable();

	m_RayTracingShader->use();

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	m_RayTracingShader->disable();
}

RayTracingScene::~RayTracingScene()
{
	delete[] m_RayTracingShader;
	delete[] m_RayTracingComputeShader;
}