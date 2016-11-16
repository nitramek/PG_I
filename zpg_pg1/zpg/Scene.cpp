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

			//v odmocnine je totalni odraz, tzn. R = 1
			//svetlo je to odkud prijimame paprsek tzn. v tomhle kode je to refract vector


			//renderovaci rovnice
			//svitivost pod uhlem omega o = Le(omega_o) + integral skrze hemisferu(Li(omega_i)*f_r(omega_o, omega_i) * (n * omega_i)(
			//soucet toku prenasoboene odrazivosti a prenasobena nìèím jako cos)
			//omega o - odrazena svitivost
			//omega i - incoming uhel
			//fr 4d funkce - bidirectional distribution funkction - funkce odrazovosti
			//pro vypocet integralu použít monte carlo a dosadit za xi = omega i
			//pro zacatek staci Li = 1, pozdeji vysledek z CubeMapy
			//fr pro lamberutv povrch bude fr = albedo / PI albedo..odraziovost materialu v <0,1>
			//omega i = smerovy vektor
			//rekurze az bude Le > 0
			//vypocet aspoò 10 prvku, klasicky vezmeme
			//vice vzorku pro jeden pixel tìsnì vedle posunutue - prusecik, generovani omega i, 
			//poslani paprsku ve smeru omega, omezeni rekurze, pak se vrátí 0, pro nas pripad Le = 1
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
			//vypocitame hodnoty ve funkci, kterou aproximujeme na obdelnik a pote staci udìlat obsah obdelniku
			//reseni integralu od a do b f(x)~=  (b-a) * prumer f (1/N) * (suma f(i) podle N) - 
			// 1/N * suma skrze N (fi(xi) / pdf(xi) 
			//diky 1 / pdf(xi) - castejsi vzorky budou mit mensi hodnotu, mene castejsi vyssi
			//- pdf je pravdepodobnost vyskytu xi, pro uniformni pripad (1 / (b - a))

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
			refractedColor * diffuse_color * normal.DotProduct(lightVector) * (1 - material->reflectivity) +
			material->reflectivity *  material->specular * material->get_reflexivity();
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
	cv::Mat lambertImg(height, width, CV_32FC3);

	auto start = std::chrono::system_clock::now();

	int nest = 3;

	for (int row = 0; row < lambertImg.rows; row++)
	{
#pragma omp parallel for
		for (int col = 0; col < lambertImg.cols; col++)
		{
			Ray ray = camera->GenerateRay(col, row);
			ray.tnear = 0.01;
			cv::Vec3f color(0.f);
			for (int i = 0; i < 10; ++i)
			{
				Ray copy = ray;
				color += this->radiosity(copy, nest).toCV();
			}
			lambertImg.at<cv::Vec3f>(row, col) = (1.0f / 10.f) * color;
			
		}
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	printf("Tracing for depth %d, took %f s\n", nest, diff.count());
	cv::namedWindow("Phong", CV_WINDOW_AUTOSIZE);
	cv::imshow("Phong", lambertImg);
	cv::waitKey(0);
}
