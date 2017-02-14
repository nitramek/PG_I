#pragma once
#include "stdafx.h"

#define EXPECT(expr, cmnt) if (expr) { printf(cmnt); printf(" passed\n"); } else{ printf(cmnt); printf(" DIDNT NOT PASSED \n"); finalTest &= false;}


namespace Testing
{
	static bool test_sphere_intersect()
	{
		bool finalTest = true;
		Ray ray = Ray(Vector3(0.f, 0.f, -5.f), Vector3(0.f, 0.f, 1.f), 0.001);
		SphereArea sphere_area = SphereArea{Vector3(0.f, 0.f, 0.f), 1.f};
		Intersector::intersect(ray, sphere_area);
		EXPECT(ray.isCollided() == true, "Collision");
		Vector3 position = ray.eval(ray.tfar);
		EXPECT(ray.isCollided() == true, "Collision");
		return finalTest;
	}
	static bool test_sphere_not_intersect()
	{
		bool finalTest = true;
		Ray ray = Ray(Vector3(0.f, 0.f, -5.f), Vector3(0.f, 0.f, -1.f), 0.001);
		SphereArea sphere_area = SphereArea{ Vector3(0.f, 0.f, 0.f), 1.f };
		Intersector::intersect(ray, sphere_area);
		EXPECT(ray.isCollided() == false, "Not Collision");
		return finalTest;
	}
	static bool test_normal()
	{
		bool finalTest = true;
		Ray ray = Ray(Vector3(0.f, 0.f, -5.f), Vector3(0.f, 0.f, 1.f), 0.001);
		SphereArea sphere_area = SphereArea{ Vector3(0.f, 0.f, 0.f), 2.f };
		Intersector::intersect(ray, sphere_area);
		Vector3 position = ray.collidedPosition();
		Vector3 normal = ray.collided_normal;
		EXPECT(position == Vector3(0.f, 0.f, -2.f), "Position passed");
		EXPECT(normal == Vector3(0.f, 0.f, -1.f), "Normal passed");
		Ray passThrough = Ray(position, Vector3(0.f, 0.f, 1.f), 0.01f);
		Intersector::intersect(passThrough, sphere_area);
		Vector3 passThroughCollision = passThrough.collidedPosition();
		EXPECT(passThroughCollision == Vector3(0.f, 0.f, 2.f), "Passthrough colision");
		return finalTest;
	}



	static bool testAll()
	{
		bool result = true;
		result &= test_sphere_intersect();
		result &= test_sphere_not_intersect();
		result &= test_normal();

		return result;
	}
}
