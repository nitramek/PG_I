#pragma once
#include "stdafx.h"
class Scene
{
	std::vector<Surface *> surfaces;
	std::vector<Material *> materials;
	std::unique_ptr<CubeMap> cubeMap;
	RTCScene scene;

	uint width;
	uint height;
	
	std::unique_ptr<Camera> camera;
	std::unique_ptr<OmniLight> light;
	std::unique_ptr<Tracer> tracer;

	RayPayload resolveRay(Ray& collidedRay) const;


public:
	void initEmbree(RTCDevice& device);
	Scene(RTCDevice& device, uint width, uint height, std::string tracing);
	~Scene();

	void draw();
};

