#pragma once
#include "stdafx.h"

class Sampler
{
private:
	std::uniform_real_distribution<float> distribution;
	std::default_random_engine generator;
public:
	virtual ~Sampler()
	{
	}

	float random()
	{
		return distribution(generator);
	}

	virtual std::tuple<Color4, Vector3> sample(const Vector3& incoming_direction, const Vector3& normal, const Material* const material) = 0;
	
};

