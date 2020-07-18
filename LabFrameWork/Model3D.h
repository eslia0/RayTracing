
#pragma once
// Std. Includes
#include <string>
#include <iostream>
#include <map>
#include <chrono>
#include <vector>
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

#include "Mesh.h"
#include "Loader.h"
#include "ModelView.h"

class Timer {
	typedef std::chrono::high_resolution_clock high_resolution_clock;
	typedef std::chrono::milliseconds milliseconds;
public:
	explicit Timer(bool run = false)
	{
		if (run)
			Reset();
	}
	void Reset()
	{
		_start = high_resolution_clock::now();
	}
	milliseconds Elapsed() const
	{
		return std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - _start);
	}
	template <typename T, typename Traits>
	friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
	{
		return out << timer.Elapsed().count();
	}
private:
	high_resolution_clock::time_point _start;
};

class Model3D
{
public:
	// To make sure that scene data didn't changed
	const aiScene* scene;
	const aiScene* animScene;

	GLuint VAO;
	GLuint VBO_offset, SSBO_animationFrame, SSBO_isAnimating, SSBO_animationSpeed, SSBO_transformMatrix, SSBO_transformFrame;
	std::vector<MeshData> meshDatum;
	std::vector<Texture> textures_loaded;
	std::vector<GLuint> data_indices;

	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
	float shininess;
	int hasTexture;
	int hasTexture2;
	
	/* Bone Data */
	int m_NumBones;
	std::map<std::string, GLuint> m_BoneMapping;
	std::vector<BoneInfo> m_BoneInfo;
	aiMatrix4x4 m_GlobalInverseTransform;

	/*  Functions  */
	// Constructor, expects a filepath to a 3D model.
	Model3D(std::string path)
	{
		loadModel(path.c_str());
	}

	// draw mode is instance + elementsBaseVertex Draw
	// instance for instancing
	// and elementsBase Vertex for 3D Model Data
	void draw(unsigned int index, unsigned int count)
	{
		glBindVertexArray(VAO);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, meshDatum[index].numIndices, GL_UNSIGNED_INT, (void *)(sizeof(GLuint) * meshDatum[index].baseIndex), count, meshDatum[index].baseVertex);
		glBindVertexArray(0);
	}

	void AddAnimationData(std::string path);
	
private:
	Assimp::Importer importer, animImporter;

	/*  Model Data  */
	std::string directory;
	int textureIndex;
	int baseVertex, baseIndex;

	std::vector<glm::vec3> data_positions, data_normals;
	std::vector<glm::vec2> data_texcoords;
	std::vector<glm::vec4> data_boneWeights;
	std::vector<glm::ivec4> data_boneIds;
	std::vector<aiMatrix4x4> data_boneTransforms;

	GLuint EBO, VBO_position, VBO_normal, VBO_texcoord, VBO_boneId, VBO_boneWeight, SSBO_boneTransform;

	/* Functions */
	void loadModel(std::string path);
	void processNode(aiNode* node);
	void processMesh(aiMesh* mesh, int meshIndex);
	void setup();

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	GLint Model3D::TextureFromFile(const char* path, std::string directory);

	/* Animation Function */
	void BoneTransform(const aiScene* modelScene, float TimeInSeconds, std::vector<aiMatrix4x4>& Transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	GLuint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	GLuint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	GLuint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
};