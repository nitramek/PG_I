#include "stdafx.h"

namespace embree_structs
{
	struct Vertex
	{
		float x, y, z, a;
	};

	typedef Vertex Normal;

	struct Triangle
	{
		int v0, v1, v2;
	};
};

Scene::Scene(RTCDevice& device, uint width, uint height)
{
	this->width = width;
	this->height = height;
	if (LoadOBJ("../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), this->surfaces, this->materials) < 0)
	{
		throw std::exception("Could not load object");
	}
	this->cubeMap = new CubeMap(std::string("../../data/cubebox"));
	this->scene = scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1/* | RTC_INTERPOLATE*/);
	for (auto* surface : this->surfaces)
	{
		unsigned geom_id = rtcNewTriangleMesh(scene, RTC_GEOMETRY_STATIC,
		                                      surface->no_triangles(), surface->no_vertices());
		rtcSetUserData(scene, geom_id, surface);
		embree_structs::Vertex* vertices = static_cast<embree_structs::Vertex *>(
			rtcMapBuffer(scene, geom_id, RTC_VERTEX_BUFFER));

		for (int t = 0; t < surface->no_triangles(); ++t)
		{
			for (int v = 0; v < 3; ++v)
			{
				embree_structs::Vertex& vertex = vertices[t * 3 + v];

				vertex.x = surface->get_triangles()[t].vertex(v).position.x;
				vertex.y = surface->get_triangles()[t].vertex(v).position.y;
				vertex.z = surface->get_triangles()[t].vertex(v).position.z;
			}
		}
		rtcUnmapBuffer(scene, geom_id, RTC_VERTEX_BUFFER);

		// vytváøení indexù vrcholù pro jednotlivé trojúhelníky
		embree_structs::Triangle* triangles = static_cast<embree_structs::Triangle *>(
			rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER));

		for (int t = 0, v = 0; t < surface->no_triangles(); ++t)
		{
			embree_structs::Triangle& triangle = triangles[t];

			triangle.v0 = v++;
			triangle.v1 = v++;
			triangle.v2 = v++;
		}

		rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	}
	rtcCommit(scene);
	//this->camera = new Camera(width, height, Vector3(-400.f, -500.f, 370.f),
	//	Vector3(70.f, -40.5f, 5.0f), DEG2RAD(42.185f));
	//this->camera = new Camera(width, height, Vector3(-400.0f, -500.0f, 370.0f), Vector3(70.0f, -40.5f, 5.0f), DEG2RAD(40.0f));
	this->camera = new Camera(width, height, Vector3(-140.0f, -175.0f, 110.0f), Vector3(0.0f, 0.0f, 40.0f), DEG2RAD(42.185f));
	//this->camera->view_from(),
	this->light = new OmniLight(Vector3(-200, 0, 0),
	                            Vector3(0.1f), Vector3(1.f), Vector3(1.f));
}

Scene::~Scene()
{
	rtcDeleteScene(scene); // zrušení Embree scény

	SafeDeleteVectorItems<Material *>(materials);
	SafeDeleteVectorItems<Surface *>(surfaces);
	delete this->cubeMap;
	delete this->camera;
	delete this->light;
}


Vector3 Scene::trace(Ray& ray, uint nest)
{
	rtcIntersect(this->scene, ray);
	if (ray.isCollided())
	{
		//srazeno
		Surface* surface = ray.collidedSurface(this->surfaces);
		Triangle triangle = surface->get_triangle(ray.primID);
		Material* material = surface->get_material();
		Vector3 normal = ray.collidedNormal(this->surfaces);
		Vector3 pos = ray.collidedPosition();
		Vector2 tuv = triangle.texture_coord(ray.u, ray.v);

		Vector3 lightVector = this->light->position - pos;

		//vektor smerujici ke kamere
		//Vector3 cameraVector = this->camera->view_from() - pos;
		Vector3 cameraVector = -Vector3(ray.dir);
		//vektor smerujici ke svetle
		Vector3 halfVector = cameraVector + lightVector;

		Ray shadowRay = Ray(pos, lightVector, 0.01f, lightVector.L2Norm());
		rtcOccluded(this->scene, shadowRay);
		float inShadow = shadowRay.isCollided() ? 0 : 1;
		//inShadow = 1.f;


		Vector3 rayDirection = ray.direction();
		Vector3 reflectedRayDirection = rayDirection - 2 * normal.cosBetween(rayDirection) * normal;

		Vector3 gainedByReflection = Vector3(0.f);

		if (nest > 0)
		{
			Ray reflectedRay = Ray(pos, reflectedRayDirection, 0.01);
			gainedByReflection = trace(reflectedRay, nest - 1);
		}

		Texture* diff_text = material->get_texture(Material::kDiffuseMapSlot);
		Vector3 ambient_color = material->ambient;
		Vector3 diffuse_color = material->diffuse;
		Vector3 specular_color = material->specular;
		if (diff_text != NULL)
		{
			Color4 diff_texel = diff_text->get_texel(tuv.x, tuv.y);
			diffuse_color = Vector3(diff_texel.r, diff_texel.g, diff_texel.b);
		}
		float cosLN = std::max(lightVector.cosBetween(normal), 0.f);
		float cosHN = std::pow(std::max(halfVector.cosBetween(normal), 0.f), 2);

		Vector3 ambient = ambient_color * this->light->ambient;
		Vector3 diffuse = diffuse_color * cosLN * this->light->diffuse;
		Vector3 specular = specular_color * cosHN * this->light->specular;

		float reflectivity = material->reflectivity;

		Vector3 outputColor = ambient + (diffuse * inShadow * (1 - reflectivity)) +
			reflectivity * specular * gainedByReflection;

		//1 - reflexivity a zajisti rozmistìní složek, pøi velké reflexivitì bude dominantní specukulární A + D + R
		//tak bude velké R a malé D a pøi malé reflectiviì bude velké D a malé R
		return outputColor;
	}
	else
	{
		Vector3 direction = ray.direction();
		Color4 texel = this->cubeMap->get_texel(direction);
		return Vector3(texel.b, texel.g, texel.r);
	}
	//barva pozadi
}


void Scene::draw()
{
	//Camera camera = Camera(width, height, Vector3(0.f, 0.f, 0.f),
	//	Vector3(-1.0f, 0.f, 0.f), DEG2RAD(120.f));


	cv::Mat normalImg(height, width, CV_32FC3);
	cv::Mat lambertImg(height, width, CV_32FC3);
	for (int row = 0; row < normalImg.rows; row++)
	{
		for (int col = 0; col < normalImg.cols; col++)
		{
			Ray ray = camera->GenerateRay(col, row);
			ray.tnear = 0.01;
			lambertImg.at<cv::Vec3f>(row, col) = this->trace(ray, 10).toCV();
		}
	}
	cv::namedWindow("Phong", CV_WINDOW_AUTOSIZE);
	cv::imshow("Phong", lambertImg);
	cv::waitKey(0);
}
