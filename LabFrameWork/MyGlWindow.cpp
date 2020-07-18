#include "MyGlWindow.h"
#include "global.h"
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model3D.h"

static float DEFAULT_VIEW_POINT[3] = { 50, 50, 50 };
static float DEFAULT_VIEW_CENTER[3] = { 0, 0, 0 };
static float DEFAULT_UP_VECTOR[3] = { 0, 1, 0 };

MyGlWindow::MyGlWindow(int w, int h)
//==========================================================================
{
	startTime = (float)clock();
	m_width = w;
	m_height = h;
	m_cube = 0;

	glm::vec3 viewPoint(DEFAULT_VIEW_POINT[0], DEFAULT_VIEW_POINT[1], DEFAULT_VIEW_POINT[2]);
	glm::vec3 viewCenter(DEFAULT_VIEW_CENTER[0], DEFAULT_VIEW_CENTER[1], DEFAULT_VIEW_CENTER[2]);
	glm::vec3 upVector(DEFAULT_UP_VECTOR[0], DEFAULT_UP_VECTOR[1], DEFAULT_UP_VECTOR[2]);

	float aspect = (w / (float)h);
	m_viewer = new Viewer(viewPoint, viewCenter, upVector, 45.0f, aspect);
	m_rotate = 0;

	m_floorShader = 0;
	m_cubeShader = 0;
	m_model3DShader = 0;

	setupShaders();
	initialize();
}

void MyGlWindow::initialize()
{
	m_ModelManager = new ModelManager();
	m_floor = new checkeredFloor();

	// 모델 로드
	Model3D* model3D = new Model3D("models/EthanWalk.fbx");
	// 애니메이션 로드
	model3D->AddAnimationData("models/EthanWalk.fbx");

	// 인스턴싱 데이터 설정
	m_ModelManager->AddAnimatedModelData("matrix.txt", model3D, 1);
}

void MyGlWindow::setupShaders()
{
	setupFloor();
	setupModel3D();
}

void MyGlWindow::setupFloor()
{
	m_floorShader = new ShaderProgram();
	m_floorShader->initFromFiles("shaders/floor.vert", "shaders/floor.frag");
	m_floorShader->addAttribute("VertexPosition");
	m_floorShader->addAttribute("VertexColor");
	m_floorShader->addUniform("MVP");
}

void MyGlWindow::setupCube()
{
	m_cubeShader = new ShaderProgram();
	m_cubeShader->initFromFiles("shaders/simple.vert", "shaders/simple.frag");
	m_cubeShader->addAttribute("coord3d");
	m_cubeShader->addAttribute("v_color");
	m_cubeShader->addUniform("mvp");
}

void MyGlWindow::setupModel3D()
{
	m_model3DShader = new ShaderProgram();
	m_model3DShader->initFromFiles("shaders/modelAnim.vert", "shaders/modelAnim.frag");

	m_model3DShader->addUniform("model");
	m_model3DShader->addUniform("view");
	m_model3DShader->addUniform("projection");
	m_model3DShader->addUniform("NormalMatrix");
	m_model3DShader->addUniform("LightPosition");
	m_model3DShader->addUniform("LightIntensity");

	m_model3DShader->addUniform("Kd");
	m_model3DShader->addUniform("Ka");
	m_model3DShader->addUniform("Ks");
	m_model3DShader->addUniform("shininess");
	m_model3DShader->addUniform("boneNum");
	m_model3DShader->addUniform("animIndex");
	m_model3DShader->addUniform("animMaxFrame");
	m_model3DShader->addUniform("transMaxFrame");

	m_model3DShader->addUniform("hasTextureDiffuse");
	m_model3DShader->addUniform("hasTextureSpecular");
	m_model3DShader->addUniform("texture_diffuse1");
	m_model3DShader->addUniform("texture_specular1");

	glUniform1i(m_model3DShader->uniform("animIndex"), 0);

	m_model3DComputeShader = new ShaderProgram();
	m_model3DComputeShader->initFromFiles("shaders/modelAnim.comp");

	m_model3DComputeShader->addUniform("animMaxFrame");
	m_model3DComputeShader->addUniform("transMaxFrame");
}

void MyGlWindow::draw(glm::mat4 view, glm::mat4 projection)
{
	glViewport(0, 0, m_width, m_height);

	float time = (clock() - startTime) / 1000.0f;

	drawFloor(m_floorShader, view, projection);

	for (unsigned int i = 0; i< m_ModelManager->m_AnimatedModelData.size(); i++)
	{
		drawAnimatedModel3D(m_ModelManager->m_AnimatedModelData[i], m_model3DShader, view, projection);
	}
}

void MyGlWindow::drawFloor(ShaderProgram * shader, glm::mat4 & view, glm::mat4 & projection)
{
	m_model.glPushMatrix();
	m_model.glTranslate(0.0f, 0.0f, 0.0f);

	shader->use();
	glm::mat4 mvp = projection * view * m_model.getMatrix();
	glUniformMatrix4fv(shader->uniform("MVP"), 1, GL_FALSE, glm::value_ptr(mvp));

	if (m_floor)
		m_floor->draw();

	shader->disable();
	m_model.glPopMatrix();
}

void MyGlWindow::drawCube(ShaderProgram * shader, glm::mat4 & view, glm::mat4 & projection)
{
	m_model.glPushMatrix();

	m_model.glTranslate(0, 1, 0);
	
	shader->use();
	{
		glm::mat4 mvp = projection * view * m_model.getMatrix();
		glUniformMatrix4fv(shader->uniform("mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

		if (m_cube)
			m_cube->draw();
	}
	shader->disable();

	m_model.glPopMatrix();
}

void MyGlWindow::drawAnimatedModel3D(AnimatedModelData* modelData, ShaderProgram * shader, glm::mat4 & view, glm::mat4 & projection)
{
	// set uniforms in compute shader
	m_model3DComputeShader->use();
	{
		glUniform1f(m_model3DComputeShader->uniform("animMaxFrame"), modelData->animMaxFrame);
		glUniform1f(m_model3DComputeShader->uniform("transMaxFrame"), modelData->transMaxFrame);
		glDispatchCompute(modelData->instancingCount, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	}
	m_model3DComputeShader->disable();

	m_model.glPushMatrix();

	m_model3DShader->use();

	m_model.glScale(0.035f, 0.035f, 0.035f);
	model = m_model.getMatrix();

	mview = view * model;

	imvp = glm::inverse(mview);
	nmat = glm::mat3(glm::transpose(imvp));

	glm::vec4 lightPos(10, 10, 10, 0);
	glm::vec3 LightIntensity(1, 1, 1);

	glUniformMatrix4fv(m_model3DShader->uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(m_model3DShader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(m_model3DShader->uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix3fv(m_model3DShader->uniform("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(nmat));

	glUniform4fv(m_model3DShader->uniform("LightPosition"), 1, glm::value_ptr(lightPos));
	glUniform3fv(m_model3DShader->uniform("LightIntensity"), 1, glm::value_ptr(LightIntensity));

	glUniform3fv(m_model3DShader->uniform("Kd"), 1, glm::value_ptr(modelData->model->diffuse));
	glUniform3fv(m_model3DShader->uniform("Ka"), 1, glm::value_ptr(modelData->model->ambient));
	glUniform3fv(m_model3DShader->uniform("Ks"), 1, glm::value_ptr(modelData->model->specular));
	glUniform1f(m_model3DShader->uniform("shininess"), modelData->model->shininess);
	glUniform1i(m_model3DShader->uniform("boneNum"), modelData->model->m_NumBones);
	glUniform1f(m_model3DShader->uniform("animMaxFrame"), modelData->animMaxFrame);
	glUniform1f(m_model3DShader->uniform("transMaxFrame"), modelData->transMaxFrame);

	glUniform1i(m_model3DShader->uniform("hasTextureDiffuse"), 0);
	glUniform1i(m_model3DShader->uniform("hasTextureSpecular"), 0);

	for (GLuint i = 0; i < modelData->model->meshDatum.size(); i++)
	{
		// Bind appropriate textures
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;

		for (GLuint j = 0; j < modelData->model->meshDatum[i].textures.size(); j++)
		{
			glActiveTexture(GL_TEXTURE0 + j); // Active proper texture unit before binding

											  // Retrieve texture number (the N in diffuse_textureN)
			std::stringstream ss;
			std::string number;
			std::string name = modelData->model->meshDatum[i].textures[j].type;

			if (name == "texture_diffuse")
			{
				ss << diffuseNr++; // Transfer GLuint to stream
				glUniform1i(m_model3DShader->uniform("hasTextureDiffuse"), 1);
			}
			else if (name == "texture_specular")
			{
				ss << specularNr++; // Transfer GLuint to stream
				glUniform1i(m_model3DShader->uniform("hasTextureSpecular"), 1);
			}

			number = ss.str();
			// Now set the sampler to the correct texture unit

			std::string nn = (name + number);
			glUniform1i(m_model3DShader->uniform((nn).c_str()), j);

			// And finally bind the texture.
			glBindTexture(GL_TEXTURE_2D, modelData->model->meshDatum[i].textures[j].id);
		}

		// Draw model
		modelData->model->draw(i, modelData->instancingCount);
	}

	m_model3DShader->disable();

	m_model.glPopMatrix();
}

void MyGlWindow::setAnimationIndex(int index)
{
	m_model3DShader->use();
	glUniform1i(m_model3DShader->uniform("animIndex"), index);
	m_model3DShader->disable();
}

void MyGlWindow::switchAnimating()
{
	m_ModelManager->SwitchisAnimating();
}

MyGlWindow::~MyGlWindow()
{
	delete m_floorShader;
	delete m_cube;
	delete m_model3DShader;
}