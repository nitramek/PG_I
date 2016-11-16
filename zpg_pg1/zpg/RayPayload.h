#pragma once
#include "stdafx.h"
struct RayPayload
{
	Vector3 normal;
	Vector3 position;
	
	Color4 ambient_color;
	Color4 diffuse_color;
	Color4 specular_color;

	

	const OmniLight *light;
	const Material *material;

	Color4 background_color;
};