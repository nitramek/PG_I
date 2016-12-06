#include "stdafx.h"


PathTracer::PathTracer(RayResolver resolver, const RTCScene& scene, std::unique_ptr<Sampler> sampler): Tracer(resolver, scene)
{

	this->sampler = std::move(sampler);
}

Color4 PathTracer::trace(Ray& ray, uint nest)
{
	return _trace(ray, nest);
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
		//Vector3 omega_i = sampler->next_direction(, omegaOut);// random_sphere_direction().normalize();
		//float dot = omega_i.dot(load.normal);
		Sampler* currentSampler = this->sampler.get();
		//random() > 0.9
		if(load.material->get_name() == "green_plastic_transparent")
		{
			currentSampler = &reflectionSampler;
		}

		std::tuple<Color4, Vector3> sample = currentSampler->sample(rd, load.normal, load.diffuse_color);
		//vysledek integralu
		Ray incomingRay = Ray(load.position, std::get<1>(sample), 0.01f);
		//incoming, ale je opacne, takze je vlastne ten co prichazi obraceny
		//Vector3 lightVector = load.light_vector();
		//const Color4 directColor = 
		//isInShadow(load.position, lightVector) * load.diffuse_color * lightVector.normalize().dot(load.normal);
		// na important tracing normal.dot(uhel)
		//na reflektivini povrch, udelat paprsek s pdf 1, který bude pøesnì opaèný
		//imporant sampling je vyorec 35., a ještì musí být normal.dot(rd) > random(0,1)
		Color4 indirectColor = _trace(incomingRay, nest - 1);
		//* load.diffuse_color
		Color4 Li = (indirectColor); //directColor
		//Color4 Le = load.ambient_color;
		return (Li * std::get<0>(sample));
	}
	else
	{
		//Li//
		return load.background_color;
		return Color4(1.0, 1.0);
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

Color4 PathTracer::fr(Color4 diffuseColor, const Vector3& omega_out, const Vector3& omega_in) const
{
	return Color4(0.6f / M_PI);
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
