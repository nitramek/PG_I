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
	Vector3 trace(Ray,uint nest);
public:
	Scene(RTCDevice& device, uint width, uint height);
	~Scene();

	void draw();
};

