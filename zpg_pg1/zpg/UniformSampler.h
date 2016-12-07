#pragma once
#include "stdafx.h"

class UniformSampler : public Sampler
{
public:

	float pdf();
	Color4 fr(Color4 color, const Vector3& omega_out, const Vector3& omega_in);
	Vector3 next_direction(const Vector3& normal, const Vector3& incomingDirection);

	std::tuple<Color4, Vector3> sample(const Vector3& incoming_direction, const Vector3& normal, const Color4& diffuse_color) override;

	virtual ~UniformSampler()
	{
	}
};
