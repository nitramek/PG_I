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

	virtual float pdf(const Vector3& normal) = 0;
	virtual Vector3 next_direction(const Vector3& normal, const Vector3& incomingDirection) = 0;
	float random()
	{
		return distribution(generator);
	}
	virtual Color4 fr(Color4 diffuseColor, const Vector3& normal, const Vector3& omega_out, const Vector3& omega_in);
};


class UniformSampler : public Sampler
{
public:
	float pdf(const Vector3& normal) override;

	Vector3 next_direction(const Vector3& normal, const Vector3& incomingDirection) override;
	virtual ~UniformSampler() {}
};
class ImportantSampler : public Sampler
{
public:
	float pdf(const Vector3& normal) override;

	Color4 fr(Color4 diffuseColor, const Vector3& normal, const Vector3& omega_out, const Vector3& omega_in) override;
	Vector3 next_direction(const Vector3& normal, const Vector3& incomingDirection) override;
	virtual ~ImportantSampler() {}
};