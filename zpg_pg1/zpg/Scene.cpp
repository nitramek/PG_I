#include "stdafx.h"

namespace embree_structs
{
	struct Vertex { float x, y, z, a; };
	typedef Vertex Normal;
	struct Triangle { int v0, v1, v2; };
};
Scene::Scene(RTCDevice& device):cubeMap()
{
	/*if (LoadOBJ("../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), this->surfaces, this->materials) < 0)
	{
		throw std::exception("Could not load object");
	}*/
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

		// vytv��en� index� vrchol� pro jednotliv� troj�heln�ky
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
}

Scene::~Scene()
{
	rtcDeleteScene(scene); // zru�en� Embree sc�ny

	SafeDeleteVectorItems<Material *>(materials);
	SafeDeleteVectorItems<Surface *>(surfaces);
	delete this->cubeMap;
}

void Scene::draw()
{
	int width = 640;
	int height = 480;
	Camera camera = Camera(width, height, Vector3(0.f, 0.f, 0.f),
		Vector3(-1.0f, 0.f, 0.f), DEG2RAD(120.f));
	/*Camera(width, height, Vector3(-400.f, -500.f, 370.f),
		Vector3(70.f, -40.5f, 5.0f), DEG2RAD(42.185f));*/
		
	//
	//42.185f
	camera.Print();

	Vector3 lightPosition(-400.f, -500.f, 370.f);
	cv::Mat normalImg(480, 640, CV_32FC3);
	cv::Mat lambertImg(480, 640, CV_32FC3);
	for (int row = 0; row < normalImg.rows; row++) {
		for (int col = 0; col < normalImg.cols; col++) {
			Ray ray = camera.GenerateRay(col, row);
			//rtcIntersect(scene, ray); 
			if (ray.geomID != RTC_INVALID_GEOMETRY_ID)
			{

				Surface * surface = surfaces[ray.geomID];
				Triangle & triangle = surface->get_triangle(ray.primID);
				const Vector3 normal = triangle.normal(ray.u, ray.v);

				//normala z trojuhelniku je vypoctena na konkretnim zasazeni, zatimco Ng bude pro cel� trojuhelnik
				Vector3 geometry_normal = normal; // Ng je nenormalizovan� norm�la zasa�en�ho troj�heln�ka vypo�ten� nesouhlasn� s pravidlem prav� ruky o z�vitu
				geometry_normal.Normalize(); // norm�la zasa�en�ho troj�heln�ka vypo�ten� souhlasn� s pravidlem prav� ruky o z�vitu

				//normalovy shader
				Vector3 normal_ = (geometry_normal / 2.f ) + Vector3(0.5f, 0.5f, 0.5f);
				normalImg.at<cv::Vec3f>(row, col) = normal_.toCV();;

				//lambertuv
				Vector3 position = ray.eval(ray.tfar);
				position.Normalize();
				Vector3 lightVector = lightPosition - position;
				lightVector.Normalize();

				float cos = lightVector.DotProduct(geometry_normal);
				lambertImg.at<cv::Vec3f>(row, col) = cos * cv::Vec3f(0.5f, 0.5f, 0.5f);
			}
			else {
				Color4 color = this->cubeMap->get_texel(ray.direction());
				cv::Vec3f colorCV = cv::Vec3f(color.b, color.g, color.r);
				lambertImg.at<cv::Vec3f>(row, col) = colorCV;
				normalImg.at<cv::Vec3f>(row, col) = colorCV;
			}

		}
	}

	//cv::cvtColor(normalImg, normalImg, CV_RGB2BGR);
	//cv::namedWindow("NormalShader", CV_WINDOW_AUTOSIZE);
	//cv::imshow("NormalShader", normalImg);
	cv::cvtColor(lambertImg, lambertImg, CV_RGB2BGR);
	cv::namedWindow("LambertShader", CV_WINDOW_AUTOSIZE);
	cv::imshow("LambertShader", lambertImg);

	cv::waitKey(0);

}
