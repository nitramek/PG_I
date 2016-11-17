#include "stdafx.h"

#include <chrono>
#include <random>


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

void Scene::initEmbree(RTCDevice& device)
{
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
}

Scene::Scene(RTCDevice& device, uint width, uint height, std::string tracing)
{
	this->width = width;
	this->height = height;
	if (LoadOBJ("../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), this->surfaces, this->materials) < 0)
	{
		throw std::exception("Could not load object");
	}
	this->cubeMap = std::make_unique<CubeMap>("../../data/cubebox");
	this->initEmbree(device);
	auto resolve_ray_func = std::bind(&Scene::resolveRay, this, std::placeholders::_1);
	if(tracing == "RT")
		this->tracer = std::make_unique<RayTracer>(resolve_ray_func, scene);
	else
		this->tracer = std::make_unique<PathTracer>(resolve_ray_func, scene);
	//this->camera = new Camera(width, height, Vector3(-400.f, -500.f, 370.f),
	//	Vector3(70.f, -40.5f, 5.0f), DEG2RAD(42.185f));
	//this->camera = new Camera(width, height, Vector3(-400.0f, -500.0f, 370.0f), Vector3(70.0f, -40.5f, 5.0f), DEG2RAD(40.0f));

	Vector3 viewFrom = Vector3(-140.0f, -175.0f, 110.0f);
	/*viewFrom.x = -50;
	viewFrom.y -= 30;
	viewFrom.z -= 30;*/

	this->camera = std::make_unique<Camera>(width, height, viewFrom, Vector3(0.0f, 0.0f, 40.0f), DEG2RAD(42.185f));


	//this->camera->view_from(),
	this->light = std::make_unique<OmniLight>(this->camera->view_from(),
	                            Vector3(0.1f), Vector3(1.f), Vector3(1.f));
	//this->light->position.x = this->light->position.x / 2;
	//this->light->position.y = 0.f;
}

Scene::~Scene()
{
	rtcDeleteScene(scene); // zrušení Embree scény

	SafeDeleteVectorItems<Material *>(materials);
	SafeDeleteVectorItems<Surface *>(surfaces);
}

RayPayload Scene::resolveRay(Ray& collidedRay) const
{
	if (collidedRay.isCollided())
	{
		Surface* surface = this->surfaces[collidedRay.geomID];
		const Material* material = surface->get_material();
		Triangle& triangle = surface->get_triangle(collidedRay.primID);
		Vector3 position = collidedRay.eval(collidedRay.tfar);
		Vector3 normal = triangle.normal(collidedRay.u, collidedRay.v).normalize();
		Vector2 texture_uv = triangle.texture_coord(collidedRay.u, collidedRay.v);
		Color4 ambient_color = material->ambient;
		Color4 diffuse_color = material->diffuse;
		Color4 specular_color = material->specular;
		Texture* diffuseTexture = material->get_texture(Material::kDiffuseMapSlot);
		if (diffuseTexture != nullptr)
		{
			diffuse_color = diffuseTexture->get_texel(texture_uv.x, texture_uv.y);
		}

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

void Scene::draw()
{
	cv::Mat lambertImg(height, width, CV_32FC4);

	auto start = std::chrono::system_clock::now();

	int nest = 5;

	//not an error

#pragma omp parallel for
	for (int row = 0; row < lambertImg.rows; row++)
	{
		for (int col = 0; col < lambertImg.cols; col++)
		{
			Ray ray = camera->GenerateRay(col, row);
			lambertImg.at<cv::Vec4f>(row, col) = tracer->trace(ray, nest).toCV();
			//lambertImg.at<cv::Vec3f>(row, col) = trace(ray, 5, nullptr).toCV();
		}
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	printf("Tracing for depth %d, took %f s\n", nest, diff.count());
	cv::namedWindow("Phong", CV_WINDOW_AUTOSIZE);
	cv::imshow("Phong", lambertImg);
	cv::waitKey(0);
}
