#pragma once
#include "stdafx.h"

struct SphereArea
{
	Vector3 center;
	double radius;
};

namespace Intersector
{
	static Ray intersect(Ray& ray, SphereArea sphere_area)
	{
		Vector3 u = ray.direction().normalize();
		Vector3 A = ray.org;
		Vector3 S = sphere_area.center;

		double b = 2 * (A.x * u.x - S.x * u.x +
			A.y * u.y - S.y * u.y +
			A.z * u.z - S.z * u.z);
		double c = SQR(A.x - S.x) + SQR(A.y - S.y) + SQR(A.z - S.z) - SQR(sphere_area.radius);
		double discriminant = SQR(b) - 4 * c;
		if (discriminant >= 0)
		{
			discriminant = sqrt(discriminant);
			if (FLOAT_EQ(discriminant,0))
			{
				double t = -b / 2.0;
				ray.tfar = t;
			}
			else
			{
				double t1 = (-b + discriminant) / 2.0;
				double t2 = (-b - discriminant) / 2.0;
				if (t2 > ray.tnear)
				{
					ray.tfar = t2;
				}
				else
				{
					ray.tfar = t1;
				}
			}
			ray.customIntersector = true;
			ray.collided_normal = ray.eval(ray.tfar) - S;
			ray.collided_normal.normalize();
			ray.geomID = 0;
			return ray;
		}
		else
		{
			return ray;
		}
	}
}
