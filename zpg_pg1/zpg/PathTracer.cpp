#include "stdafx.h"


PathTracer::PathTracer(RayResolver resolver, const RTCScene& scene, std::unique_ptr<Sampler> sampler): Tracer(resolver, scene)
{
	this->sampler = std::move(sampler);
}

Color4 PathTracer::trace(Ray& ray, uint nest)
{
	return _trace(ray, nest, 1.0f);
}


Color4 PathTracer::_trace(Ray& ray, uint nest, float beforeIor)
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
		float reflectivityChance = 0.3f;
		if (load.material->get_name() == "green_plastic_transparent")
		{
			float n1 = beforeIor;
			float n2 = load.material->ior;
			if (beforeIor == 1.5f)
			{
				n2 = 1.0;
			}

			std::tuple<Vector3, Vector3, float, float> reverseSchnellAndFresnel =
				Tracer::reverse_schnell_and_fresnel(n1, n2, load.normal, rd);
			float reflectivity = std::get<2>(reverseSchnellAndFresnel);
			float transmitivity = std::get<3>(reverseSchnellAndFresnel);

			if (transmitivity > random())
			{
				Vector3 outcomeDir = std::get<0>(reverseSchnellAndFresnel);
				Ray outcomeRay = Ray(load.position, outcomeDir, 0.01f);
				Color4 indirectColor = _trace(outcomeRay, nest - 1, n2);
				float dot = load.normal.dot(-outcomeDir);
				return (indirectColor * load.diffuse_color);//l * transmitivity;//; * dot * transmitivity * load.material->diffuse;
			}
			else
			{
				std::tuple<Color4, Vector3> sample = reflectionSampler.sample(rd, load.normal, load.specular_color);
				Ray incomingRay = Ray(load.position, std::get<1>(sample), 0.01f);
				Color4 indirectColor = _trace(incomingRay, nest - 1, n2);
				//Color4 Le = load.ambient_color;
				return (indirectColor * std::get<0>(sample));// * reflectivity; //*reflectivity;
			}
		}
		else
		{
			if (reflectivityChance > random())
			{
				currentSampler = &reflectionSampler;
			}

		}

		std::tuple<Color4, Vector3> sample = currentSampler->sample(rd, load.normal, load.diffuse_color);
		Ray incomingRay = Ray(load.position, std::get<1>(sample), 0.01f);
		Color4 Li = _trace(incomingRay, nest - 1, load.material->ior);
		Color4 brdfColor = std::get<0>(sample);
		return (Li * brdfColor);
		//incoming, ale je opacne, takze je vlastne ten co prichazi obraceny
		//Vector3 lightVector = load.light_vector();
		//const Color4 directColor = 
		//isInShadow(load.position, lightVector) * load.diffuse_color * lightVector.normalize().dot(load.normal);
		// na important tracing normal.dot(uhel)
		//na reflektivini povrch, udelat paprsek s pdf 1, kter� bude p�esn� opa�n�
		//imporant sampling je vyorec 35., a je�t� mus� b�t normal.dot(rd) > random(0,1)
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
