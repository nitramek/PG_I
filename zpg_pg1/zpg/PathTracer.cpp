#include "stdafx.h"
#include "PathTracer.h"




PathTracer::PathTracer(RayResolver resolver, const RTCScene& scene): Tracer(resolver, scene)
{
}

Color4 PathTracer::trace(Ray& ray, uint nest)
{
	return Color4();
}

PathTracer::~PathTracer()
{
}
