#pragma once


class Tracer
{
protected :
	const RayResolver resolver;
	const RTCScene& scene;

public:

	Tracer(RayResolver resolver, const RTCScene& scene): resolver(resolver), scene(scene)
	{
	}

	RayPayload getRayPayload(Ray& ray) const
	{
		return resolver(ray);
	}

	virtual Color4 trace(Ray& ray, uint nest) = 0;

	virtual ~Tracer()
	{
	}
};

