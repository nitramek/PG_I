#pragma once
#include "stdafx.h"

class ImportantSampler: public Sampler
{
public:


	float pdf(float cosTheta);
	Vector3 next_direction();
	Color4 ImportantSampler::fr(Color4 diffuse, float cos_in, float cos_out);
	std::tuple<Color4, Vector3> sample(const Vector3& incoming_direction,
	                                   const Vector3& normal, const Color4& diffuse_color) override;
};


