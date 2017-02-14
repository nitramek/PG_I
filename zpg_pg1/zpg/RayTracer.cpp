#include "stdafx.h"
#include "RayTracer.h"


RayTracer::RayTracer(RayResolver resolver, const RTCScene& scene): Tracer(resolver, scene)
{
}

Color4 RayTracer::trace(Ray& direction, uint nest)
{
	std::unique_ptr<Material> defaultMat = std::make_unique<Material>();
	//just so there is no null pointer
	return this->trace(direction, nest, 1.0f);
}


Color4 RayTracer::transmited_color(RayPayload& load, uint nest, Vector3 rd, float iorBefore) const
{
	float n1 = iorBefore;
	float n2 = load.material->ior;//load.material->ior;
	if(iorBefore == 1.5f)
	{
		n2 = 1.0;
	}

	std::tuple<Vector3, Vector3, float, float> transmition = Tracer::reverse_schnell_and_fresnel(n1, n2, load.normal, rd);


	float reflectivity = std::get<2>(transmition);
	float transmitivity = std::get<3>(transmition);

	Color4 transmitedColor = Color4();
	if (transmitivity > 0.0f)
	{
		Ray transmitedRay(load.position, std::get<0>(transmition), 0.01f);
		transmitedColor = this->trace(transmitedRay, nest - 1, n2);
	}

	Color4 reflected_color = Color4();
	if (reflectivity > 0.0f)
	{
		Ray reflectedRay(load.position, rd.normalize().reflect(load.normal), 0.01f);
		reflected_color = this->trace(reflectedRay, nest - 1, n2);
	}
	return transmitivity * transmitedColor * load.diffuse_color
		+ reflectivity * reflected_color * load.material->reflectivity;
}

Color4 RayTracer::reflectedColor(RayPayload& load, uint nest, Vector3 rd) const
{
	Vector3 reflectedRayDirection = rd.normalize().reflect(load.normal);
	Ray reflectedRay = Ray(load.position, reflectedRayDirection, 0.01f);
	return this->trace(reflectedRay, nest - 1, 1.0f);
}

Color4 RayTracer::trace(Ray& ray, uint nest, float iorBefore) const
{
	RayPayload load = this->getRayPayload(ray);

	if (ray.isCollided())
	{
		Vector3 rd = ray.direction().normalize();
		Vector3 lightVector = load.light_vector();

		Color4 reflected_color = Color4(0.f);
		if (nest > 0)
		{
			if (load.material->get_name() == "green_plastic_transparent")
			{
				return transmited_color(load, nest, rd, iorBefore);
			}
			else
			{
				reflected_color = reflectedColor(load, nest, rd);
			}
		}

		float inShadow = isInShadow(load.position, lightVector);
		lightVector.normalize();
		Color4 diffuse = load.diffuse_color * std::max(lightVector.dot(load.normal), 0.f);
		Color4 specular = load.specular_color * std::pow(std::max(load.half_vector().normalize().dot(load.normal), 0.f), 4);
		float diffuseSlider = 1.0f - load.material->reflectivity;
		float specularSlider = load.material->reflectivity;

		return load.ambient_color * load.light->ambient +
			diffuse * inShadow * diffuseSlider +
			specular * specularSlider * reflected_color;
	}
	else
	{
		//return Color4(1.0);
		return load.background_color;
	}
}

RayTracer::~RayTracer()
{
}
