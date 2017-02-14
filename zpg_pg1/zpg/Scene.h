#pragma once
#include "stdafx.h"
class Scene
{
protected:
	std::vector<Surface *> surfaces;
	std::vector<Material *> materials;
	std::unique_ptr<CubeMap> cubeMap;
	RTCScene scene;

	uint width;
	uint height;
	
	std::unique_ptr<Camera> camera;
	std::unique_ptr<OmniLight> light;
	std::unique_ptr<Tracer> tracer;
	int nest;
	int super_samples;
	virtual RayPayload resolveRay(Ray& collidedRay) const;



public:
	
	void initEmbree(RTCDevice& device);
	Scene(RTCDevice& device, uint width, uint height, std::string tracing, int nest, int super_samples, std::unique_ptr<Sampler> sampler);
	virtual ~Scene();

	void drawIn(std::string window_name);
};

