#pragma once
#include "stdafx.h"

class RayTracer : public Tracer
{
public:


	RayTracer(RayResolver resolver, const RTCScene& scene);

	Color4 trace(Ray& ray, uint nest) override;
	float isInShadow(Vector3 position, Vector3 lightVector) const;
	Color4 transmited_color(RayPayload& load, uint nest, Vector3 rd, Color4 reflected_trace, const Material* materialBefore) const;
	Color4 reflectedColor(RayPayload load, uint nest, Vector3 rd, Color4& reflected_trace) const;
	Color4 trace(Ray& ray, uint nest, const Material* material) const;
	virtual ~RayTracer();
};

