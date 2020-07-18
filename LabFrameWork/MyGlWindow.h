//#define  FREEGLUT_LIB_PRAGMAS  0

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

#include "cube.h"
#include "ModelManager.h"
#include "Viewer.h"
#include "ModelView.h"

#pragma warning(pop)

class MyGlWindow {
public:
	MyGlWindow(int w, int h);
	~MyGlWindow();
	void draw(glm::mat4 view, glm::mat4 projection);
	void setSize(int w, int h) { m_width = w; m_height = h; }
	void setAspect(float r) { m_viewer->setAspectRatio(r); }
	void setAnimationIndex(int index);
	void switchAnimating();
	Viewer *m_viewer;
	float m_rotate;
	float speed;

private:
	glm::vec3 eye, look, up;
	glm::mat4 model, view, projection, mview, imvp;
	glm::mat3 nmat;

	int m_width;
	int m_height;
	float startTime;

	void initialize();

	void setupShaders();
	void drawFloor(ShaderProgram * shader, glm::mat4 & view, glm::mat4 & projection);
	void drawCube(ShaderProgram * shader, glm::mat4 & view, glm::mat4 & projection);
	void drawAnimatedModel3D(AnimatedModelData* model3D, ShaderProgram * shader, glm::mat4 & view, glm::mat4 & projection);
	void setupFloor();
	void setupCube();
	void setupModel3D();

	colorCube * m_cube;
	checkeredFloor * m_floor;
	ModelManager * m_ModelManager;

	Model m_model;

	//shaders
	ShaderProgram * m_floorShader;
	ShaderProgram * m_cubeShader;
	ShaderProgram * m_model3DShader;
	ShaderProgram * m_model3DComputeShader;
};
