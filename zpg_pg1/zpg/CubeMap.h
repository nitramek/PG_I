#pragma once
#include "stdafx.h"



class CubeMap
{
private:
	Texture *textures[6];
public:
	CubeMap(std::string path);
	Color4 get_texel(Vector3 & direction);
	~CubeMap();
};

