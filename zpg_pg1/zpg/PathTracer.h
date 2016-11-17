#pragma once
#include "stdafx.h"
class PathTracer :
	public Tracer
{
public:
	PathTracer(RayResolver resolver, const RTCScene& scene);
	virtual ~PathTracer();


	Color4 trace(Ray& ray, uint nest) override;
	
};

