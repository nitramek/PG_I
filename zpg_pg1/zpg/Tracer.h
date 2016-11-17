#pragma once


class Tracer
{
protected :
	const RayResolver resolver;
	const RTCScene& scene;

public:

	Tracer(RayResolver resolver, const RTCScene& scene): resolver(resolver), scene(scene)
	{
	}

	float isInShadow(Vector3 position, Vector3 lightVector) const
	{
		Ray shadowRay = Ray(position, lightVector, 0.01f, lightVector.L2Norm());
		rtcOccluded(scene, shadowRay);
		return shadowRay.isCollided() ? 0.f : 1.f;
	}
	RayPayload getRayPayload(Ray& ray) const
	{
		return resolver(ray);
	}

	virtual Color4 trace(Ray& ray, uint nest) = 0;

	virtual ~Tracer()
	{
	}

	/**
	 * Return: <Vektor ktery prorazil, reflectivity, transmitivity>
	 */
	static std::tuple<Vector3, float, float> schnell_and_fresnel(float n1, float n2, const Vector3& normal, const Vector3& transmited)
	{
		Vector3 normal_copy = normal;
		float cos_theta2 = (normal_copy).dot(transmited);
		if (cos_theta2 < 0)
		{
			normal_copy = -normal_copy;
			cos_theta2 = (normal_copy).dot(transmited);
		}

		float cos_thetai_sqrt = 1 - SQR(n1 / n2) * (1 - SQR(cos_theta2));
		if (cos_thetai_sqrt < 0)
		{
			//wtf to do in total reflection
			cos_thetai_sqrt = 0.6;
		}
		float cos_theta1 = sqrt(cos_thetai_sqrt);
		Vector3 rr = -(n1 / n2) * (-transmited) - ((n1 / n2) * cos_theta2 + cos_theta1) * normal_copy;

		Vector3 l = rr - (2 * (normal_copy.dot(rr))) * normal_copy;
		Vector3 lr = -l;

		std::tuple<float, float> fresnel_res = fresnel(n1, n2, cos_theta1, cos_theta2);
		return std::make_tuple(lr, std::get<0>(fresnel_res), std::get<1>(fresnel_res));
	}

	/**
	 * First item is reflectivity, second is transmitivity
	 */
	static std::tuple<float, float> fresnel(float n1, float n2, float cos_theta_i, float cos_theta_o)
	{
		float rs = std::pow((n1 * cos_theta_i - n2 * cos_theta_o) / (n1 * cos_theta_i + n2 * cos_theta_o), 2);
		float rp = std::pow((n1 * cos_theta_o - n2 * cos_theta_i) / (n1 * cos_theta_o + n2 * cos_theta_i), 2);
		float reflectivity = 0.5f * (rs + rp);
		float transmitivity = 1 - reflectivity;
		return std::make_tuple(reflectivity, transmitivity);
	}
};

