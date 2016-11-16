#include "stdafx.h"
#include "RayTracer.h"


RayTracer::RayTracer(RayResolver resolver, const RTCScene& scene): Tracer(resolver, scene)
{
}

Color4 RayTracer::trace(Ray& direction, uint nest)
{
	return this->trace(direction, nest, nullptr);
}

Color4 RayTracer::trace(Ray& ray, uint nest, Material* materialBefore) const
{
	rtcIntersect(scene, ray);
	RayPayload rayPayload = this->getRayPayload(ray);
	if(ray.isCollided())
	{
		return rayPayload.ambient_color + rayPayload.diffuse_color;
	}
	else
	{
		return rayPayload.background_color;
	}
}

RayTracer::~RayTracer()
{
}
