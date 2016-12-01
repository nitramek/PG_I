#pragma once
#include "stdafx.h"

class ImportantSampler: public Sampler
{
public:


	float pdf(float cosTheta);
	std::tuple<Color4, Vector3> sample(const Vector3& incoming_direction, 
		const Vector3& normal, const Material* const material) override;
};


