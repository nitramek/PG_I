#pragma once
#include "stdafx.h"

class RayTracer : public Tracer
{
public:


	RayTracer(RayResolver resolver, const RTCScene& scene);

	Color4 trace(Ray& ray, uint nest) override;
	Color4 trace(Ray& ray, uint nest, Material * material) const;
	virtual ~RayTracer();
};

