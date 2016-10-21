#include "stdafx.h"

namespace embree_structs
{
	struct Vertex { float x, y, z, a; };
	typedef Vertex Normal;
	struct Triangle { int v0, v1, v2; };
};
Scene::Scene(RTCDevice & device, uint width, uint height)
{
	this->width = width;
	this->height = height;
	if (LoadOBJ("../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), this->surfaces, this->materials) < 0)
	{
		throw std::exception("Could not load object");
	}
	this->cubeMap = new CubeMap(std::string("../../data/cubebox"));
	this->scene = scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1/* | RTC_INTERPOLATE*/);
	for (auto * surface : this->surfaces) {
		unsigned geom_id = rtcNewTriangleMesh(scene, RTC_GEOMETRY_STATIC,
			surface->no_triangles(), surface->no_vertices());
		rtcSetUserData(scene, geom_id, surface);
		embree_structs::Vertex * vertices = static_cast< embree_structs::Vertex * >(
			rtcMapBuffer(scene, geom_id, RTC_VERTEX_BUFFER));

		for (int t = 0; t < surface->no_triangles(); ++t)
		{
			for (int v = 0; v < 3; ++v)
			{
				embree_structs::Vertex & vertex = vertices[t * 3 + v];

				vertex.x = surface->get_triangles()[t].vertex(v).position.x;
				vertex.y = surface->get_triangles()[t].vertex(v).position.y;
				vertex.z = surface->get_triangles()[t].vertex(v).position.z;
			}
		}
		rtcUnmapBuffer(scene, geom_id, RTC_VERTEX_BUFFER);

		// vytváøení indexù vrcholù pro jednotlivé trojúhelníky
		embree_structs::Triangle * triangles = static_cast< embree_structs::Triangle * >(
			rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER));

		for (int t = 0, v = 0; t < surface->no_triangles(); ++t)
		{
			embree_structs::Triangle & triangle = triangles[t];

			triangle.v0 = v++;
			triangle.v1 = v++;
			triangle.v2 = v++;
		}

		rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	}
	rtcCommit(scene);
	this->camera = new Camera(width, height, Vector3(-400.f, -500.f, 370.f),
		Vector3(70.f, -40.5f, 5.0f), DEG2RAD(42.185f));
	this->light = new OmniLight(this->camera->view_from(), 
		Vector3(0.1f), Vector3(1.f) , Vector3(1.f));
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
	if(nest > 0){
		rtcIntersect(this->scene, ray);
		if (ray.isCollided()) {
			//srazeno

			Surface * surface = ray.collidedSurface(this->surfaces);
			Material * material = surface->get_material();
			Vector3 normal = ray.collidedNormal(this->surfaces);
			Vector3 pos = ray.collidedPosition();

			//vektor smerujici ke kamere
			Vector3 cameraVector = this->camera->view_from() - pos;
			//vektor smerujici ke svetle
			Vector3 lightVector = this->light->position - pos;
			Vector3 halfVector = cameraVector + lightVector;

			Ray shadowRay = Ray(pos, lightVector);
			rtcIntersect(this->scene, shadowRay);

			float inShadow = shadowRay.isCollided() * 0.8f;
			



			Vector3 diffuse =
				material->diffuse * std::max(lightVector.cosBetween(normal), 0.f) * this->light->diffuse;
			Vector3 specular =
				material->specular * std::pow(std::max(halfVector.cosBetween(normal), 0.f), 12) * this->light->specular;
			Vector3 ambient = material->ambient * this->light->ambient;

			Vector3 rayDirection = ray.direction();
			Vector3 reflectedRayDirection = 2 * (-rayDirection * normal) * normal + rayDirection;

			Vector3 gainedByReflection = trace(Ray(pos, reflectedRayDirection), --nest);
			//Vector3 gainedByReflection = Vector3(0.f);
			Vector3 outputColor = ambient + (diffuse + specular) * inShadow +
				material->get_reflexivity() * gainedByReflection;
			return outputColor;
		}
	}
	//barva pozadi
	Color4 texel = this->cubeMap->get_texel(ray.direction());
	return Vector3(texel.b, texel.g, texel.r);
	
}


void Scene::draw()
{
	//Camera camera = Camera(width, height, Vector3(0.f, 0.f, 0.f),
	//	Vector3(-1.0f, 0.f, 0.f), DEG2RAD(120.f));
	

	cv::Mat normalImg(height, width, CV_32FC3);
	cv::Mat lambertImg(height, width, CV_32FC3);
	for (int row = 0; row < normalImg.rows; row++) {
		for (int col = 0; col < normalImg.cols; col++) {
			Ray ray = camera->GenerateRay(col, row);
			lambertImg.at<cv::Vec3f>(row, col) = this->trace(ray, 5).toCV();
		}
	}
	cv::namedWindow("Phong", CV_WINDOW_AUTOSIZE);
	cv::imshow("Phong", lambertImg);
	cv::waitKey(0);
}
