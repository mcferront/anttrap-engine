
void Matrix::SetOrientation(
   const Vector &look,
   const Vector &up
)
{
   Vector newUp, right;

   Math::CrossProduct( &right, up, look );
   Math::Normalize( &right, right );

   Math::CrossProduct( &newUp, look, right );
   Math::Normalize( &newUp, newUp );

   Math::CrossProduct( &right, newUp, look );

   SetRight( right );
   SetUp( newUp );
   SetLook( look );
}

void Matrix::SetOrientation(
   const Matrix &matrix
)
{
   SetRight( matrix.row[ 0 ] );
   SetUp( matrix.row[ 1 ] );
   SetLook( matrix.row[ 2 ] );
}

void Matrix::SetOrientation(
   const Quaternion &quaternion
)
{
   float x = 2.0f * quaternion.quaternion.x;
	float y = 2.0f * quaternion.quaternion.y;
	float z = 2.0f * quaternion.quaternion.z;

	float wx = x * quaternion.quaternion.w;
	float wy = y * quaternion.quaternion.w;
	float wz = z * quaternion.quaternion.w;
	float xx = x * quaternion.quaternion.x;
	float xy = y * quaternion.quaternion.x;
	float xz = z * quaternion.quaternion.x;
	float yy = y * quaternion.quaternion.y;
	float yz = z * quaternion.quaternion.y;
	float zz = z * quaternion.quaternion.z;

	SetRight( Vector(1.0f - (yy + zz), xy + wz, xz - wy) );
	SetUp( Vector(xy - wz, 1.0f - (xx + zz), yz + wx) );
	SetLook( Vector(xz + wy, yz - wx, 1.0f - (xx + yy)) );
}

void Transform::SetScale(
   const Vector &_scale
) 
{
   scale = _scale;
}

void Transform::SetOrientation(
   const Vector &look,
   const Vector &up
)
{
   transform.SetOrientation( look, up );
}

void Transform::SetOrientation(
   const Matrix2x2 &matrix
)
{
   Vector2 right, up;
   matrix.GetRight( &right );
   matrix.GetUp( &up );

   SetRight( right );
   SetUp( up );
   SetLook( Vector(0, 0, 1) );
}

void Transform::SetOrientation(
   const Transform &_transform
)
{
   transform.SetOrientation( _transform.transform );
}

void Transform::SetOrientation(
   const Quaternion &quaternion
)
{
   transform.SetOrientation( quaternion );
}

Matrix Transform::ToMatrix(
   bool applyScaling
) const
{
   if ( true == applyScaling && true == IsScaled( ) )
   {
      Matrix matrix;
      Math::Scale( &matrix, scale );

      Math::Multiply( &matrix, matrix, transform );
   
      return matrix;
   }
   else
   {
      return transform;
   }
}

Transform Transform::GetInverse() const
{ 
    Transform transform;
    Math::Invert( &transform, *this );
    
    return transform;
}

void Transform::SetIdentity()
{
    *this = Math::IdentityTransform();
}

Transform Transform::operator * (const Transform &rhs) const
{
    Transform t;
    Math::Multiply( &t, *this, rhs );

    return t;
}

void Quaternion::Set(
   const Transform &_transform
)
{
   // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
   // article "Quaternion Calculus and Fast Animation".
   // modified version of the one found in WildMagic
   float root, trace;

   Matrix transform = _transform.ToMatrix( false );
   Math::Transpose( &transform, transform );

   trace = transform.row[ 0 ].x + transform.row[ 1 ].y + transform.row[ 2 ].z;
   
   if ( trace > 0.0f )
   {
      root = sqrtf( trace + 1.0f );  // 2w
         
      quaternion.w = .5f * root;
         
      root = .5f / root; // 1/(4w)

      quaternion.x = ( transform.row[ 2 ].y - transform.row[ 1 ].z ) * root;
      quaternion.y = ( transform.row[ 0 ].z - transform.row[ 2 ].x ) * root;
      quaternion.z = ( transform.row[ 1 ].x - transform.row[ 0 ].y ) * root;
   }
   else
   {
      // |w| <= 1/2
      int i = 0;
      float *row_i, *row_j, *row_k;
      float *quat;

      int next[ 3 ] = { 1, 2, 0 };

      if ( transform.row[ 1 ].y > transform.row[ 0 ].x )
      {
         i = 1;
      }
      
      row_i = (float *) &transform.row[ i ];

      if ( transform.row[ 2 ].z > row_i[ i ] )
      {
         i = 2;
      }
      
      int j = next[ i ];
      int k = next[ j ];
      
      row_i = (float *) &transform.row[ i ];
      row_j = (float *) &transform.row[ j ];
      row_k = (float *) &transform.row[ k ];

      root = Math::Sqrt( row_i[ i ] - row_j[ j ] - row_k[ k ] + 1.0f );
      
      quat = (float *) &quaternion.x;

      quat[ i ] = .5f * root;
      
      root = .5f / root;

      quaternion.w = ( row_k[ j ] - row_j[ k ] ) * root;
      quat[ j ] = ( row_j[ i ] + row_i[ j ] ) * root;
      quat[ k ] = ( row_k[ i ] + row_i[ k ] ) * root;
   }
}

void Quaternion::Set(
   const Vector &_look,
   const Vector &_up
)
{
   Transform t(_look, _up, Vector(0.0f, 0.0f, 0.0f));
   Set(t);
}

float Math::DegreesToRadians(
   float degrees
)
{
   return (degrees * Math::PI()) / 180.0f;
}

float Math::Rand(
   float minValue, 
   float maxValue
)
{
   return minValue + (rand( ) / (float) RAND_MAX) * (maxValue - minValue);
}

int Math::Rand(
   int minValue, 
   int maxValue
)
{
   if (maxValue == minValue) return minValue;

   return minValue + (rand( ) % (maxValue - minValue));
}

float Math::Cos(
   float radians
)
{
   return cosf( radians );
}

float Math::ACos(
   float radians
)
{
   return acosf( radians );
}

float Math::Sin(
   float radians
)
{
   return sinf( radians );
}

float Math::Sign(
   float value
)
{
   return value >= 0.0f ? 1.0f : -1.0f;
}

float Math::Sign(
   int value
)
{
   return value >= 0 ? 1.0f : -1.0f;
}

void Math::CosSin(
   float *pCos,
   float *pSin,
   float radians
)
{
   *pCos = Math::Cos( radians );
   *pSin = Math::Sin( radians );
}

float Math::Tan(
   float radians
)
{
   return tanf( radians );
}

float Math::ATan(
   float radians
)
{
   return atanf( radians );
}

float Math::Sqrt(
   float value
)
{
   return sqrtf( value );
}

int Math::MakePowerOf2(
   int value
)
{
   int power2 = 0;
   int shiftValue = value;

   while ( 0 != (shiftValue = shiftValue >> 1) )
   {
      ++power2;
   }
   
   if ( value != 1 << power2 )
   {
      value = 1 << (power2 + 1);
   }   
   
   return value;
}

float Math::Magnitude(
   const Vector &vector
)
{
   return Math::Sqrt( MagnitudeSq(vector) );
}

float Math::Magnitude4(
   const Vector &vector
)
{
   return Math::Sqrt( MagnitudeSq4(vector) );
}

float Math::Magnitude(
   const Vector2 &vector
)
{
   return Math::Sqrt( MagnitudeSq(vector) );
}

float Math::MagnitudeSq(
   const Vector &vector
)
{
   return DotProduct( vector, vector );
}

float Math::MagnitudeSq4(
   const Vector &vector
)
{
   return DotProduct4( vector, vector );
}

float Math::MagnitudeSq(
   const Vector2 &vector
)
{
   return DotProduct( vector, vector );
}

float Math::DotProduct(
   const Vector &vectorA,
   const Vector &vectorB
)
{
   return vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z;
}

float Math::DotProduct4(
   const Vector &vectorA,
   const Vector &vectorB
)
{
   return vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z + vectorA.w * vectorB.w;
}

float Math::DotProduct(
   const Vector2 &vectorA,
   const Vector2 &vectorB
)
{
   return vectorA.x * vectorB.x + vectorA.y * vectorB.y;
}

float Math::DotProduct(
   const Quaternion &qA,
   const Quaternion &qB
)
{
   return qA.quaternion.x * qB.quaternion.x + qA.quaternion.y * qB.quaternion.y + qA.quaternion.z * qB.quaternion.z + qA.quaternion.w * qB.quaternion.w;
}

void Math::CrossProduct(
   Vector *pResult,
   const Vector &vectorA,
   const Vector &vectorB
)
{
   Vector result;

   result.x =  vectorA.y * vectorB.z - vectorA.z * vectorB.y;
	result.y =  vectorA.z * vectorB.x - vectorA.x * vectorB.z;
	result.z =  vectorA.x * vectorB.y - vectorA.y * vectorB.x;
   
   *pResult = result;
}

int Math::Abs(
   int value
)
{
   return abs( value );
}

float Math::Abs(
   float value
)
{
   return fabsf( value );
}

void Math::Abs(
   Vector *pResult,
   const Vector &vector
)
{
   pResult->x = Math::Abs( vector.x );
   pResult->y = Math::Abs( vector.y );
   pResult->z = Math::Abs( vector.z );
}

void Math::Ceil(
   Vector *pResult,
   const Vector &vector
)
{
   pResult->x = ceilf( vector.x );
   pResult->y = ceilf( vector.y );
   pResult->z = ceilf( vector.z );
}

void Math::Abs(
   Vector2 *pResult,
   const Vector2 &vector
)
{
   pResult->x = Math::Abs( vector.x );
   pResult->y = Math::Abs( vector.y );
}

int Math::FloatToInt(
   float value
)
{
   return (int) value;
}

bool Math::FloatTest(
  float valueA,
  float valueB,
  float range
)
{
    return Math::Abs(valueA - valueB) <= range;
}

int Math::Min(
   int valueA,
   int valueB
)
{
   return valueA < valueB ? valueA : valueB;
}

int Math::Max(
   int valueA,
   int valueB
)
{
   return valueA > valueB ? valueA : valueB;
}

float Math::Min(
   float valueA,
   float valueB
)
{
   return valueA < valueB ? valueA : valueB;
}

float Math::Max(
   float valueA,
   float valueB
)
{
   return valueA > valueB ? valueA : valueB;
}

double Math::Min(
   double valueA,
   double valueB
)
{
   return valueA < valueB ? valueA : valueB;
}

double Math::Max(
   double valueA,
   double valueB
)
{
   return valueA > valueB ? valueA : valueB;
}

int Math::Clamp(
   int value,
   int minValue,
   int maxValue
)
{
   int returnValue = Math::Min( value, maxValue );
   return Math::Max( returnValue, minValue );   
}

float Math::Clamp(
   float value,
   float minValue,
   float maxValue
)
{
   float returnValue = Math::Min( value, maxValue );
   return Math::Max( returnValue, minValue );
}   

float Math::Lerp(
   float startValue,
   float endValue,
   float time
)
{
   return startValue + ( endValue - startValue ) * time;
}

void Math::Lerp(
   float *pResult,
   float startValue,
   float endValue,
   float time
)
{
   *pResult = Lerp( startValue, endValue, time );
}

void Math::Lerp(
   Vector *pResult,
   const Vector &startValue,
   const Vector &endValue,
   float time
)
{
   Vector result;
   result = (endValue - startValue) * time + startValue;

   *pResult = result;
}

void Math::Lerp(
   Quaternion *pResult,
   const Quaternion startValue,
   const Quaternion endValue,
   float time
)
{
   *pResult = startValue * (1.0f - time);
   *pResult += endValue * time;
   Math::Normalize( pResult, *pResult );
}

void Math::Slerp(
   Quaternion *pResult,
   const Quaternion startValue,
   const Quaternion endValue,
   float time
)
{
   // from quaternion rotation visualization demo at:
   // http://www.cs.indiana.edu/~hanson/quatvis/QuatRot/QuatRot.c

   float beta;			   // complementary interp parameter
   float theta;			// angle between A and B
   float sin_t, cos_t;	// sine, cosine of theta
   float phi;				// theta plus spins
   int   bflip;			// use negation of B?

   // cosine theta = dot product of A and B
   cos_t = Math::DotProduct(startValue,endValue);

   // if B is on opposite hemisphere from A, use -B instead
   if (cos_t < 0.0) 
   {
      cos_t = -cos_t;
      bflip = 1;
   } 
   else
   {
      bflip = 0;
   }

   // if B is (within precision limits) the same as A,
   // just linear interpolate between A and B.
   if (1.0 - cos_t < 1e-7) 
   {
      beta = 1.0f - time;
   } 
   else // normal case
   {
      theta = Math::ACos(cos_t);
      phi   = theta;
      sin_t = Math::Sin(theta);
      beta  = Math::Sin(theta - time*phi) / sin_t;
      time  = Math::Sin(time*phi) / sin_t;
   }

   if (bflip) time = -time;

   // interpolate
   pResult->quaternion.x = beta*startValue.quaternion.x + time*endValue.quaternion.x;
   pResult->quaternion.y = beta*startValue.quaternion.y + time*endValue.quaternion.y;
   pResult->quaternion.z = beta*startValue.quaternion.z + time*endValue.quaternion.z;
   pResult->quaternion.w = beta*startValue.quaternion.w + time*endValue.quaternion.w;

   Math::Normalize(pResult, *pResult);
}

void Math::Slerp(
   Transform *pResult,
   const Transform startValue,
   const Transform endValue,
   float time
)
{
   Vector vResult, v1, v2;
   Quaternion qResult, q1, q2;

   // lerp orientation
   q1.Set(startValue);
   q2.Set(endValue);
   Slerp(&qResult, q1, q2, time);

   // lerp position
   startValue.GetTranslation(&v1);
   endValue.GetTranslation(&v2);
   Lerp(&vResult, v1, v2, time);

   // put back in transform
   pResult->SetOrientation(qResult);
   pResult->SetTranslation(vResult);
}

void Math::VectorFromARGB(
   Vector *pVector,
   unsigned int argb
)
{
   int b = argb & 0xff;
   int g = (argb >>  8) & 0xff;
   int r = (argb >> 16) & 0xff;
   int a = (argb >> 24) & 0xff;

   pVector->x = r / 255.0f;
   pVector->y = g / 255.0f;
   pVector->z = b / 255.0f;
   pVector->w = a / 255.0f;
}

void Math::Min(
   Vector *pResult,
   const Vector &valueA,
   const Vector &valueB
)
{
   pResult->x = Math::Min( valueA.x, valueB.x );
   pResult->y = Math::Min( valueA.y, valueB.y );
   pResult->z = Math::Min( valueA.z, valueB.z );
}

void Math::Max(
   Vector *pResult,
   const Vector &valueA,
   const Vector &valueB
)
{
   pResult->x = Math::Max( valueA.x, valueB.x );
   pResult->y = Math::Max( valueA.y, valueB.y );
   pResult->z = Math::Max( valueA.z, valueB.z );
}

void Math::Min(
   Vector2 *pResult,
   const Vector2 &valueA,
   const Vector2 &valueB
)
{
   pResult->x = Math::Min( valueA.x, valueB.x );
   pResult->y = Math::Min( valueA.y, valueB.y ); 
}

void Math::Max(
   Vector2 *pResult,
   const Vector2 &valueA,
   const Vector2 &valueB
)
{
   pResult->x = Math::Max( valueA.x, valueB.x );
   pResult->y = Math::Max( valueA.y, valueB.y );
}

float Math::Normalize(
   Vector *pResult,
   const Vector &vector
)
{
   float magnitude = Magnitude( vector );
   
   if ( magnitude > 0.0f )
   {
      *pResult = vector * ( 1.0f / magnitude ); 
   }
   else
   {
      *pResult = Math::ZeroVector();
   }

   return magnitude;
}

float Math::Normalize4(
   Vector *pResult,
   const Vector &vector
)
{
   float magnitude = Magnitude4( vector );
   
   if ( magnitude > 0.0f )
   {
      *pResult = vector * ( 1.0f / magnitude ); 
   }
   else
   {
      *pResult = Math::ZeroVector();
   }

   return magnitude;
}

float Math::NormalizePlane(
   Vector *pResult,
   const Vector &vector
)
{
   //a plane divides x, y, z, w by the magnitude of 
   //the normal only, so don't do Magnitude4
   float magnitude = Magnitude( vector );
   
   if ( magnitude > 0.0f )
   {
      float invMag = 1.0f / magnitude;

      pResult->x = vector.x * invMag;
      pResult->y = vector.y * invMag;
      pResult->z = vector.z * invMag;
      pResult->w = vector.w * invMag;
   }
   else
   {
      pResult->x = 0;
      pResult->y = 0;
      pResult->z = 0;
      pResult->w = vector.w;
   }

   return magnitude;
}

float Math::Normalize(
   Vector2 *pResult,
   const Vector2 &vector
)
{
   float magnitude = Magnitude( vector );
   
   if ( magnitude > 0.0f )
   {
      *pResult = vector * ( 1.0f / magnitude ); 
   }
   else
   {
      *pResult = Math::ZeroVector2();
   }

   return magnitude;
}

float Math::Normalize(
   Quaternion *pResult,
   const Quaternion &quaternion
)
{
   float magnitudeSq = Math::DotProduct4( quaternion.quaternion, quaternion.quaternion );
   
   if ( magnitudeSq > 0.0f )
   {
      float m = Math::Sqrt( magnitudeSq );

      pResult->quaternion = quaternion.quaternion * (1.0f / m);
      return m;
   }
   else
   {
      pResult->quaternion = Math::ZeroVector();  
      return 0.0f;
   }
}

void Math::Transpose(
   Matrix2x2 *pResult,
   const Matrix2x2 &matrix
)
{
   Matrix2x2 result;

   result.row[ 0 ].x = matrix.row[ 0 ].x;
   result.row[ 0 ].y = matrix.row[ 1 ].x;
   result.row[ 1 ].x = matrix.row[ 0 ].y;
   result.row[ 1 ].y = matrix.row[ 1 ].y;
   
   *pResult = result;
}

void Math::Transpose(
   Matrix *pResult,
   const Matrix &matrix
)
{
   Matrix result;

   result.row[ 0 ].x = matrix.row[ 0 ].x;
   result.row[ 0 ].y = matrix.row[ 1 ].x;
   result.row[ 0 ].z = matrix.row[ 2 ].x;
   result.row[ 0 ].w = matrix.row[ 3 ].x;

   result.row[ 1 ].x = matrix.row[ 0 ].y;
   result.row[ 1 ].y = matrix.row[ 1 ].y;
   result.row[ 1 ].z = matrix.row[ 2 ].y;
   result.row[ 1 ].w = matrix.row[ 3 ].y;

   result.row[ 2 ].x = matrix.row[ 0 ].z;
   result.row[ 2 ].y = matrix.row[ 1 ].z;
   result.row[ 2 ].z = matrix.row[ 2 ].z;
   result.row[ 2 ].w = matrix.row[ 3 ].z;

   result.row[ 3 ].x = matrix.row[ 0 ].w;
   result.row[ 3 ].y = matrix.row[ 1 ].w;
   result.row[ 3 ].z = matrix.row[ 2 ].w;
   result.row[ 3 ].w = matrix.row[ 3 ].w;

   *pResult = result;
}

void Math::Invert(
   Matrix *pResult,
   const Matrix &matrix
)
{
   Matrix result;

   const float fa0 = matrix.row[ 0 ].x * matrix.row[ 1 ].y - matrix.row[ 0 ].y * matrix.row[ 1 ].x;
   const float fa1 = matrix.row[ 0 ].x * matrix.row[ 1 ].z - matrix.row[ 0 ].z * matrix.row[ 1 ].x;
   const float fa2 = matrix.row[ 0 ].x * matrix.row[ 1 ].w - matrix.row[ 0 ].w * matrix.row[ 1 ].x;
   const float fa3 = matrix.row[ 0 ].y * matrix.row[ 1 ].z - matrix.row[ 0 ].z * matrix.row[ 1 ].y;
   const float fa4 = matrix.row[ 0 ].y * matrix.row[ 1 ].w - matrix.row[ 0 ].w * matrix.row[ 1 ].y;
   const float fa5 = matrix.row[ 0 ].z * matrix.row[ 1 ].w - matrix.row[ 0 ].w * matrix.row[ 1 ].z;
   const float fb0 = matrix.row[ 2 ].x * matrix.row[ 3 ].y - matrix.row[ 2 ].y * matrix.row[ 3 ].x;
   const float fb1 = matrix.row[ 2 ].x * matrix.row[ 3 ].z - matrix.row[ 2 ].z * matrix.row[ 3 ].x;
   const float fb2 = matrix.row[ 2 ].x * matrix.row[ 3 ].w - matrix.row[ 2 ].w * matrix.row[ 3 ].x;
   const float fb3 = matrix.row[ 2 ].y * matrix.row[ 3 ].z - matrix.row[ 2 ].z * matrix.row[ 3 ].y;
   const float fb4 = matrix.row[ 2 ].y * matrix.row[ 3 ].w - matrix.row[ 2 ].w * matrix.row[ 3 ].y;
   const float fb5 = matrix.row[ 2 ].z * matrix.row[ 3 ].w - matrix.row[ 2 ].w * matrix.row[ 3 ].z;

   const float fdet = fa0 * fb5 - fa1 * fb4 + fa2 * fb3 + fa3 * fb2 - fa4 * fb1 + fa5 * fb0;

   if ( 0 != fdet )
   {
      result.row[ 0 ].x =   matrix.row[ 1 ].y * fb5 - matrix.row[ 1 ].z * fb4 + matrix.row[ 1 ].w * fb3;
      result.row[ 1 ].x = - matrix.row[ 1 ].x * fb5 + matrix.row[ 1 ].z * fb2 - matrix.row[ 1 ].w * fb1;
      result.row[ 2 ].x =   matrix.row[ 1 ].x * fb4 - matrix.row[ 1 ].y * fb2 + matrix.row[ 1 ].w * fb0;
      result.row[ 3 ].x = - matrix.row[ 1 ].x * fb3 + matrix.row[ 1 ].y * fb1 - matrix.row[ 1 ].z * fb0;
      result.row[ 0 ].y = - matrix.row[ 0 ].y * fb5 + matrix.row[ 0 ].z * fb4 - matrix.row[ 0 ].w * fb3;
      result.row[ 1 ].y =   matrix.row[ 0 ].x * fb5 - matrix.row[ 0 ].z * fb2 + matrix.row[ 0 ].w * fb1;
      result.row[ 2 ].y = - matrix.row[ 0 ].x * fb4 + matrix.row[ 0 ].y * fb2 - matrix.row[ 0 ].w * fb0;
      result.row[ 3 ].y =   matrix.row[ 0 ].x * fb3 - matrix.row[ 0 ].y * fb1 + matrix.row[ 0 ].z * fb0;
      result.row[ 0 ].z =   matrix.row[ 3 ].y * fa5 - matrix.row[ 3 ].z * fa4 + matrix.row[ 3 ].w * fa3;
      result.row[ 1 ].z = - matrix.row[ 3 ].x * fa5 + matrix.row[ 3 ].z * fa2 - matrix.row[ 3 ].w * fa1;
      result.row[ 2 ].z =   matrix.row[ 3 ].x * fa4 - matrix.row[ 3 ].y * fa2 + matrix.row[ 3 ].w * fa0;
      result.row[ 3 ].z = - matrix.row[ 3 ].x * fa3 + matrix.row[ 3 ].y * fa1 - matrix.row[ 3 ].z * fa0;
      result.row[ 0 ].w = - matrix.row[ 2 ].y * fa5 + matrix.row[ 2 ].z * fa4 - matrix.row[ 2 ].w * fa3;
      result.row[ 1 ].w =   matrix.row[ 2 ].x * fa5 - matrix.row[ 2 ].z * fa2 + matrix.row[ 2 ].w * fa1;
      result.row[ 2 ].w = - matrix.row[ 2 ].x * fa4 + matrix.row[ 2 ].y * fa2 - matrix.row[ 2 ].w * fa0;
      result.row[ 3 ].w =   matrix.row[ 2 ].x * fa3 - matrix.row[ 2 ].y * fa1 + matrix.row[ 2 ].z * fa0;

      const float invdet = 1.0f / fdet;

      pResult->row[ 0 ].x = result.row[ 0 ].x * invdet;
      pResult->row[ 0 ].y = result.row[ 0 ].y * invdet;
      pResult->row[ 0 ].z = result.row[ 0 ].z * invdet;
      pResult->row[ 0 ].w = result.row[ 0 ].w * invdet;

      pResult->row[ 1 ].x = result.row[ 1 ].x * invdet;
      pResult->row[ 1 ].y = result.row[ 1 ].y * invdet;
      pResult->row[ 1 ].z = result.row[ 1 ].z * invdet;
      pResult->row[ 1 ].w = result.row[ 1 ].w * invdet;

      pResult->row[ 2 ].x = result.row[ 2 ].x * invdet;
      pResult->row[ 2 ].y = result.row[ 2 ].y * invdet;
      pResult->row[ 2 ].z = result.row[ 2 ].z * invdet;
      pResult->row[ 2 ].w = result.row[ 2 ].w * invdet;

      pResult->row[ 3 ].x = result.row[ 3 ].x * invdet;
      pResult->row[ 3 ].y = result.row[ 3 ].y * invdet;
      pResult->row[ 3 ].z = result.row[ 3 ].z * invdet;
      pResult->row[ 3 ].w = result.row[ 3 ].w * invdet;
   }
   else
   {
      pResult->row[ 0 ] = Math::ZeroVector();
      pResult->row[ 1 ] = Math::ZeroVector();
      pResult->row[ 2 ] = Math::ZeroVector();
      pResult->row[ 3 ] = Math::ZeroVector();
   }
}

void Math::Multiply(
   Matrix *pResult,
   const Matrix &matrixA,
   const Matrix &matrixB
)
{
   Matrix result;
   Matrix transpose;

   Math::Transpose( &transpose, matrixB );

   result.row[ 0 ].x = Math::DotProduct4( matrixA.row[ 0 ], transpose.row[ 0 ] );
   result.row[ 0 ].y = Math::DotProduct4( matrixA.row[ 0 ], transpose.row[ 1 ] );
   result.row[ 0 ].z = Math::DotProduct4( matrixA.row[ 0 ], transpose.row[ 2 ] );
   result.row[ 0 ].w = Math::DotProduct4( matrixA.row[ 0 ], transpose.row[ 3 ] );
   
   result.row[ 1 ].x = Math::DotProduct4( matrixA.row[ 1 ], transpose.row[ 0 ] );
   result.row[ 1 ].y = Math::DotProduct4( matrixA.row[ 1 ], transpose.row[ 1 ] );
   result.row[ 1 ].z = Math::DotProduct4( matrixA.row[ 1 ], transpose.row[ 2 ] );
   result.row[ 1 ].w = Math::DotProduct4( matrixA.row[ 1 ], transpose.row[ 3 ] );

   result.row[ 2 ].x = Math::DotProduct4( matrixA.row[ 2 ], transpose.row[ 0 ] );
   result.row[ 2 ].y = Math::DotProduct4( matrixA.row[ 2 ], transpose.row[ 1 ] );
   result.row[ 2 ].z = Math::DotProduct4( matrixA.row[ 2 ], transpose.row[ 2 ] );
   result.row[ 2 ].w = Math::DotProduct4( matrixA.row[ 2 ], transpose.row[ 3 ] );

   result.row[ 3 ].x = Math::DotProduct4( matrixA.row[ 3 ], transpose.row[ 0 ] );
   result.row[ 3 ].y = Math::DotProduct4( matrixA.row[ 3 ], transpose.row[ 1 ] );
   result.row[ 3 ].z = Math::DotProduct4( matrixA.row[ 3 ], transpose.row[ 2 ] );
   result.row[ 3 ].w = Math::DotProduct4( matrixA.row[ 3 ], transpose.row[ 3 ] );

   *pResult = result;
}

void Math::Multiply(
   Matrix2x2 *pResult,
   const Matrix2x2 &matrixA,
   const Matrix2x2 &matrixB
)
{
   Matrix2x2 result;
   Matrix2x2 transpose;

   Math::Transpose( &transpose, matrixB );

   result.row[ 0 ].x = Math::DotProduct( matrixA.row[ 0 ], transpose.row[ 0 ] );
   result.row[ 0 ].y = Math::DotProduct( matrixA.row[ 0 ], transpose.row[ 1 ] );
   result.row[ 1 ].x = Math::DotProduct( matrixA.row[ 1 ], transpose.row[ 0 ] );
   result.row[ 1 ].y = Math::DotProduct( matrixA.row[ 1 ], transpose.row[ 1 ] );

   *pResult = result;
}

void Math::Multiply(
   Transform *pResult,
   const Transform &transformA,
   const Transform &transformB
)
{
   Matrix result;

   Matrix a = transformA.ToMatrix( true );
   Matrix b = transformB.ToMatrix( true );

   Math::Multiply( &result, a, b );

   *pResult = Transform( result );
}

inline void Math::Rotate(
   Vector2 *pResult,
   const Vector2 &vector,
   const Matrix2x2 &matrix
)
{
   Matrix2x2 transpose;
   Vector2 result;

   Math::Transpose( &transpose, matrix );

   result.x = Math::DotProduct( vector, transpose.row[ 0 ] );
   result.y = Math::DotProduct( vector, transpose.row[ 1 ] );
   
   *pResult = result;
}

void Math::Rotate(
   Vector *pResult,
   const Vector &vector,
   const Matrix &matrix
)
{
   Matrix transpose;
   Vector result;

   Math::Transpose( &transpose, matrix );

   result.x = Math::DotProduct( vector, transpose.row[ 0 ] );
   result.y = Math::DotProduct( vector, transpose.row[ 1 ] );
   result.z = Math::DotProduct( vector, transpose.row[ 2 ] );
   result.w = 0.0f;
   
   *pResult = result;
}

void Math::Rotate(
   Vector *pResult,
   const Vector &vector,
   const ::Transform &transform
)
{
   Vector result;
   Matrix matrix = transform.ToMatrix( true );

   Math::Rotate( &result, vector, matrix );
   
   *pResult = result;
}

void Math::Rotate(
   Vector2 *pResult,
   const Vector2 &vector,
   const ::Transform &transform
)
{
   Matrix transpose;
   Vector2 result;

   Math::Transpose( &transpose, transform.ToMatrix(true) );

   result.x = Math::DotProduct( vector, transpose.row[ 0 ] );
   result.y = Math::DotProduct( vector, transpose.row[ 1 ] );
   
   *pResult = result;
}

void Math::TransformPosition(
   Vector *pResult,
   const Vector &vector,
   const Matrix &matrix
)
{
   Matrix transposed;
   Vector result;
   
   //force w to 1 so we get a valid position transform
   //because our math library assumes 0 or 1
   //for most transform / rotation operations
   Vector position = vector;
   position.w = 1.0f;

   Math::Transpose( &transposed, matrix );
   
   Vector row[ 4 ];

   transposed.GetRow( 0, &row[ 0 ] );
   transposed.GetRow( 1, &row[ 1 ] );
   transposed.GetRow( 2, &row[ 2 ] );
   transposed.GetRow( 3, &row[ 3 ] );

   result.x = Math::DotProduct4( position, row[ 0 ] );
   result.y = Math::DotProduct4( position, row[ 1 ] );
   result.z = Math::DotProduct4( position, row[ 2 ] );
   result.w = Math::DotProduct4( position, row[ 3 ] );
   
   *pResult = result;
}

void Math::TransformPosition(
   Vector *pResult,
   const Vector &vector,
   const Transform &transform
)
{
   Matrix matrix = transform.ToMatrix( true );
   TransformPosition( pResult, vector, matrix );
}

void Math::TransformPosition(
   Vector2 *pResult,
   const Vector2 &vector,
   const Transform &transform
)
{
   Matrix transposed;
   Vector2 result;
   
   //force w to 1 so we get a valid position transform
   //because our math library assumes 0 or 1
   //for most transform / rotation operations
   Vector position = vector;
   position.w = 1.0f;

   Math::Transpose( &transposed, transform.ToMatrix(true) );
   
   Vector row[ 2 ];
   transposed.GetRow( 0, &row[ 0 ] );
   transposed.GetRow( 1, &row[ 1 ] );

   result.x = Math::DotProduct4( position, row[ 0 ] );
   result.y = Math::DotProduct4( position, row[ 1 ] );
   
   *pResult = result;
}

void Math::Transform4(
   Vector *pResult,
   const Vector &vector,
   const Matrix &matrix
)
{
   Matrix transposed;
   Vector result;
   
   Math::Transpose( &transposed, matrix );
   
   Vector row[ 4 ];

   transposed.GetRow( 0, &row[ 0 ] );
   transposed.GetRow( 1, &row[ 1 ] );
   transposed.GetRow( 2, &row[ 2 ] );
   transposed.GetRow( 3, &row[ 3 ] );

   result.x = Math::DotProduct4( vector, row[ 0 ] );
   result.y = Math::DotProduct4( vector, row[ 1 ] );
   result.z = Math::DotProduct4( vector, row[ 2 ] );
   result.w = Math::DotProduct4( vector, row[ 3 ] );
   
   *pResult = result;
}

void Math::TransformPlane(
   Vector *pResult,
   const Vector &vector,
   const Matrix &matrix
)
{
   Vector result;
   
   Math::Transform4( &result, vector, matrix );
   
   *pResult = result;
}

void Math::Invert(
   ::Transform *pResult,
   const ::Transform &transform
)
{
   if( true == transform.IsScaled( ) )
   {
      Matrix matrix;
      Math::Invert( &matrix, transform.ToMatrix(true) );

      Vector scale;
      scale.x = Math::Normalize4( &matrix.row[0], matrix.row[0] );
      scale.y = Math::Normalize4( &matrix.row[1], matrix.row[1] );
      scale.z = Math::Normalize4( &matrix.row[2], matrix.row[2] );
      scale.w = Math::Normalize4( &matrix.row[3], matrix.row[3] );

      pResult->Set( matrix, scale );
   }
   else
   {
      Vector translation;
      transform.GetTranslation( &translation );

      Matrix rotation = transform.ToMatrix( false );
      rotation.SetTranslation( Math::ZeroVector() );
      Math::Transpose( &rotation, rotation );
      
      translation = - translation;
      Math::Rotate( &translation, translation, rotation );
            
      Matrix result = rotation;
      result.SetTranslation( translation );
      
      pResult->Set( result, Vector(1, 1, 1, 1) );
   }
}

void Math::RotateX(
   Matrix *pResult,
   float radians
)
{
   float c, s;

   Math::CosSin( &c, &s, radians );

   pResult->row[ 0 ].x = 1;
   pResult->row[ 0 ].y = 0;
   pResult->row[ 0 ].z = 0;
   pResult->row[ 0 ].w = 0;

   pResult->row[ 1 ].x = 0;
   pResult->row[ 1 ].y = c;
   pResult->row[ 1 ].z = s;
   pResult->row[ 1 ].w = 0;

   pResult->row[ 2 ].x = 0;
   pResult->row[ 2 ].y = - s;
   pResult->row[ 2 ].z = c;
   pResult->row[ 2 ].w = 0;

   pResult->row[ 3 ].x = 0;
   pResult->row[ 3 ].y = 0;
   pResult->row[ 3 ].z = 0;
   pResult->row[ 3 ].w = 1;
}

void Math::RotateX(
   Transform *pResult,
   float radians
)
{
   Matrix result;
   RotateX( &result, radians );

   pResult->Set( result, Vector(1, 1, 1, 1) );
}

void Math::RotateY(
   Matrix *pResult,
   float radians
)
{
   float c, s;

   Math::CosSin( &c, &s, radians );

   pResult->row[ 0 ].x = c;
   pResult->row[ 0 ].y = 0;
   pResult->row[ 0 ].z = - s;
   pResult->row[ 0 ].w = 0;

   pResult->row[ 1 ].x = 0;
   pResult->row[ 1 ].y = 1;
   pResult->row[ 1 ].z = 0;
   pResult->row[ 1 ].w = 0;

   pResult->row[ 2 ].x = s;
   pResult->row[ 2 ].y = 0;
   pResult->row[ 2 ].z = c;
   pResult->row[ 2 ].w = 0;

   pResult->row[ 3 ].x = 0;
   pResult->row[ 3 ].y = 0;
   pResult->row[ 3 ].z = 0;
   pResult->row[ 3 ].w = 1;
}

void Math::RotateY(
   Transform *pResult,
   float radians
)
{
   Matrix result;
   RotateY( &result, radians );

   pResult->Set( result, Vector(1, 1, 1, 1) );
}

void Math::RotateZ(
   Matrix2x2 *pResult,
   float radians
)
{
   float c, s;

   Math::CosSin( &c, &s, radians );

   pResult->row[ 0 ].x = c;
   pResult->row[ 0 ].y = s;

   pResult->row[ 1 ].x = - s;
   pResult->row[ 1 ].y = c;
}

void Math::RotateZ(
   Matrix *pResult,
   float radians
)
{
   float c, s;

   Math::CosSin( &c, &s, radians );

   pResult->row[ 0 ].x = c;
   pResult->row[ 0 ].y = s;
   pResult->row[ 0 ].z = 0;
   pResult->row[ 0 ].w = 0;

   pResult->row[ 1 ].x = - s;
   pResult->row[ 1 ].y = c;
   pResult->row[ 1 ].z = 0;
   pResult->row[ 1 ].w = 0;

   pResult->row[ 2 ].x = 0;
   pResult->row[ 2 ].y = 0;
   pResult->row[ 2 ].z = 1;
   pResult->row[ 2 ].w = 0;

   pResult->row[ 3 ].x = 0;
   pResult->row[ 3 ].y = 0;
   pResult->row[ 3 ].z = 0;
   pResult->row[ 3 ].w = 1;
}

void Math::RotateZ(
   Transform *pResult,
   float radians
)
{
   Matrix result;
   RotateZ( &result, radians );

   pResult->Set( result, Vector(1, 1, 1, 1) );
}

void Math::Scale(
   Matrix *pResult,
   const Vector &scale
)
{
   pResult->SetRow( 0, Vector(scale.x, 0, 0, 0) );
   pResult->SetRow( 1, Vector(0, scale.y, 0, 0) );
   pResult->SetRow( 2, Vector(0, 0, scale.z, 0) );
   pResult->SetRow( 3, Vector(0, 0, 0, scale.w) );
}

void Math::ScaleVector(
   Vector *pResult,
   const Vector &vector,
   float scale
)
{
   *pResult = vector * scale;
}

void Math::Perspective(
   Matrix *pProjection,
   float fovX,
   float aspect,
   float nearZ,
   float farZ
)
{
   //thx to https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
   // for helping me get this working correctly
   float w = 1.0f / Math::Tan(fovX / 2.0f);
   float h = w * aspect;

   float q = farZ / (farZ - nearZ);

   pProjection->row[ 0 ].x = w;
   pProjection->row[ 0 ].y = 0.0f;
   pProjection->row[ 0 ].z = 0.0f;
   pProjection->row[ 0 ].w = 0.0f;

   pProjection->row[ 1 ].x = 0.0f;
   pProjection->row[ 1 ].y = h;
   pProjection->row[ 1 ].z = 0.0f;
   pProjection->row[ 1 ].w = 0.0f;

   pProjection->row[ 2 ].x = 0.0f;
   pProjection->row[ 2 ].y = 0.0f;
   pProjection->row[ 2 ].z = q;
   pProjection->row[ 2 ].w = 1.0f;

   pProjection->row[ 3 ].x = 0.0f;
   pProjection->row[ 3 ].y = 0.0f;
   pProjection->row[ 3 ].z = - nearZ * q;
   pProjection->row[ 3 ].w = 0.0f;
}

void Math::PerspectiveWH(
   Matrix *pProjection,
   float width,
   float height,
   float nearZ,
   float farZ
)
{
   float w = width;
   float h = height;

   // Support reverse depth projection
   float m = Math::Min( nearZ, farZ );

   w = (2.0f * m) / w;
   h = (2.0f * m) / h;

   float q = farZ / (farZ - nearZ);

   pProjection->row[ 0 ].x = w;
   pProjection->row[ 0 ].y = 0.0f;
   pProjection->row[ 0 ].z = 0.0f;
   pProjection->row[ 0 ].w = 0.0f;

   pProjection->row[ 1 ].x = 0.0f;
   pProjection->row[ 1 ].y = h;
   pProjection->row[ 1 ].z = 0.0f;
   pProjection->row[ 1 ].w = 0.0f;

   pProjection->row[ 2 ].x = 0.0f;
   pProjection->row[ 2 ].y = 0.0f;
   pProjection->row[ 2 ].z = q;
   pProjection->row[ 2 ].w = 1.0f;

   pProjection->row[ 3 ].x = 0.0f;
   pProjection->row[ 3 ].y = 0.0f;
   pProjection->row[ 3 ].z = - nearZ * q;
   pProjection->row[ 3 ].w = 0.0f;
}

void Math::PerspectiveOffCenter(
   Matrix *pProjection,
   float left,
   float right,
   float top,
   float bottom,
   float nearZ,
   float farZ
   )
{
   PerspectiveWH( pProjection, right - left, top - bottom, nearZ, farZ );

   pProjection->row[ 2 ].x = (left + right) / (left - right);
   pProjection->row[ 2 ].y = (top + bottom) / (top - bottom);
}

void Math::Orthogonal(
   Matrix *pProjection,
   float width,
   float height,
   float nearZ,
   float farZ
)
{
   float q = 1 / (farZ - nearZ);

   pProjection->row[ 0 ].x = 2.0f / width;
   pProjection->row[ 0 ].y = 0.0f;
   pProjection->row[ 0 ].z = 0.0f;
   pProjection->row[ 0 ].w = 0.0f;

   pProjection->row[ 1 ].x = 0.0f;
   pProjection->row[ 1 ].y = 2.0f / height;
   pProjection->row[ 1 ].z = 0.0f;
   pProjection->row[ 1 ].w = 0.0f;

   pProjection->row[ 2 ].x = 0.0f;
   pProjection->row[ 2 ].y = 0.0f;
   pProjection->row[ 2 ].z = q;
   pProjection->row[ 2 ].w = 0.0f;

   pProjection->row[ 3 ].x = 0.0f;
   pProjection->row[ 3 ].y = 0.0f;
   pProjection->row[ 3 ].z = nearZ / (nearZ - farZ);
   pProjection->row[ 3 ].w = 1;
}
