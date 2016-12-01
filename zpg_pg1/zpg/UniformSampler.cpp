#include "stdafx.h"

Color4 UniformSampler::fr(const Material * const material, const Vector3& omega_out, const Vector3& omega_in)
{
	return Color4(0.3f / M_PI);
}

float UniformSampler::pdf()
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
	}
	return randomDirection;
}

std::tuple<Color4, Vector3> UniformSampler::sample(const Vector3& incoming_direction, const Vector3& normal, const Material* const material)
{
	Vector3 outcomingDirection = next_direction(normal, incoming_direction);
	outcomingDirection.normalize();
	Color4 brdfColor = normal.dot(outcomingDirection) * fr(material, incoming_direction, outcomingDirection) * (1.0f / pdf());
	return std::make_tuple(brdfColor, outcomingDirection);

}



