#include "Model3D.h"
#include <FreeImage.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/code/BVH/BVHLoader.h>
#include <assimp/IOSystem.hpp>

// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model3D::loadModel(const std::string path)
{
	// Read file via ASSIMP
	scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	
	// Check for errors
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	// Retrieve the directory path of the filepath
	this->directory = path.substr(0, path.find_last_of('/'));

	Timer timer(true);
	// Process ASSIMP's root node recursively
	this->processNode(scene->mRootNode);

	std::cout << "Elapsed time: " << std::fixed << timer << "ms\n";
}

void Model3D::processNode(aiNode* node)
{
	baseVertex = 0;
	baseIndex = 0;

	// Count vertices
	int numVertices = 0;
	for (GLuint i = 0; i < scene->mNumMeshes; i++) {
		numVertices += scene->mMeshes[i]->mNumVertices;
	}

	// Data to fill
	data_boneIds.resize(numVertices);
	data_boneWeights.resize(numVertices);

	// Process each mesh located at the current node
	for (GLuint i = 0; i < scene->mNumMeshes; i++)
	{
		// The node object only contains indices to index the actual objects in the scene. 
		// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[i];
		this->processMesh(mesh, i);
	}

	setup();
}

void Model3D::processMesh(aiMesh* mesh, int meshIndex)
{
	MeshData meshData;
	meshData.baseVertex = baseVertex;
	meshData.baseIndex = baseIndex;
	meshData.numIndices = mesh->mNumFaces * 3;
	baseVertex += mesh->mNumVertices;
	baseIndex += meshData.numIndices;

	// Walk through each of the mesh's vertices
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 vec3;
		glm::vec2 vec2;
		// Positions
		if (mesh->HasPositions()) {
			vec3.x = mesh->mVertices[i].x;
			vec3.y = mesh->mVertices[i].y;
			vec3.z = mesh->mVertices[i].z;
			data_positions.push_back(vec3);
		}
		
		// Normals
		if (mesh->HasNormals()) {
			vec3.x = mesh->mNormals[i].x;
			vec3.y = mesh->mNormals[i].y;
			vec3.z = mesh->mNormals[i].z;
			data_normals.push_back(vec3);
		}

		// Texture Coordinates
		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec2.x = mesh->mTextureCoords[0][i].x;
			vec2.y = 1.0 - mesh->mTextureCoords[0][i].y;
			data_texcoords.push_back(vec2);
		}
		else
			data_texcoords.push_back(glm::vec2(0, 0));
	}

	// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (GLuint j = 0; j < face.mNumIndices; j++) {
			data_indices.push_back(face.mIndices[j]);
		}
	}

	float shininessStrength = 0;
	aiColor3D dcolor(0.f, 0.f, 0.f);
	aiColor3D acolor(0.f, 0.f, 0.f);
	aiColor3D scolor(0.f, 0.f, 0.f);

	std::vector<Texture> textures;

	// Process materials
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// Diffuse: texture_diffuseN
		// Specular: texture_specularN
		// Normal: texture_normalN

		material->Get(AI_MATKEY_SHININESS, shininessStrength);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, dcolor);
		material->Get(AI_MATKEY_COLOR_AMBIENT, acolor);
		material->Get(AI_MATKEY_COLOR_SPECULAR, scolor);

		// 1. Diffuse maps
		std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. Specular maps
		std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	meshData.textures = textures;
	meshDatum.push_back(meshData);

	glm::vec3 a(acolor.r, acolor.g, acolor.b);
	glm::vec3 d(dcolor.r, dcolor.g, dcolor.b);
	glm::vec3 s(scolor.r, scolor.g, scolor.b);

	if (glm::length(a) == 0) a = glm::vec3(0.2, 0.2, 0.2);
	if (glm::length(d) == 0) d = glm::vec3(1, 1, 1);
	if (glm::length(s) == 0) s = glm::vec3(0.3, 0.3, 0.3);

	diffuse = d;
	ambient = a;
	specular = s;

	if (shininessStrength < 0) shininessStrength = 1.0f;
	// Return a mesh object created from the extracted mesh data

	// Get Global Inverse Matrix
	m_GlobalInverseTransform = scene->mRootNode->mTransformation.Inverse();

	// Get bone Info, Bone Indexs and Bone Weights
	for (GLuint i = 0; i < mesh->mNumBones; i++) {
		GLuint boneIndex = 0;
		std::string BoneName(mesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			// Allocate an index for a new bone
			boneIndex = m_NumBones;
			m_NumBones++;

			// get bone info
			BoneInfo bi;
			bi.BoneOffset = mesh->mBones[i]->mOffsetMatrix;
			m_BoneInfo.push_back(bi);
			m_BoneMapping[BoneName] = boneIndex;
		}
		else {
			boneIndex = m_BoneMapping[BoneName];
		}

		// Get bone Weights and Index form mBones
		for (GLuint j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			GLuint VertexId = meshDatum[meshIndex].baseVertex + mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;

			// Store data into 4 dimension
			for (int k = 0; k < 4; k++) {
				if (data_boneWeights[VertexId][k] == 0) {
					data_boneIds[VertexId][k] = boneIndex;
					data_boneWeights[VertexId][k] = weight;
					break;
				}
			}
		}
	}
}

void Model3D::setup()
{
	std::vector<glm::mat4> translations;
	int index = 0;
	float offset = 0.0f;
	int count = 20;
	int size = 6;

	for (int y = -(count * size) / 2; y < (count * size) / 2; y += size)
	{
		for (int x = -(count * size) / 2; x < (count * size) / 2; x += size)
		{
			glm::vec3 translation;
			translation.x = (float)x + offset;
			translation.z = (float)y + offset;
			glm::mat4 matrix = glm::translate(glm::mat4(1.0f), translation);
			translations.push_back(matrix);
		}
	}

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO_position);
	glGenBuffers(1, &this->VBO_normal);
	glGenBuffers(1, &this->VBO_texcoord);
	glGenBuffers(1, &this->VBO_boneId);
	glGenBuffers(1, &this->VBO_boneWeight);
	glGenBuffers(1, &this->VBO_offset);
	glGenBuffers(1, &this->EBO);
	
	glBindVertexArray(this->VAO);

	// Load data into vertex buffers
	// Vertex Positions
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_position);
	glBufferData(GL_ARRAY_BUFFER, this->data_positions.size() * sizeof(glm::vec3), &this->data_positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Vertex Normals
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, this->data_normals.size() * sizeof(glm::vec3), &this->data_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Vertex Texture Coords
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_texcoord);
	glBufferData(GL_ARRAY_BUFFER, this->data_texcoords.size() * sizeof(glm::vec2), &this->data_texcoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Vertex BoneIds
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_boneId);
	glBufferData(GL_ARRAY_BUFFER, data_boneIds.size() * sizeof(glm::ivec4), data_boneIds.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, 0, 0);

	// Vertex BoneWeights
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_boneWeight);
	glBufferData(GL_ARRAY_BUFFER, data_boneWeights.size() * sizeof(glm::vec4), data_boneWeights.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->data_indices.size() * sizeof(GLuint), &this->data_indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

// Checks all material textures of a given type and loads the textures if they're not loaded yet.
// The required info is returned as a Texture struct.
std::vector<Texture> Model3D::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		GLboolean skip = false;
		for (GLuint j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == str)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str;
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

GLint Model3D::TextureFromFile(const char* path, std::string directory)
{
	//Generate texture ID and load texture data 
	std::string filename = std::string(path);
	std::string fullfilename = directory + '/' + filename;

	glActiveTexture(GL_TEXTURE0 + textureIndex);

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	BYTE *pixels(0);
	//image width and height
	unsigned int width(0), height(0);
	//OpenGL's image ID to map to

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(fullfilename.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(fullfilename.c_str());
	//if still unkown, return failure
	if (fif == FIF_UNKNOWN)
		return false;

	dib = FreeImage_Load(fif, fullfilename.c_str());
	int bitsPerPixel = FreeImage_GetBPP(dib);

	// Convert image by bitperPixel
	FIBITMAP* bitmap32;
	if (bitsPerPixel == 32)
	{
		bitmap32 = dib;
	}
	else
	{
		bitmap32 = FreeImage_ConvertTo32Bits(dib);
	}

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	// Texture loaded with upsidedown. flip vertical
	FreeImage_FlipVertical(dib);

	GLubyte* textureData = FreeImage_GetBits(dib);

	// Generate and bind texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//store the texture data for OpenGL use
	if(fif == FIF_PNG){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, textureData);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData);
	}
	
	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);
	delete(bits);

	textureIndex++;
	return textureID;
}

// Add model's animation data to ssbo
void Model3D::AddAnimationData(std::string path)
{
	// Read file via ASSIMP
	animScene = animImporter.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

	int dur = (int)animScene->mAnimations[0]->mDuration;
	// Store animation data for all play duration
	for (int i = 0; i < dur; i++)
	{
		std::vector<aiMatrix4x4> transform;
		BoneTransform(scene, i, transform);
		data_boneTransforms.insert(data_boneTransforms.end(), transform.begin(), transform.end());
	}

	glBindVertexArray(VAO);

	// Store animation data to ssbo
	glGenBuffers(1, &this->SSBO_boneTransform);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_boneTransform);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data_boneTransforms.size() * sizeof(glm::mat4), data_boneTransforms.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO_boneTransform);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindVertexArray(0);
}

// Calculate Animation Bone Transform
void Model3D::BoneTransform(const aiScene* modelScene, float TimeInSeconds, std::vector<aiMatrix4x4>& Transforms)
{
	aiMatrix4x4 Identity = aiMatrix4x4();

	float TicksPerSecond = (float)(animScene->mAnimations[0]->mTicksPerSecond != 0 ? animScene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float AnimationTime = fmod(TimeInSeconds, roundf(animScene->mAnimations[0]->mDuration));

	ReadNodeHeirarchy(AnimationTime, modelScene->mRootNode, Identity);

	Transforms.resize(m_NumBones);
	for (int i = 0; i < m_NumBones; i++)
	{
		Transforms[i] = m_BoneInfo[i].FinalTransformation.Transpose();
	}
}

// Read animation data in specific time
void Model3D::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	// Find animation by scene animation, nodeName
	const aiAnimation* pAnimation = animScene->mAnimations[0];
	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	aiMatrix4x4 NodeTransformation(pNode->mTransformation);

	if (pNodeAnim)
	{
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		aiMatrix4x4 ScalingM = aiMatrix4x4(Scaling.x, 0.0f, 0.0f, 0.0f,
			0.0f, Scaling.y, 0.0f, 0.0f,
			0.0f, 0.0f, Scaling.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		aiMatrix4x4 RotationM = aiMatrix4x4(RotationQ.GetMatrix());

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);



		aiMatrix4x4 TranslationM = aiMatrix4x4(1.0f, 0.0f, 0.0f, Translation.x,
											   0.0f, 1.0f, 0.0f, Translation.y,
											   0.0f, 0.0f, 1.0f, Translation.z,
											   0.0f, 0.0f, 0.0f, 1.0f);

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	aiMatrix4x4 GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end())
	{
		GLuint BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (GLuint i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

const aiNodeAnim* Model3D::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (GLuint i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void Model3D::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	GLuint PositionIndex = FindPosition(AnimationTime, pNodeAnim);//(unsigned int)AnimationTime;//
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[PositionIndex + 1].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	// assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[PositionIndex + 1].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void Model3D::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	GLuint RotationIndex = FindRotation(AnimationTime, pNodeAnim);//(unsigned int)AnimationTime;// 
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[RotationIndex + 1].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	// assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[RotationIndex + 1].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void Model3D::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	GLuint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);//(unsigned int)AnimationTime;// 
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[ScalingIndex + 1].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	// assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[ScalingIndex + 1].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

GLuint Model3D::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (GLuint i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

GLuint Model3D::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (GLuint i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

GLuint Model3D::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (GLuint i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}