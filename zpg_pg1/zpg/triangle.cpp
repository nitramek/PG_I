#include "stdafx.h"

Triangle::Triangle( const Vertex & v0, const Vertex & v1, const Vertex & v2, Surface * surface )
{
	vertices_[0] = v0;
	vertices_[1] = v1;
	vertices_[2] = v2;

	// v�po�et tangenty
	const Vector3 e1 = v1.position - v0.position;
	const Vector3 e2 = v2.position - v0.position;
	const Vector2 duv1 = v1.texture_coords[0] - v0.texture_coords[0];
	const Vector2 duv2 = v2.texture_coords[0] - v0.texture_coords[0];
	const float rd = 1 / ( duv1.x * duv2.y - duv1.y * duv2.x );
	vertices_[0].tangent = ( e1 * duv2.y - e2 * duv1.y ) * rd;		
	vertices_[0].tangent.normalize(); // t�m odpad� nutnost po��tat s rd
	//vertices_[0].tangent *= -1;
	vertices_[1].tangent = vertices_[0].tangent;
	vertices_[2].tangent = vertices_[0].tangent;
	Vector3 bitangent = ( e2 * duv1.x - e1 * duv2.x ) * rd;
	bitangent.normalize();
	if ( fabs( vertices_[0].tangent.L2Norm() - 1 ) > 0.0001 )
	{
		vertices_[0].tangent.Print();
	}

	// ukazatel na surface schov�me (!pokud se tam vleze!) do paddingu prvn�ho vertexu
	*reinterpret_cast<Surface **>( &vertices_[0].pad ) = surface;	
}

Vertex Triangle::vertex( const int i )
{
	return vertices_[i];
}

Vector3 Triangle::normal( const Vector3 & p, Vector2 * texture_coord )
{		
	Vector3 v0 = vertices_[2].position - vertices_[0].position;
	Vector3 v1 = vertices_[1].position - vertices_[0].position;
	Vector3 v2 = p - vertices_[0].position;

	float dot00 = v0.DotProduct( v0 );
	float dot01 = v0.DotProduct( v1 );
	float dot02 = v0.DotProduct( v2 );
	float dot11 = v1.DotProduct( v1 );
	float dot12 = v1.DotProduct( v2 );

	float inv_denom = 1 / ( dot00 * dot11 - dot01 * dot01 );
	float u = ( dot11 * dot02 - dot01 * dot12 ) * inv_denom;
	float v = ( dot00 * dot12 - dot01 * dot02 ) * inv_denom;

	/*Vector3 n = per_vertex_normals[0] +
	u * ( per_vertex_normals[1] - per_vertex_normals[0] ) +
	v * ( per_vertex_normals[2] - per_vertex_normals[0] );
	n.Normalize();*/

	Vector3 normal = u * vertices_[2].normal +
		v * vertices_[1].normal +
		( 1 - u - v ) * vertices_[0].normal;
	normal.normalize();

	if ( texture_coord != NULL )
	{
		*texture_coord = u * vertices_[2].texture_coords[0] +
		v * vertices_[1].texture_coords[0] +
		( 1 - u - v ) * vertices_[0].texture_coords[0];
	}

	return normal;	
}

Vector3 Triangle::normal( const float u, const float v )
{
	Vector3 normal = u * vertices_[1].normal +
		v * vertices_[2].normal +
		( 1.0f - u - v ) * vertices_[0].normal;
	normal.normalize();

	return normal;
}

Vector2 Triangle::texture_coord( const float u, const float v )
{
	Vector2 texture_coord = u * vertices_[1].texture_coords[0] +
		v * vertices_[2].texture_coords[0] +
		( 1.0f - u - v ) * vertices_[0].texture_coords[0];

	return texture_coord;
}

Vector3 Triangle::baricenter()
{
	return ( vertices_[0].position + vertices_[1].position + vertices_[2].position ) / 3;
}

Surface * Triangle::surface()
{	
	return *reinterpret_cast<Surface **>( vertices_[0].pad ); // FIX: chyb� verze pro 64bit
}
