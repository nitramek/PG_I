#pragma once
#include "stdafx.h"

class RayTracer : public Tracer
{
public:


	RayTracer(RayResolver resolver, const RTCScene& scene);

	Color4 trace(Ray& ray, uint nest) override;
	Color4 transmited_color(RayPayload& load, uint nest, Vector3 rd, float iorBefore) const;
	Color4 reflectedColor(RayPayload& load, uint nest, Vector3 rd) const;
	Color4 trace(Ray& ray, uint nest, float iorBefore) const;
	virtual ~RayTracer();
};

