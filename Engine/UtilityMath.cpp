   #include "EnginePch.h"

#include "UtilityMath.h"
#include "UtilityClock.h"

Matrix  Math::sm_IdentityMatrix( Vector(1, 0, 0, 0),
                              Vector(0, 1, 0, 0),
                              Vector(0, 0, 1, 0),
                              Vector(0, 0, 0, 1) );
                              
Matrix  Math::sm_ZeroMatrix( 0 );

Vector  Math::sm_ZeroVector( 0, 0, 0, 0 );
Vector  Math::sm_OneVector( 1, 1, 1, 1 );
Vector2 Math::sm_ZeroVector2( 0, 0 );
Vector2 Math::sm_OneVector2( 1, 1 );
Quaternion Math::sm_IdentityQuaternion( 0, 0, 0, 1 );

float   Math::sm_PI = 3.1415926535897932384626433832795f;

Matrix2x2 Math::sm_IdentityMatrix2x2( Vector2(1, 0), Vector2(0, 1) ); 

Transform Math::sm_IdentityTransform( Vector(0, 0, 1), Vector(0, 1, 0), Vector(0, 0, 0) );

Transform::Transform(
   const Matrix &matrix
)
{
   Vector right, up, look, translation;

   matrix.GetRight( &right );      
   scale.x = Math::Normalize( &right, right );

   matrix.GetUp( &up );      
   scale.y = Math::Normalize( &up, up );

   matrix.GetLook( &look );      
   scale.z = Math::Normalize( &look, look );

   matrix.GetTranslation( &translation );      
   scale.w = translation.w;

   transform.SetRight( right );
   transform.SetUp( up );
   transform.SetLook( look );
   transform.SetTranslation( translation );
}

void Math::Initialize( void )
{
   srand( (uint32) time(0) );
}
