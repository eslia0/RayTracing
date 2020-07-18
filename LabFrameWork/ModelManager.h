#pragma once
#include <vector>
#include "Model3D.h"

// �ִϸ��̼� ����� ���� ������
struct AnimatedModelData {
	// �� ������
	Model3D* model;

	// �ν��Ͻ� ����
	int instancingCount;
	
	// �ִϸ��̼� �ӵ�
	float* animationSpeed;

	// �ִϸ��̼��� ���� ������
	float* animCurrentFrame;
	
	// �ִϸ��̼��� �ִ� ������
	float animMaxFrame;

	// �ִϸ��̼� ���, ����
	int* isAnimating;

	//��ġ, ȸ�� ������
	std::vector<glm::mat4> transMatrix;
	
	//��ġ, ȸ�� ���� ������
	float* transStartFrame;
	
	//��ġ, ȸ�� ���� ������
	float* transCurrentFrame;

	//��ġ, ȸ�� �ִ� ������
	float transMaxFrame;
};

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	std::vector<AnimatedModelData*> m_AnimatedModelData;

	// Add Animated Model data to Manager
	void AddAnimatedModelData(std::string transformMatrixPath, Model3D* model3D, int _instancingCount);
	
	// get position, rotation data from txt file
	void SetTransformData(std::string path, int index);
	
	// Animation on, off
	void SwitchisAnimating();

private:
	// Model Index
	int dataIndex;

	// Send Instancing data to shader
	void SetInstancingData(int index);
};