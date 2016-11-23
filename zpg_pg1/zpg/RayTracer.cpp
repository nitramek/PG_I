#include "stdafx.h"
#include "RayTracer.h"


RayTracer::RayTracer(RayResolver resolver, const RTCScene& scene): Tracer(resolver, scene)
{
}

Color4 RayTracer::trace(Ray& direction, uint nest)
{
	std::unique_ptr<Material> defaultMat = std::make_unique<Material>();
	//just so there is no null pointer
	return this->trace(direction, nest, defaultMat.get());
}


Color4 RayTracer::transmited_color(RayPayload& load, uint nest, Vector3 rd, Color4 reflected_trace, const Material* materialBefore) const
{
	//float cos_theta2 = (load.normal).dot(-rd);
	load.normal.normalize();
	rd.normalize();
	float n1 = materialBefore->ior;
	float n2 = load.material->ior;
	if (materialBefore->get_name() == "green_plastic_transparent")
	{
		n2 = 1;
	}
	std::tuple<Vector3, float, float> transmition = Tracer::reverse_schnell_and_fresnel(n1, n2, load.normal, -rd);

	Ray transmitedRay(load.position, std::get<0>(transmition), 0.01f);
	float reflectivity = std::get<1>(transmition);
	float transmitivity = std::get<2>(transmition);
	Color4 transmitedColor = Color4();
	if (transmitivity > 0) {
		transmitedColor = this->trace(transmitedRay, nest - 1, load.material);
	}

	
	return load.ambient_color +
		transmitivity * transmitedColor * load.material->diffuse +
		reflectivity * reflected_trace * load.material->get_reflexivity();
}

Color4 RayTracer::reflectedColor(RayPayload load, uint nest, Vector3 rd, Color4& reflected_trace) const
{
	Vector3 reflectedRayDirection = (-rd).reflect(load.normal);
	Ray reflectedRay = Ray(load.position, reflectedRayDirection, 0.01f);
	return this->trace(reflectedRay, nest - 1, load.material);
}

Color4 RayTracer::trace(Ray& ray, uint nest, const Material* materialBefore) const
{
	rtcIntersect(scene, ray);
	RayPayload load = this->getRayPayload(ray);
	if (ray.isCollided())
	{
		Vector3 rd = ray.direction().normalize();
		Vector3 lightVector = load.light_vector();

		Color4 reflected_color = Color4(0.f);
		if (nest > 0)
		{
			reflected_color = reflectedColor(load, nest, rd, reflected_color);

			if (load.material->get_name() == "green_plastic_transparent")
			{
				return transmited_color(load, nest, rd, reflected_color, materialBefore);
			}
		}
		
		float inShadow = isInShadow(load.position, lightVector);
		Color4 diffuse = load.diffuse_color * std::max(lightVector.normalize().dot(load.normal), 0.f);

		Vector3 halfVector = load.half_vector().normalize();
		Color4 specular = load.specular_color * std::pow(std::max(halfVector.dot(load.normal), 0.f), 2);
		float diffuseScale = (1 - load.material->reflectivity);

		

		return load.ambient_color +
			inShadow * diffuse * diffuseScale + 
			specular * load.material->reflectivity * reflected_color;
	}
	else
	{
		return load.background_color;
	}
}

RayTracer::~RayTracer()
{
}
