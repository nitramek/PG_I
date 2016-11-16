#include "stdafx.h"
#include "CubeMap.h"



CubeMap::CubeMap(std::string path)
{

	this->textures[Direction::FRONT] = Texture::loadTexture((path +  "/front.jpg").c_str());
	this->textures[Direction::BACK] = Texture::loadTexture((path +   "/back.jpg").c_str());
	this->textures[Direction::LEFT] = Texture::loadTexture((path +   "/left.jpg").c_str());
	this->textures[Direction::RIGHT] = Texture::loadTexture((path +  "/right.jpg").c_str());
	this->textures[Direction::TOP] = Texture::loadTexture((path +    "/top.jpg").c_str());
	this->textures[Direction::BOTTOM] = Texture::loadTexture((path + "/bottom.jpg").c_str());
}

Color4 CubeMap::get_texel(Vector3 direction) const {
	Direction dir = direction.LargestComponentSigned();
	float u = 0, v = 0;

	float tmp;
	switch (dir)
	{
	case Direction::FRONT: 
		tmp = 1.f / abs(direction.x);
		u = (direction.y * tmp + 1)  * 0.5f;
		v = (direction.z * tmp + 1)  * 0.5f;
		break;
	case Direction::RIGHT:
		tmp = 1.f / abs(direction.y);
		u = 1 - ((direction.x * tmp + 1)  * 0.5f);
		v = (direction.z * tmp + 1)  * 0.5f;
		
		break;
	case Direction::TOP:
		tmp = 1.f / abs(direction.z);
		u = (direction.y * tmp + 1)  * 0.5f;
		v = 1 - ((direction.x * tmp + 1)  * 0.5f);
		break;
	case Direction::BACK:
		tmp = 1.f / abs(direction.x);
		u = 1 - ((direction.y * tmp + 1)  * 0.5f);
		v = (direction.z * tmp + 1)  * 0.5f;
		break;
	case Direction::LEFT:
		tmp = 1.f / abs(direction.y);
		u = (direction.x * tmp + 1)  * 0.5f;
		v = (direction.z * tmp + 1)  * 0.5f;
		break;
	case Direction::BOTTOM:
		tmp = 1.f / abs(direction.z);
		u = (direction.y * tmp + 1)  * 0.5f;
		v = 1 - (((direction.x * tmp + 1))  * 0.5f);
		break;
	}
	
	

	return this->textures[dir]->get_texel(u, v);
}

CubeMap::~CubeMap()
{
	for (uint i = 0; i < 6; i++)
	{
		delete this->textures[i];
	}
}
