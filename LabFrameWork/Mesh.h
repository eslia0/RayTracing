#pragma once
// Std. Includes
#include <iostream>
#include <string>
#include <vector>

// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <assimp/scene.h>

struct Vertex {
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
};

struct Texture {
	GLuint id;
	std::string type;
	aiString path;
};

#define NUM_BONES_PER_VERTEX 4

struct BoneInfo
{
	aiMatrix4x4 BoneOffset;
	aiMatrix4x4 FinalTransformation;

	BoneInfo()
	{
		BoneOffset = aiMatrix4x4();
		FinalTransformation = aiMatrix4x4();
	}
};

class Mesh {
public:
	GLuint VAO;

	/*  Mesh Data  */
	std::vector<Texture> textures;

	/*  Functions  */
	// Constructor
	Mesh(std::vector<Texture> textures, GLuint baseVertex, GLuint baseIndex);

	// Render the mesh
	void Mesh::draw(GLuint count);

private:

};

struct MeshData
{
	std::vector<Texture> textures;
	GLuint baseVertex;
	GLuint baseIndex;
	GLuint numIndices;
};