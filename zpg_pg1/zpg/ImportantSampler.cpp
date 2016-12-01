#include "stdafx.h"

float ImportantSampler::pdf(float cosTheta)
{
	return cosTheta / PI_2;
}

std::tuple<Color4, Vector3> ImportantSampler::sample(const Vector3& incoming_direction, 
	const Vector3& normal, const Material* const material)
{
	//generovani dalsiho smeru podle verze 35.
	//kontrola polokoule podle normaly
	//while dot(genDir)
	//normal.dot(rd) > random(0, 1)
}


