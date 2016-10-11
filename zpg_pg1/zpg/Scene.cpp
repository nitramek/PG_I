#include "stdafx.h"
#include "Scene.h"


Scene::Scene()
{
	if (LoadOBJ("../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), this->surfaces, this->materials) < 0)
	{
		throw std::exception("Could not load object");
	}

}


Scene::~Scene()
{
}
