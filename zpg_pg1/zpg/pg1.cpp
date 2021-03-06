#include "stdafx.h"

void rtc_error_function( const RTCError code, const char * str )
{
	printf( "ERROR in Embree: %s\n", str );
	exit( 1 );
}

RTCError check_rtc_or_die( RTCDevice & device )
{
	const RTCError error = rtcDeviceGetError( device );

	if ( error != RTC_NO_ERROR )
	{
		printf( "ERROR in Embree: " );

		switch ( error )
		{
		case RTC_UNKNOWN_ERROR:
			printf( "An unknown error has occurred." );
			break;

		case RTC_INVALID_ARGUMENT:
			printf( "An invalid argument was specified." );
			break;

		case RTC_INVALID_OPERATION:
			printf( "The operation is not allowed for the specified object." );
			break;

		case RTC_OUT_OF_MEMORY:
			printf( "There is not enough memory left to complete the operation." );
			break;

		case RTC_UNSUPPORTED_CPU:
			printf( "The CPU is not supported as it does not support SSE2." );
			break;

		case RTC_CANCELLED:
			printf( "The operation got cancelled by an Memory Monitor Callback or Progress Monitor Callback function." );
			break;
		}

		fflush( stdout );
		exit( 1 );
	}

	return error;
}

// struktury pro ukládání dat pro Embree
namespace embree_structs
{
	struct Vertex { float x, y, z, a; };
	typedef Vertex Normal;
	struct Triangle { int v0, v1, v2; };
};

void filter_intersection( void * user_ptr, Ray & ray )
{
	/*  All hit information inside the ray is valid.
		The filter function can reject a hit by setting the geomID member of the ray to
	    RTC_INVALID_GEOMETRY_ID, otherwise the hit is accepted.The filter function is not
		allowed to modify the ray input data (org, dir, tnear, tfar), but can modify
		the hit data of the ray( u, v, Ng, geomID, primID ). */

	Surface * surface = reinterpret_cast<Surface *>( user_ptr );	
	printf( "intersection of: %s, ", surface->get_name().c_str() );
	const Vector3 p = ray.eval( ray.tfar );
	printf( "at: %0.3f (%0.3f, %0.3f, %0.3f)\n", ray.tfar, p.x, p.y, p.z );
	
	ray.geomID = RTC_INVALID_GEOMETRY_ID; // reject hit
}

void filter_occlusion( void * user_ptr, Ray & ray )
{
	/*  All hit information inside the ray is valid.
	The filter function can reject a hit by setting the geomID member of the ray to
	RTC_INVALID_GEOMETRY_ID, otherwise the hit is accepted.The filter function is not
	allowed to modify the ray input data (org, dir, tnear, tfar), but can modify
	the hit data of the ray( u, v, Ng, geomID, primID ). */

	Surface * surface = reinterpret_cast<Surface *>( user_ptr );	
	printf( "occlusion of: %s, ", surface->get_name().c_str() );
	const Vector3 p = ray.eval( ray.tfar );
	printf( "at: %0.3f (%0.3f, %0.3f, %0.3f)\n", ray.tfar, p.x, p.y, p.z );

	ray.geomID = RTC_INVALID_GEOMETRY_ID; // reject hit
}

int test( RTCScene & scene, std::vector<Surface *> & surfaces )
{
	// --- test rtcIntersect -----
	Ray rtc_ray = Ray( Vector3( -1.0f, 0.1f, 0.2f ), Vector3( 2.0f, 0.0f, 0.0f ), 0.0f );		
	//Ray rtc_ray = Ray( Vector3( 4.0f, 0.1f, 0.2f ), Vector3( -1.0f, 0.0f, 0.0f ) );
	//rtc_ray.tnear = 0.6f;
	//rtc_ray.tfar = 2.0f;
	rtcIntersect( scene, rtc_ray ); // find the closest hit of a ray segment with the scene
	// pri filtrovani funkce rtcIntersect jsou pruseciky prochazeny od nejblizsiho k nejvzdalenejsimu
	// u funkce rtcOccluded jsou nalezene pruseciky vraceny v libovolnem poradi

	if ( rtc_ray.geomID != RTC_INVALID_GEOMETRY_ID )
	{
		Surface * surface = surfaces[rtc_ray.geomID];
		Triangle & triangle = surface->get_triangle( rtc_ray.primID );
		//Triangle * triangle2 = &( surface->get_triangles()[rtc_ray.primID] );

		// získání souřadnic průsečíku, normál, texturovacích souřadnic atd.
		const Vector3 p = rtc_ray.eval( rtc_ray.tfar );
		Vector3 geometry_normal = -Vector3( rtc_ray.Ng ); // Ng je nenormalizovaná normála zasaženého trojúhelníka vypočtená nesouhlasně s pravidlem pravé ruky o závitu
		geometry_normal.normalize(); // normála zasaženého trojúhelníka vypočtená souhlasně s pravidlem pravé ruky o závitu
		const Vector3 normal = triangle.normal( rtc_ray.u, rtc_ray.v );
		const Vector2 texture_coord = triangle.texture_coord( rtc_ray.u, rtc_ray.v );

		printf("");
	}

	// --- test rtcOccluded -----
	rtc_ray = Ray( Vector3( -1.0f, 0.1f, 0.2f ), Vector3( 1.0f, 0.0f, 0.0f ) );
	//rtc_ray.tfar = 1.5;	
	rtcOccluded( scene, rtc_ray ); // determining if any hit between a ray segment and the scene exists
	// po volání rtcOccluded je nastavena pouze hodnota geomID, ostatni jsou nezměněny
	if ( rtc_ray.geomID == 0 )
	{
		// neco jsme nekde na zadaném intervalu (tnear, tfar) trefili, ale nevime co ani kde
	}

	return 0;
}


int main( int argc, char * argv[] )
{
	printf( "PG1, (c)2011-2016 Tomas Fabian\n\n" );	

	_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON ); // Flush to Zero, Denormals are Zero mode of the MXCSR
	_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );
	RTCDevice device = rtcNewDevice( NULL ); // musíme vytvořit alespoň jedno Embree zařízení		
	check_rtc_or_die( device ); // ověření úspěšného vytvoření Embree zařízení
	rtcDeviceSetErrorFunction( device, rtc_error_function ); // registrace call-back funkce pro zachytávání chyb v Embree	

	try{
	
		Scene scene(device, 640, 480, "PT", 5, 5, std::make_unique<ImportantSampler>());
		scene.drawIn("ImporantSampling");

		//Scene uniform(device, 640, 480, "PT", 5, 10, std::make_unique<UniformSampler>());
		//uniform.drawIn("Uniform");

		//Scene rayTracing(device, 640, 480, "RT", 5, 4, nullptr);
		//rayTracing.drawIn("rayTracing");

		cv::waitKey(0);
	}
	catch (std::exception& e) 
	{
		std::cout << e.what() << std::endl;
	}

	rtcDeleteDevice( device ); // Embree zařízení musíme také uvolnit před ukončením aplikace

	return 0;
}
