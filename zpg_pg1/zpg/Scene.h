#pragma once
#include "stdafx.h"

class Scene
{
	std::vector<Surface *> surfaces;
	std::vector<Material *> materials;
	CubeMap* cubeMap;
	RTCScene scene;

	uint width;
	uint height;
	
	Camera* camera;
	OmniLight* light;

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution;

	Vector3 trace(Ray& ray, uint nest, Material const * materialBefore = nullptr);
	Vector3 radiosity(Ray& ray, uint nest);
	Vector3 genOmega(Vector3 normal, float r1, float r2);
	float fr(Vector3 omega_i, Vector3 omega_o);
public:
	void initEmbree(RTCDevice& device);
	Scene(RTCDevice& device, uint width, uint height);
	~Scene();

	void draw();
};

