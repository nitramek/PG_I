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

	Vector3 trace(Ray& ray, uint nest, Material const * materialBefore = nullptr);
public:
	void initEmbree(RTCDevice& device);
	Scene(RTCDevice& device, uint width, uint height);
	~Scene();

	void draw();
};

