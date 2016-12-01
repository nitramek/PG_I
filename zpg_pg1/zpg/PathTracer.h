#pragma once
#include "stdafx.h"
#include "Sampler.h"


class PathTracer :
	public Tracer
{

private:
	std::uniform_real_distribution<float> distribution;
	std::default_random_engine generator;
	bool imporantSampling;
	inline float random();
	inline float pdf() const;
	Color4 fr(Color4 diffuseColor, const Vector3& omega_out, const Vector3& omega_in) const;
	Vector3 random_sphere_direction();
	
	Color4 _trace(Ray& ray, uint nest);
	std::unique_ptr<Sampler> sampler;
public:
	virtual ~PathTracer();

	PathTracer(RayResolver resolver, const RTCScene& scene, std::unique_ptr<Sampler> sampler);
	Color4 trace(Ray& ray, uint nest) override;
	
};

