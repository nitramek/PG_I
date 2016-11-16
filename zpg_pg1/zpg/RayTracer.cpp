#include "stdafx.h"
#include "RayTracer.h"


RayTracer::RayTracer(RayResolver resolver, const RTCScene& scene): Tracer(resolver, scene)
{
}

Color4 RayTracer::trace(Ray& direction, uint nest)
{
	return this->trace(direction, nest, nullptr);
}

float RayTracer::isInShadow(Vector3 position, Vector3 lightVector) const
{
	Ray shadowRay = Ray(position, lightVector, 0.01f, lightVector.L2Norm());
	rtcOccluded(scene, shadowRay);
	return shadowRay.isCollided() ? 0.f : 1.f;
}

Color4 RayTracer::trace(Ray& ray, uint nest, const Material* materialBefore) const
{
	rtcIntersect(scene, ray);
	RayPayload load = this->getRayPayload(ray);
	if (ray.isCollided())
	{
		Vector3 rayDirection = ray.direction();
		Vector3 lightVector = load.light_vector();
		float inShadow = isInShadow(load.position, lightVector);
		lightVector.normalize();

		Color4 diffuse = load.diffuse_color * std::max(lightVector.dot(load.normal), 0.f);


		Vector3 halfVector = load.half_vector().normalize();

		Color4 specular = load.specular_color * std::pow(std::max(halfVector.dot(load.normal), 0.f), 4);
		Color4 reflected_trace = Color4(0.f);
		if(nest > 0)
		{
			Vector3 reflectedRayDirection = rayDirection.reflect(load.normal);
			Ray reflectedRay = Ray(load.position, reflectedRayDirection, 0.01f);
			reflected_trace = this->trace(reflectedRay, nest - 1, load.material);
		}
		float diffuseScale = (1 - load.material->reflectivity);


	
	

		return load.ambient_color +
			   inShadow * diffuse * diffuseScale +
			   specular * load.material->reflectivity * reflected_trace;
	}
	else
	{
		return load.background_color;
	}
}

RayTracer::~RayTracer()
{
}
