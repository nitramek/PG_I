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
	static std::tuple<Vector3, Vector3, float, float> reverse_schnell_and_fresnel(float n1, float n2, Vector3 normal, Vector3 rd)
	{
		rd.normalize();
		normal.normalize();
		float r = n1 / n2;
		
		float cos_theta2 = (-normal).dot(rd);
		if(cos_theta2 < 0)
		{
			normal = -normal;
			cos_theta2 = (-normal).dot(rd);
		}
		float cos_thetai_sqrt = 1.0f - (r * r) * (1 - cos_theta2 * cos_theta2);

		/*bool total = false;*/
		if (cos_thetai_sqrt < 0.0f)
		{
			//wtf to do in total reflection
			//cos_thetai_sqrt = std::asin(n2 / n1);
			//;
			return std::make_tuple(Vector3(), Vector3(), 1.0f, 0.0f);
			//total = true;
		}
		float cos_theta1 = sqrt(cos_thetai_sqrt);

		Vector3 rr = (-r) * (rd) - (r * cos_theta2 + cos_theta1) * normal;
		rr.normalize();
		Vector3 l = rr - (2 * (normal.dot(rr))) * normal;
		Vector3 lr = -l;


		/*if (total && false)
		{
			return std::make_tuple(lr, rr, 1.f, 0.f);
		}*/
		std::tuple<float, float> fresnel_res = fresnel(n2, n1, cos_theta2, cos_theta1);
		return std::make_tuple(lr, rr, std::get<0>(fresnel_res), std::get<1>(fresnel_res));
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

