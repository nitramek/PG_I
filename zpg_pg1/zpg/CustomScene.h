#pragma once
#include "Intersector.h"

class CustomScene: public Scene
{
	SphereArea sphere;
public:

	CustomScene(RTCDevice& device, uint width, uint height, const std::string& tracing, int nest, int super_samples, std::unique_ptr<Sampler> sampler)
		: Scene(device, width, height, tracing, nest, super_samples, std::move(sampler))
	{
		sphere = SphereArea{ Vector3(0.f), 1.f };
	}

	RayPayload resolveRay(Ray& collidedRay) const override;
};

inline RayPayload CustomScene::resolveRay(Ray& collidedRay) const
{
	Intersector::intersect(collidedRay, sphere);
	if (collidedRay.isCollided())
	{
		Surface* surface = this->surfaces[0];
		Material* material = surface->get_material();
		material->set_name("Nope");
		Vector3 position = collidedRay.eval(collidedRay.tfar);
		Vector3 normal = collidedRay.collided_normal;
		if (collidedRay.direction().dot(normal) > 0)
		{
			normal = -normal;
		}
		Color4 ambient_color = material->ambient;
		Color4 diffuse_color = material->diffuse;
		Color4 specular_color = material->specular;


		const OmniLight* light = this->light.get();

		return RayPayload{normal, position, ambient_color, diffuse_color, specular_color, light, material, camera->view_from()};
	}
	else
	{
		RayPayload rayPayload = RayPayload();
		Color4 background = this->cubeMap->get_texel(collidedRay.direction());
		Vector3 background_color = Vector3(background.b, background.g, background.r);
		rayPayload.background_color = background_color;
		return rayPayload;
	}
}

