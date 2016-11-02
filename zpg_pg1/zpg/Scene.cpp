#include "stdafx.h"
#include <chrono>

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

Scene::Scene(RTCDevice& device, uint width, uint height)
{
	this->width = width;
	this->height = height;
	if (LoadOBJ("../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), this->surfaces, this->materials) < 0)
	{
		throw std::exception("Could not load object");
	}
	this->cubeMap = new CubeMap("../../data/cubebox");
	this->initEmbree(device);
	//this->camera = new Camera(width, height, Vector3(-400.f, -500.f, 370.f),
	//	Vector3(70.f, -40.5f, 5.0f), DEG2RAD(42.185f));
	//this->camera = new Camera(width, height, Vector3(-400.0f, -500.0f, 370.0f), Vector3(70.0f, -40.5f, 5.0f), DEG2RAD(40.0f));
	
	Vector3 viewFrom = Vector3(-140.0f, -175.0f, 110.0f);
	/*viewFrom.x = -50;
	viewFrom.y -= 30;
	viewFrom.z -= 30;*/
	this->camera = new Camera(width, height, viewFrom, Vector3(0.0f, 0.0f, 40.0f), DEG2RAD(42.185f));
	
	
	//this->camera->view_from(),
	this->light = new OmniLight(this->camera->view_from(),
	                            Vector3(0.1f), Vector3(1.f), Vector3(1.f));
	this->light->position.x = this->light->position.x / 2;
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


Vector3 Scene::trace(Ray& ray, uint nest, Material const* materialBefore)
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
		Vector3 cameraVector = -Vector3(ray.dir);
		Vector3 halfVector = cameraVector + lightVector;

		//shneluv zákon - smìry
		//fresneul zakon R - urèuje reflexivitu
		//pro pruhledne materialy bude zvlast vypocet - //green_plastic_transparent
		// cos fit = -n dot odrazeny ve svetle
		// cos fio = V dot N
		// pro obracene normaly treba dat if na záporný cosinus a pøípadì obrátit normálu

		//vektor smerujici ke kamere
		//Vector3 cameraVector = this->camera->view_from() - pos;


		//vektor smerujici ke svetle


		Vector3 rayDirection = ray.direction();
		Vector3 reflectedRayDirection = rayDirection - 2 * normal.cosBetween(rayDirection) * normal;

		Vector3 gainedByReflection = Vector3(0.f);

		Texture* diff_text = material->get_texture(Material::kDiffuseMapSlot);
		Vector3 ambient_color = material->ambient;
		Vector3 diffuse_color = material->diffuse;
		Vector3 specular_color = material->specular;
		if (diff_text != nullptr)
		{
			Color4 diff_texel = diff_text->get_texel(tuv.x, tuv.y);
			diffuse_color = Vector3(diff_texel.r, diff_texel.g, diff_texel.b);
		}

		if (nest > 0)
		{
			Ray reflectedRay = Ray(pos, reflectedRayDirection, 0.01);
			gainedByReflection = trace(reflectedRay, nest - 1);
		}
		if (material->get_name() == "green_plastic_transparent" && nest > 0)
		{
			float n1 = materialBefore != nullptr && materialBefore->get_name() == "green_plastic_transparent" ? 1.46f : 1.f;
			float n2 = 1.46f; //jinak index lomu materialu

			//Vector3 refract = (Vector3(0.707107, -0.707107, 0)).refract(0.9, 1, Vector3(0, 1, 0));
			Vector3 refract = (rayDirection).refract(n1, n2, normal);
			//odrážíme ray nebo paprsek svetla?

			Ray transmitedRay(pos, refract, 0.01f);
			Vector3 transmitedColor = this->trace(transmitedRay, nest - 1, material);
			float theta_i = (rayDirection).cosBetween(normal);
			if (theta_i < 0)
			{
				//printf("Theta i lt 0, Nest: %d, geomId: %d, primId: %d\n", nest, ray.geomID, ray.primID);
				theta_i = (rayDirection).cosBetween(-normal);
			}
			float theta_t = 1;
			refract.cosBetween(-normal);
			if (theta_t < 0)
			{
				theta_t = (refract).cosBetween(normal);
			}
			float rs = std::pow((n1 * theta_i - n2 * theta_t) / (n1 * theta_i + n2 * theta_t), 2);
			float rp = std::pow((n1 * theta_t - n2 * theta_i) / (n1 * theta_t + n2 * theta_i), 2);
			float reflectivity = 0.5f * (rs + rp);
			//float r0 = std::pow((n1-n2) / (n1 + n2),2);
			//float reflectivity = r0 + (1 - r0)* std::pow(1 - theta_i,5);
			float transmitivity = 1 - reflectivity;
			return ambient_color + transmitedColor * transmitivity * diffuse_color +
				   specular_color * gainedByReflection * reflectivity;
		}
		Ray shadowRay = Ray(pos, lightVector, 0.01f, lightVector.L2Norm());
		rtcOccluded(this->scene, shadowRay);
		float inShadow = shadowRay.isCollided() ? 0 : 1;
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
	cv::Mat lambertImg(height, width, CV_32FC3);
	
	auto start = std::chrono::system_clock::now();
	
	int nest = 10;
	
	for (int row = 0; row < lambertImg.rows; row++)
	{
		#pragma omp parallel for
		for (int col = 0; col < lambertImg.cols; col++)
		{
			Ray ray = camera->GenerateRay(col, row);
			ray.tnear = 0.01;
			lambertImg.at<cv::Vec3f>(row, col) = this->trace(ray, nest).toCV();
		}
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	printf("Tracing for depth %d, took %f s\n", nest, diff.count());
	cv::namedWindow("Phong", CV_WINDOW_AUTOSIZE);
	cv::imshow("Phong", lambertImg);
	cv::waitKey(0);
}
