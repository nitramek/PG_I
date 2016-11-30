#include "stdafx.h"


Color4 Sampler::fr(Color4 diffuseColor, const Vector3& normal, const Vector3& omega_out, const Vector3& omega_in)
{
	return Color4(0.6f / M_PI);
}

float UniformSampler::pdf(const Vector3& normal)
{
	return 1.0f / PI_2;
}

Vector3 UniformSampler::next_direction(const Vector3& normal, const Vector3& incomingDirection)
{
	float r1 = random();
	float r2 = random();
	double pi2r1 = PI_2 * r1;
	float sqrtf = std::sqrt(1 - SQR(r2));
	double x = cos(pi2r1) * sqrtf;
	double y = sin(pi2r1) * sqrtf;
	float z = r2;
	Vector3 randomDirection = Vector3(x, y, z);
	randomDirection.normalize();
	float dot = randomDirection.dot(normal);
	if (dot < 0) //opacna normal
	{
		randomDirection = -randomDirection;

		return randomDirection;
	}
}

float ImportantSampler::pdf(const Vector3& normal)
{
	
}

Color4 ImportantSampler::fr(Color4 diffuseColor, const Vector3& normal, const Vector3& omega_out, const Vector3& omega_in)
{
}

Vector3 ImportantSampler::next_direction(const Vector3& normal, const Vector3& incomingDirection)
{
}
