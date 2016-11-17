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

	RayPayload resolveRay(Ray& collidedRay) const;

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution;

	Vector3 radiosity(Ray& ray, uint nest);
	Vector3 genOmega(Vector3 normal, float r1, float r2);
	float fr(Vector3 omega_i, Vector3 omega_o);
	std::unique_ptr<Tracer> tracer;
public:
	void initEmbree(RTCDevice& device);
	Scene(RTCDevice& device, uint width, uint height, std::string tracing);
	~Scene();

	void draw();
};

