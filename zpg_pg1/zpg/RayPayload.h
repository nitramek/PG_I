#pragma once
#include "stdafx.h"
struct RayPayload
{
	/**
	 * Vzdy bude normalizovana
	 */
	Vector3 normal;
	Vector3 position;
	
	Color4 ambient_color;
	Color4 diffuse_color;
	Color4 specular_color;

	

	const OmniLight *light;
	const Material *material;
	const Vector3 camera_positon;
	Color4 background_color;





	Vector3 light_vector() const
	{
		return light->position - position;
	}
	Vector3 camera_vector() const
	{
		return camera_positon - position;
	}

	Vector3 half_vector() const
	{
		return camera_vector() + light_vector();
	}
};