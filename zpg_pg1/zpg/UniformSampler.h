#pragma once
#include "stdafx.h"

class UniformSampler : public Sampler
{
public:

	float pdf();
	Color4 fr(const Material*const material, const Vector3& omega_out, const Vector3& omega_in);
	Vector3 next_direction(const Vector3& normal, const Vector3& incomingDirection);

	std::tuple<Color4, Vector3> sample(const Vector3& incoming_direction, const Vector3& normal, const Material* const material) override;

	virtual ~UniformSampler()
	{
	}
};
