#include "stdafx.h"


PathTracer::PathTracer(RayResolver resolver, const RTCScene& scene): Tracer(resolver, scene)
{
}

Color4 PathTracer::trace(Ray& ray, uint nest)
{
	Color4 color_sum = Color4();
	int hits = 10;
	float hit_scale = 1.0f / hits;
	for (int i = 0; i < hits; ++i)
	{
		Ray ray_copy = ray;
		//M_PI je zde pdf
		color_sum += _trace(ray_copy, nest);
	}
	return color_sum * hit_scale;
}


Color4 PathTracer::_trace(Ray& ray, uint nest)
{
	rtcIntersect(scene, ray);
	RayPayload load = getRayPayload(ray);
	if (nest == 0)
	{
		return Color4();
	}
	if (ray.isCollided())
	{
		Vector3 rd = ray.direction().normalize();
		Vector3 omegaOut = (-rd);
		float cosOoN = omegaOut.dot(load.normal);

		Vector3 omega_i = random_sphere_direction().normalize();
		if (omega_i.dot(load.normal) < 0) //opacna normal
		{
			omega_i = -omega_i;
		}
		//vysledek integralu
		Ray incomingRay = Ray(load.position, omega_i, 0.01f); //incoming, ale je opacne, takze je vlastne ten co prichazi obraceny
		Vector3 lightVector = load.light_vector();
		const Color4 directColor = isInShadow(load.position, lightVector) * load.diffuse_color * lightVector.normalize().dot(load.normal);

		Color4 indirectColor =
			cosOoN * _trace(incomingRay, nest - 1) * load.material->get_reflexivity();

		Color4 Li = (directColor + indirectColor);
		Color4 Le = load.ambient_color;
		return Le + Li * fr(omegaOut, omega_i) * (1.0f / pdf());
	}
	else
	{
		//Li
		return load.background_color;
	}
}

PathTracer::~PathTracer()
{
}

float PathTracer::random()
{
	return distribution(generator);
}

Vector3 PathTracer::random_sphere_direction()
{
	float r1 = random();
	float r2 = random();
	double pi2r1 = PI_2 * r1;
	float sqrtf = std::sqrt(1 - SQR(r2));
	double x = cos(pi2r1) * sqrtf;
	double y = sin(pi2r1) * sqrtf;
	float z = r2;
	return Vector3(x, y, z);
}

float PathTracer::pdf() const
{
	return 1.0f / PI_2;
}

float PathTracer::fr(const Vector3& omega_out, const Vector3& omega_in) const
{
	return 0.3 / M_PI;
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