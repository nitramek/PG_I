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

		// vytv��en� index� vrchol� pro jednotliv� troj�heln�ky
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
	rtcDeleteScene(scene); // zru�en� Embree sc�ny

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


//v odmocnine je totalni odraz, tzn. R = 1
//svetlo je to odkud prijimame paprsek tzn. v tomhle kode je to refract vector


//renderovaci rovnice
//svitivost pod uhlem omega o = Le(omega_o) + integral skrze hemisferu(Li(omega_i)*f_r(omega_o, omega_i) * (n * omega_i)(
//soucet toku prenasoboene odrazivosti a prenasobena n���m jako cos)
//omega o - odrazena svitivost
//omega i - incoming uhel
//fr 4d funkce - bidirectional distribution funkction - funkce odrazovosti
//pro vypocet integralu pou��t monte carlo a dosadit za xi = omega i
//pro zacatek staci Li = 1, pozdeji vysledek z CubeMapy
//fr pro lamberutv povrch bude fr = albedo / PI albedo..odraziovost materialu v <0,1>
//omega i = smerovy vektor
//rekurze az bude Le > 0
//vypocet aspo� 10 prvku, klasicky vezmeme
//vice vzorku pro jeden pixel t�sn� vedle posunutue - prusecik, generovani omega i, 
//poslani paprsku ve smeru omega, omezeni rekurze, pak se vr�t� 0, pro nas pripad Le = 1
//a vysledek cube mapy, pokud se nic netrefi, pokud se trefi, tak poslat paprsek na dalsi rekurzi

//poslat paprsek od kamery  
// paprsek -> pokud nic netrefi, vratit Le = 1 nebo z cube mapy hodnotu
//			  pokud se trefi, vygenerovat novy paprsek ve smeru omega

//global total compodium - vzorec 34
//r1,r2 = random cisla <0,1>, generoovani v kouli a podle normaly prijmout ci odebrat
//pro zaporny dot bodu s normalou odmitnout a pozadat o novy paprsek nebo negace bodu
//uniformni qasi nahodna cisla davaji lepsi
// nebo vygenerujeme pozici v lokalnich souradnicich v hemisfere

//funkce co vraci omegai, papne normalu 
//radiance  - 

//metoda monte carlo - zpusob pocitani integralu
//vypocitame hodnoty ve funkci, kterou aproximujeme na obdelnik a pote staci ud�lat obsah obdelniku
//reseni integralu od a do b f(x)~=  (b-a) * prumer f (1/N) * (suma f(i) podle N) - 
// 1/N * suma skrze N (fi(xi) / pdf(xi) 
//diky 1 / pdf(xi) - castejsi vzorky budou mit mensi hodnotu, mene castejsi vyssi
//- pdf je pravdepodobnost vyskytu xi, pro uniformni pripad (1 / (b - a))
Vector3 Scene::radiosity(Ray& ray, uint nest)
{
	if (nest == 0)
	{
		return Vector3(0.f);
	}
	if (ray.intersect(scene).isCollided())
	{
		Surface* surface = surfaces[ray.geomID];
		Triangle triangle = surface->get_triangle(ray.primID);
		Material* material = surface->get_material();
		Vector3 normal = triangle.normal(ray.u, ray.v);
		Vector3 position = ray.collidedPosition();
		Vector3 rayDirection = ray.direction();
		Vector3 lightVector = this->light->position - position;
		Texture* diff_text = material->get_texture(Material::kDiffuseMapSlot);
		Vector2 tuv = triangle.texture_coord(ray.u, ray.v);

		Vector3 omega = genOmega(normal, distribution(generator), distribution(generator));
		if (normal.cosBetween(omega) < 0)
		{
			omega = -omega;
		}
		Ray omegaRay = Ray(position, omega, 0.01);
		Vector3 refractedColor = fr(omega, rayDirection) * normal.cosBetween(omega) * radiosity(omegaRay, nest - 1) * PI_2;

		Vector3 diffuse_color = material->diffuse;

		if (diff_text != nullptr)
		{
			Color4 diff_texel = diff_text->get_texel(tuv.x, tuv.y);
			diffuse_color = Vector3(diff_texel.r, diff_texel.g, diff_texel.b);
		}

		//return sum;
		return material->ambient +
			refractedColor * diffuse_color * normal.dot(lightVector) * (1 - material->reflectivity) +
			material->reflectivity * material->specular * material->get_reflexivity();
	}
	else
	{
		Vector3 direction = ray.direction();
		Color4 texel = this->cubeMap->get_texel(direction);
		return Vector3(texel.b, texel.g, texel.r);
	}
}

Vector3 Scene::genOmega(Vector3 normal, float r1, float r2)
{
	double sqrt1 = sqrt(1 - SQR(r2));
	double x = cos(PI_2 * r1) * sqrt1;
	double y = sin(PI_2 * r1) * sqrt1;
	return Vector3(x, y, r2);
}

float Scene::fr(Vector3 omega_i, Vector3 omega_o)
{
	return 0.3 / M_PI;
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
