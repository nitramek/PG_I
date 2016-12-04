#include "stdafx.h"

Color4 ReflectionSampler::fr()
{
	return Color4(0.6f / M_PI);
}

std::tuple<Color4, Vector3> ReflectionSampler::sample(const Vector3& incoming_direction,
	const Vector3& normal, const Color4& diffuse_color)
{
	Vector3 reflected_direction;
	
	if(incoming_direction.dot(normal) < 0)
	{
		reflected_direction = incoming_direction.reflect(-normal).normalize();
	}else
	{
		reflected_direction = incoming_direction.reflect(normal).normalize();
	}
	


	float dot = reflected_direction.dot(normal);
	Color4 color = fr() * dot;
	return std::make_tuple(color, reflected_direction);
}
