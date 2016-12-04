#pragma once
#include "stdafx.h"
class ReflectionSampler :
	public Sampler
{
public:
	Color4 fr();
	std::tuple<Color4, Vector3> sample(const Vector3& incoming_direction, const Vector3& normal, const Color4& diffuse_color) override;
};


