#include "stdafx.h"

Vector3::Vector3( const float * v )
{
	assert( v != NULL );

	x = v[0];
	y = v[1];
	z = v[2];
}

float Vector3::L2Norm() const
{
	return sqrt( SQR( x ) + SQR( y ) + SQR( z ) );
}

float Vector3::SqrL2Norm() const
{
	return SQR( x ) + SQR( y ) + SQR( z );
}

Vector3& Vector3::normalize() 
{
	const float norm = this->SqrL2Norm();

	if ( norm != 0 )
	{
		const float rn = 1 / sqrt( norm );

		this->x *= rn;
		this->y *= rn;
		this->z *= rn;
	}
	return *this;
}

Vector3 Vector3::reflect(const Vector3 & normal) const{

	Vector3 thisV = *this;
	thisV.normalize();
	thisV = -thisV;
	return 2 * (thisV.dot(normal)) * normal - thisV;
}

Vector3 Vector3::CrossProduct( const Vector3 & v ) const
{
	return Vector3(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x );
}

Vector3 Vector3::Abs() const
{
	return Vector3( abs( x ), abs( y ), abs( z ) );		
}

Vector3 Vector3::Max( const float a ) const
{
	return Vector3( MAX( x, a ), MAX( y, a ), MAX( z, a ) );
}

float Vector3::dot( const Vector3 & v ) const
{
	return x * v.x + y * v.y + z * v.z;
}

float Vector3::cosBetween(Vector3  v)
{

	return this->normalize().dot(v.normalize());
}

/*Vector3 Vector3::Rotate( const float phi )
{
	const float cos_phi = cos( phi );
	const float sin_phi = sin( phi );

	return Vector2( x * cos_phi + y * sin_phi,
		-x * sin_phi + y * cos_phi );
}*/

char Vector3::LargestComponent( const bool absolute_value )
{
	const Vector3 d = ( absolute_value )? Vector3( abs( x ), abs( y ), abs( z ) ) : *this;

	if ( d.x > d.y )
	{
		if ( d.x > d.z )
		{			
			return 0 ;
		}
		else
		{
			return 2;
		}
	}
	else
	{
		if ( d.y > d.z )
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	return -1;
}
Direction Vector3::LargestComponentSigned() {
	const Vector3 d = Vector3(abs(x), abs(y), abs(z));

	if (d.x > d.y)
	{
		if (d.x > d.z)
		{
			if (this->x >= 0) {
				return Direction::FRONT;
			}
			else {
				return Direction::BACK;
			}
		}
		else
		{
			if (this->z >= 0) {
				return Direction::TOP;
			}
			else {
				return Direction::BOTTOM;
			}
		}
	}
	else
	{
		if (d.y > d.z)
		{
			if (this->y >= 0) {
				return Direction::RIGHT;
			}
			else {
				return Direction::LEFT;
			}
			
		}
		else
		{
			if (this->z >= 0) {
				return Direction::TOP;
			}
			else {
				return Direction::BOTTOM;
			}
		}
	}
}
void Vector3::Print() const
{
	printf( "(%0.3f, %0.3f, %0.3f)\n", x, y, z ); 
	//printf( "_point %0.3f,%0.3f,%0.3f\n", x, y, z );
}

Vector3 Vector3::refract(float n1, float n2, Vector3 normal)
{
	Vector3 l = (*this);
	Vector3 n = normal;
	float c = (-n).cosBetween(l);
	//if(c < 0)
	//{
	//	c = (n).cosBetween(l);
	//}
	float r = n1 / n2;/*0.9f;*///;1.f / 1.5f; // n1/n2	
	float fi2 = std::sqrt(1 - std::pow(r, 2) * (1 - std::pow(c, 2)));
	return r * l + (r * c - fi2) * n;

}

cv::Vec3f Vector3::toCV() const
{
	return cv::Vec3f(this->z, this->y, this->x);
}

// --- operátory ------

Vector3 operator-( const Vector3 & v )
{
	return Vector3( -v.x, -v.y, -v.z );
}

Vector3 operator+( const Vector3 & u, const Vector3 & v )
{
	return Vector3( u.x + v.x, u.y + v.y, u.z + v.z );
}

Vector3 operator-( const Vector3 & u, const Vector3 & v )
{
	return Vector3( u.x - v.x, u.y - v.y, u.z - v.z );
}

Vector3 operator*( const Vector3 & v, const float a )
{
	return Vector3( a * v.x, a * v.y, a * v.z );
}

Vector3 operator*( const float a, const Vector3 & v )
{
	return Vector3( a * v.x, a * v.y, a * v.z  ); 		
}

Vector3 operator*( const Vector3 & u, const Vector3 & v )
{
	return Vector3( u.x * v.x, u.y * v.y, u.z * v.z );
}

Vector3 operator/( const Vector3 & v, const float a )
{
	return v * ( 1 / a );
}

void operator+=( Vector3 & u, const Vector3 & v )
{
	u.x += v.x;
	u.y += v.y;	
	u.z += v.z;	
}

void operator-=( Vector3 & u, const Vector3 & v )
{
	u.x -= v.x;
	u.y -= v.y;
	u.z -= v.z;
}

void operator*=( Vector3 & v, const float a )
{
	v.x *= a;
	v.y *= a;
	v.z *= a;
}

void operator/=( Vector3 & v, const float a )
{
	const float r = 1 / a;

	v.x *= r;
	v.y *= r;
	v.z *= r;
}

bool operator==(Vector3& v1, Vector3& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}


