#ifndef VECTOR3_H_
#define VECTOR3_H_
#include "stdafx.h"

/*! \struct Vector3
\brief Trojrozmìrný (3D) vektor.

Implementace tøísložkového reálného vektoru podporující základní
matematické operace.

\note
Vektor se považuje za sloupcový, pøestože je v komentáøích pro jednoduchost
uvádìn jako øádkový.

\code{.cpp}
Vector3 v = Vector3( 2.0f, 4.5f, 7.8f );
v.Normalize();
\endcode

\author Tomáš Fabián
\version 0.95
\date 2007-2015
*/
struct /*ALIGN*/ Vector3
{
public:
	union	// anonymní unie
	{
		struct
		{
			float x; /*!< První složka vektoru. */
			float y; /*!< Druhá složka vektoru. */
			float z; /*!< Tøetí složka vektoru. */
		};

		float data[3]; /*!< Pole složek vektoru. */
	};

	//! Výchozí konstruktor.
	/*!
	Inicializuje všechny složky vektoru na hodnotu nula,
	\f$\mathbf{v}=\mathbf{0}\f$.
	*/
	Vector3() : x( 0 ), y( 0 ), z( 0 ) { }	

	/*
		Vsechny slozky stejne velke
	*/
	Vector3(const float x) :x(x), y(x), z(x) {}
	//! Obecný konstruktor.
	/*!
	Inicializuje složky vektoru podle zadaných hodnot parametrù,
	\f$\mathbf{v}=(x,y,z)\f$.

	\param x první složka vektoru.
	\param y druhá složka vektoru.
	\param z tøetí složka vektoru.
	*/
	Vector3( const float x, const float y, const float z ) : x( x ), y( y ), z( z ) { }

	//! Konstruktor z pole.
	/*!
	Inicializuje složky vektoru podle zadaných hodnot pole,

	\param v ukazatel na první složka vektoru.	
	*/
	Vector3( const float * v );


	//! L2-norma vektoru.
	/*!
	\return x Hodnotu \f$\mathbf{||v||}=\sqrt{x^2+y^2+z^2}\f$.
	*/
	float L2Norm() const;

	//! Druhá mocnina L2-normy vektoru.
	/*!
	\return Hodnotu \f$\mathbf{||v||^2}=x^2+y^2+z^2\f$.
	*/
	float SqrL2Norm() const;

	//! Normalizace vektoru.
	/*!
	Po provedení operace bude mít vektor jednotkovou délku.
	Vektor vraceny je tento vektor samotny, je to kvuli retezeni
	*/
	Vector3& normalize();

	Vector3 reflect(const Vector3& reflectBy) const;

	//! Vektorový souèin.
	/*!
	\param v vektor \f$\mathbf{v}\f$.

	\return Vektor \f$(\mathbf{u}_x \mathbf{v}_z - \mathbf{u}_z \mathbf{v}_y,
	\mathbf{u}_z \mathbf{v}_x - \mathbf{u}_x \mathbf{v}_z,
	\mathbf{u}_x \mathbf{v}_y - \mathbf{u}_y \mathbf{v}_x)\f$.
	*/
	Vector3 CrossProduct( const Vector3 & v ) const;	

	Vector3 Abs() const;

	Vector3 Max( const float a = 0 ) const;

	//! Skalární souèin.
	/*!		
	\return Hodnotu \f$\mathbf{u}_x \mathbf{v}_x + \mathbf{u}_y \mathbf{v}_y + \mathbf{u}_z \mathbf{v}_z)\f$.
	*/
	float DotProduct( const Vector3 & v ) const;

	/**
	*	Oba vektory normalizuje a vrati skalarni soucin
	*/
	float cosBetween(Vector3  v);

	//! Rotace.
	/*!		
	\return Vektor orotovaný kolem poèátku o \f$\phi\f$ radiánù v kladném smyslu.
	*/
	//Vector2 Rotate( const float phi );

	//! Index nejvìtší složky vektoru.
	/*!
	\param absolute_value index bude urèen podle absolutní hodnoty složky

	\return Index nejvìtší složky vektoru.
	*/
	char LargestComponent( const bool absolute_value = false );

	Direction LargestComponentSigned();

	void Print() const;

	Vector3 refract(float n1, float n2, Vector3 normal);

	cv::Vec3f toCV() const;

	// --- operátory ------

	friend Vector3 operator-( const Vector3 & v );

	friend Vector3 operator+( const Vector3 & u, const Vector3 & v );
	friend Vector3 operator-( const Vector3 & u, const Vector3 & v );

	friend Vector3 operator*( const Vector3 & v, const float a );	
	friend Vector3 operator*( const float a, const Vector3 & v );
	friend Vector3 operator*( const Vector3 & u, const Vector3 & v );

	friend Vector3 operator/( const Vector3 & v, const float a );

	friend void operator+=( Vector3 & u, const Vector3 & v );
	friend void operator-=( Vector3 & u, const Vector3 & v );
	friend void operator*=( Vector3 & v, const float a );
	friend void operator/=( Vector3 & v, const float a );	

	// --- konverze ------

};

#endif
