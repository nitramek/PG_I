#include "stdafx.h"

Color4 ReflectionSampler::fr()
{
	return Color4(0.6f / M_PI);
}

std::tuple<Color4, Vector3> ReflectionSampler::sample(const Vector3& incoming_direction,
	const Vector3& normal, const Color4& diffuse_color)
{
	Vector3 reflect = Vector3(incoming_direction).normalize();
	Vector3 reflected_direction;
	
	if(reflect.dot(normal) < 0)
	{
		reflected_direction = reflect.reflect(-normal).normalize();
	}else
	{
		reflected_direction = reflect.reflect(normal).normalize();
	}
	

	//fr() *
	float dot = reflected_direction.dot(normal);
	Color4 color =  Color4(dot);
	return std::make_tuple(color, reflected_direction);
}
