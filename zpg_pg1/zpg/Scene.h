#pragma once
#include "stdafx.h"

class Scene
{
	std::vector<Surface *> surfaces;
	std::vector<Material *> materials;
	CubeMap* cubeMap;
	RTCScene scene;
public:
	Scene(RTCDevice& device);
	~Scene();

	void draw();
};

