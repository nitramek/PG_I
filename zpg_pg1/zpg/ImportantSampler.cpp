#include "stdafx.h"

float ImportantSampler::pdf(float cosTheta)
{
	return cosTheta / M_PI;
}

Vector3 ImportantSampler::next_direction()
{
	//generovani dalsiho smeru podle verze 35.
	float r1 = random();
	float r2 = random();
	float sqrted = sqrt(1 - r2);
	float x = cos(PI_2 * r1) * sqrted;
	float y = sin(PI_2 * r1) * sqrted;
	return Vector3(x, y, sqrt(r2));

}
float ImportantSampler::fr(float cos_in, float cos_out)
{
	return (0.3 / M_PI) * (1 - abs(cos_out - cos_in));
}
inline Vector3 orthogonal(const Vector3 & v)
{
	return (abs(v.x) > abs(v.z)) ? Vector3(-v.y, v.x, 0.0f) : Vector3(0.0f,
		-v.z, v.y);
}

std::tuple<Color4, Vector3> ImportantSampler::sample(const Vector3& incoming_direction,
                                                     const Vector3& normal, const Color4& diffuse_color)
{
	Vector3 outcoming_direction = next_direction().normalize();
	
	// normal je osa z
	Vector3 o1 = orthogonal(normal).normalize(); // o1 je pomocna osa x
	Vector3 o2 = o1.CrossProduct(normal).normalize(); // o2 je pomocna osa y

	outcoming_direction = Vector3(
		o1.x * outcoming_direction.x + o2.x * outcoming_direction.y + normal.x * outcoming_direction.z,
		o1.y * outcoming_direction.x + o2.y * outcoming_direction.y + normal.y * outcoming_direction.z,
		o1.z * outcoming_direction.x + o2.z * outcoming_direction.y + normal.z * outcoming_direction.z
	).normalize(); // direction je vstupni vektor, ktery chcete "posadit" do ws
	float cos_out = normal.dot(outcoming_direction);
	////} //"posun na nasi polokouli"

	//while (cos_out < 0.6f){
	// 
	//	outcoming_direction = next_direction().normalize();
	//	cos_out = normal.dot(outcoming_direction);
	
	float cos_in = normal.dot(-incoming_direction);
	Color4 color_coeff  = fr(cos_in, cos_out) * cos_out * (1.f / pdf(cos_out));
	



	return std::make_tuple(color_coeff, outcoming_direction);
	

	//kontrola polokoule podle normaly
	//while dot(genDir)
	//normal.dot(rd) > random(0, 1)
}


