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

float RayTracer::isInShadow(Vector3 position, Vector3 lightVector) const
{
	Ray shadowRay = Ray(position, lightVector, 0.01f, lightVector.L2Norm());
	rtcOccluded(scene, shadowRay);
	return shadowRay.isCollided() ? 0.f : 1.f;
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
	float cos_theta2 = (load.normal).dot(-rd);
	if (cos_theta2 < 0)
	{
		load.normal = -load.normal;
		cos_theta2 = (load.normal).dot(-rd);
	}

	float cos_thetai_sqrt = 1 - SQR(n1 / n2) * (1 - SQR(cos_theta2));
	if (cos_thetai_sqrt < 0)
	{
		//wtf to do in total reflection
		cos_thetai_sqrt = 0.6;
	}
	float cos_theta1 = sqrt(cos_thetai_sqrt);
	Vector3 rr = -(n1 / n2) * rd - ((n1 / n2) * cos_theta2 + cos_theta1) * load.normal;
	rr.normalize();

	Vector3 l = rr - (2 * (load.normal.dot(rr))) * load.normal;
	Vector3 lr = -l;
	Ray transmitedRay(load.position, lr, 0.01f);
	Color4 transmitedColor = this->trace(transmitedRay, nest - 1, load.material);

	float rs = std::pow((n1 * cos_theta1 - n2 * cos_theta2) / (n1 * cos_theta1 + n2 * cos_theta2), 2);
	float rp = std::pow((n1 * cos_theta2 - n2 * cos_theta1) / (n1 * cos_theta2 + n2 * cos_theta1), 2);
	float reflectivity = 0.5f * (rs + rp);
	float transmitivity = 1 - reflectivity;
	return load.ambient_color +
		transmitivity * transmitedColor * load.material->diffuse +
		reflectivity * reflected_trace * load.material->get_reflexivity();
}

Color4 RayTracer::reflectedColor(RayPayload load, uint nest, Vector3 rd, Color4& reflected_trace) const
{
	Vector3 reflectedRayDirection = (rd).reflect(load.normal);
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
		Vector3 lightVector = load.light_vector().normalize();

		Color4 reflected_color = Color4(0.f);
		if (nest > 0)
		{
			reflected_color = reflectedColor(load, nest, rd, reflected_color);

			if (load.material->get_name() == "green_plastic_transparent")
			{
				return transmited_color(load, nest, rd, reflected_color, materialBefore);
			}
		}
		Color4 diffuse = load.diffuse_color * std::max(lightVector.dot(load.normal), 0.f);

		Vector3 halfVector = load.half_vector().normalize();
		Color4 specular = load.specular_color * std::pow(std::max(halfVector.dot(load.normal), 0.f), 2);
		float diffuseScale = (1 - load.material->reflectivity);

		float inShadow = isInShadow(load.position, lightVector);

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
