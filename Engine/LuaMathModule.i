%module Math
%{
#include "EnginePch.h"
#include "UtilityMath.h"
#include "LuaMathModule.h"
#include "LuaCoreModule.h"
#include "AnimatedObject.h"
#include "FloatController.h"

extern "C" 
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}
%}

%nodefaultctor;

//%include "LuaMathModule.h"

class Vector2
{
public:
   float x;
   float y;

   Vector2( void );
   Vector2(
      float _x,
      float _y
   );
   
   void Set(
      float _x,
      float _y
   );

   Vector2 operator * (const float rhs) const;
   Vector2 operator * (const Vector2 &rhs) const;
   Vector2 operator - (const Vector2 &rhs) const;
   Vector2 operator - ( void ) const;
   Vector2 operator + (const Vector2 &rhs) const;
   bool operator == (const Vector2 &rhs) const;
};

class Vector
{
public:
   float x;
   float y;
   float z;
   float w;

   Vector( void );
   Vector(
      float _x,
      float _y,
      float _z,
      float _w
   );
   Vector(
      float _x,
      float _y,
      float _z
   );
   Vector(
      float _x,
      float _y
   );
   Vector(
      const Vector2 &vector2
   );

   void Set(
      float _x,
      float _y,
      float _z,
      float _w
   );

   Vector operator * (const float rhs) const;
   Vector operator * (const Vector &rhs) const;
   Vector operator - (const Vector &rhs) const;
   Vector operator - ( void ) const;
   Vector operator + (const Vector &rhs) const;
   bool operator == (const Vector &rhs) const;
};


class Matrix
{
public:
   Matrix( void );

   Matrix( float value );

   Matrix(
      const Vector &right,
      const Vector &up,
      const Vector &look,
      const Vector &translation
   );

   Matrix(float m00, float m01, float m02, float m03,
          float m10, float m11, float m12, float m13,
          float m20, float m21, float m22, float m23,
          float m30, float m31, float m32, float m33
    );

   void SetRight(
      const Vector &vector
   );

   void SetUp(
      const Vector &vector
   );

   void SetLook(
      const Vector &vector
   );

   void SetTranslation(
      const Vector &vector
   );
   
   void SetRow( 
      int i, 
      const Vector &vector
   );

   void GetRow(
      int i,
      Vector *pVector
   ) const;

   void GetRight( Vector *pVector )       const;
   void GetUp( Vector *pVector )          const;
   void GetLook( Vector *pVector )        const;
   void GetTranslation( Vector *pVector ) const;

   Vector GetRight( void )       const;
   Vector GetUp( void )          const;
   Vector GetLook( void )        const;
   Vector GetTranslation( void ) const;

   inline void SetOrientation(
      const Vector &look,
      const Vector &up
   );

   inline void SetOrientation(
      const Matrix &matrix
   );

   inline void SetOrientation(
      const Quaternion &quaternion
   );

   Matrix operator * (const float rhs) const;
   void operator += (const Matrix &rhs);
};

class Transform
{
public:
   Transform( void );

   Transform(
      const Matrix &matrix
   );

   Transform(
     const Transform &_transform
   );

   Transform(const Vector &look, const Vector &up, const Vector &translation);

   Transform(const Quaternion &rotation, const Vector &translation, const Vector &scale);

   void Set(
      const Matrix &noScaleMatrix, 
      const Vector &_scale
   );

   void GetRight( Vector *pRight )             const;
   void GetUp( Vector *pUp )                   const;
   void GetLook( Vector *pLook )               const;
   void GetTranslation( Vector *pTranslation ) const;
   void GetScale( Vector *pScale )             const;

   Vector GetRight( )       const;
   Vector GetUp( )          const;
   Vector GetLook( )        const;
   Vector GetTranslation( ) const;
   Vector GetScale( )       const;

   Quaternion GetOrientation( ) const;

   inline Transform GetInverse() const;

   inline void SetIdentity();

   void SetRight(
      const Vector &right
   );

   void SetUp(
      const Vector &up
   );

   void SetLook(
      const Vector &look
   );

   void SetTranslation(
      const Vector &translation
   );

   inline bool IsScaled( void ) const;

   inline void SetScale(
      const Vector &_scale
   );

   inline void SetOrientation(
      const Vector &look,
      const Vector &up
   );

   inline void SetOrientation(
      const Matrix2x2 &matrix
   );

   inline void SetOrientation(
      const Transform &transform
   );

   inline void SetOrientation(
      const Quaternion &quaternion
   );

   inline Matrix ToMatrix( 
      bool applyScaling 
   ) const;

   inline Transform operator * (const Transform &rhs) const;
   
   Transform operator * (const float rhs) const;

   void operator += (const Transform &rhs);
};

class Quaternion
{
public:
    Vector quaternion;

   Quaternion( void );
   Quaternion(
      float x,
      float y,
      float z,
      float w
   );

   Quaternion(
      const Vector &look,
      const Vector &up
   );

   void Set(
      float x,
      float y,
      float z,
      float w
   );
   inline void Set(
      const Transform &transform
   );
   inline void Set(
      const Vector &look,
      const Vector &up
   );

   Quaternion operator * (const Quaternion &rhs) const;
   Quaternion operator * (const float rhs) const;
};

class AnimatedObject
{
public:
   virtual void Create( void );
   virtual void Destroy( void );

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );
};

class FloatController : public AnimatedObject
{
public:
   FloatController();

   void StartLinear(
      float startValue,
      float endValue,
      float time
   );

   void StartFiltered(
      float startValue,
      float endValue,
      float time
   );
   
   virtual void Update(
      float deltaSeconds
   );

   float GetValue( void ) const;
   bool IsDone( void ) const;
};

class Math
{
public:
   static Vector  ZeroVector( void );
   static Vector2 ZeroVector2( void );

   static Matrix    IdentityMatrix( void );
   static Matrix2x2 IdentityMatrix2x2( void );
   static Transform IdentityTransform( void );

   static Quaternion IdentityQuaternion( void );

   static float PI( void );

   static inline float DegreesToRadians(
      float degrees
   );

   %rename(Rand_i) Rand(int, int);
   %rename(Rand_f) Rand(float, float);

   static inline int Rand(
      int minValue, 
      int maxValue
   );
   static inline float Rand(
      float minValue, 
      float maxValue
   );

   static inline float Cos(
      float radians
   );

   static inline float ACos(
      float radians
   );

   static inline float Sin(
      float radians
   );
   
   static inline float Tan(
      float radians
   );

   static inline float ATan(
      float radians
   );

   static inline float Sqrt(
      float value
   );

   static inline int MakePowerOf2(
      int value
   );

   static inline float Magnitude(
      const Vector &vector
   );
   static inline float Magnitude(
      const Vector2 &vector
   );

   static inline float MagnitudeSq(
      const Vector &vector
   );
   static inline float MagnitudeSq(
      const Vector2 &vector
   );

   static inline float DotProduct(
      const Vector &vectorA,
      const Vector &vectorB
   );
   static inline float DotProduct4(
      const Vector &vectorA,
      const Vector &vectorB
   );
   static inline float DotProduct(
      const Vector2 &vectorA,
      const Vector2 &vectorB
   );

   static inline void CrossProduct(
      Vector *pResult,
      const Vector &vectorA,
      const Vector &vectorB
   );

   %rename(Abs_i) Abs(int);
   %rename(Abs_f) Abs(float);

   static inline int Abs(
      int value
   );
   static inline float Abs(
      float value
   );
   static inline void Abs(
      Vector *pResult,
      const Vector &vector
   );

   %rename(Min_i) Min(int, int);
   %rename(Min_f) Min(float, float);
   %rename(Max_i) Max(int, int);
   %rename(Max_f) Max(float, float);

   static inline int Min(
      int valueA,
      int valueB
   );
   static inline int Max(
      int valueA,
      int valueB
   );
   static inline float Min(
      float valueA,
      float valueB
   );
   static inline float Max(
      float valueA,
      float valueB
   );

   %rename(Clamp_i) Clamp(int, int, int);
   %rename(Clamp_f) Clamp(float, float, float);

   static inline int Clamp(
      int value,
      int minValue,
      int maxValue
   );
   static inline float Clamp(
      float value,
      float minValue,
      float maxValue
   );

   %rename(Lerp_f) Lerp(float, float, float);
   %rename(Lerp_v) Lerp(Vector *, Vector, Vector, float);
   %rename(Lerp_q) Lerp(Quaternion *, Quaternion, Quaternion, float);

   static inline float Lerp(
      float startValue,
      float endValue,
      float time
   );

   static inline void Lerp(
      Vector *pResult,
      const Vector &startValue,
      const Vector &endValue,
      float time
   );

   static inline void Lerp(
      Quaternion *pResult,
      const Quaternion startValue,
      const Quaternion endValue,
      float time
   );

   %rename(Slerp_q) Slerp(Quaternion *, Quaternion, Quaternion, float);
   %rename(Slerp_t) Slerp(Transform *, Transform, Transform, float);

   static inline void Slerp(
      Quaternion *pResult,
      const Quaternion startValue,
      const Quaternion endValue,
      float time
   );

   static inline void Slerp(
      Transform *pResult,
      const Transform startValue,
      const Transform endValue,
      float time
   );

   static inline void VectorFromARGB(
      Vector *pVector,
      unsigned int argb
   );

   static inline void Min(
      Vector *pResult,
      const Vector &valueA,
      const Vector &valueB
   );
   static inline void Max(
      Vector *pResult,
      const Vector &valueA,
      const Vector &valueB
   );
   static inline void Min(
      Vector2 *pResult,
      const Vector2 &valueA,
      const Vector2 &valueB
   );
   static inline void Max(
      Vector2 *pResult,
      const Vector2 &valueA,
      const Vector2 &valueB
   );

   static inline void Normalize(
      Vector *pResult,
      const Vector &result
   );
   static inline void Normalize(
      Vector2 *pResult,
      const Vector2 &result
   );
   static inline void NormalizePlane(
      Vector *pResult,
      const Vector &result
   );
   static inline void Normalize(
      Quaternion *pResult,
      const Quaternion &result
   );

   static inline void Transpose(
      Matrix2x2 *pResult,
      const Matrix2x2 &matrix
   );
   static inline void Transpose(
      Matrix *pResult,
      const Matrix &matrix
   );

   static inline void Invert(
      Matrix *pResult,
      const Matrix &matrix
   );
   static inline void Invert(
      ::Transform *pResult,
      const ::Transform &transform
   );

   static inline void Multiply(
      Matrix *pResult,
      const Matrix &matrixA,
      const Matrix &matrixB
   );
   static inline void Multiply(
      Matrix2x2 *pResult,
      const Matrix2x2 &matrixA,
      const Matrix2x2 &matrixB
   );
   static inline void Multiply(
      Transform *pResult,
      const Transform &transformA,
      const Transform &transformB
   );

   static inline void Rotate(
      Vector *pResult,
      const Vector &vector,
      const Matrix &matrix
   );
   static inline void Rotate(
      Vector2 *pResult,
      const Vector2 &vector,
      const Matrix2x2 &matrix
   );
   static inline void Rotate(
      Vector *pResult,
      const Vector &vector,
      const ::Transform &transform
   );

   static inline void TransformPosition(
      Vector *pResult,
      const Vector &vector,
      const Matrix &matrix
   );

    static inline void TransformPosition(
        Vector *pResult,
        const Vector &vector,
        const Transform &transform
        );

   static inline void Transform4(
      Vector *pResult,
      const Vector &vector,
      const Matrix &matrix
   );
   static inline void TransformPlane(
      Vector *pResult,
      const Vector &vector,
      const Matrix &matrix
   );

   static inline void RotateX(
      Matrix *pResult,
      float radians
   );
   static inline void RotateX(
      Transform *pResult,
      float radians
   );

   static inline void RotateY(
      Matrix *pResult,
      float radians
   );
   static inline void RotateY(
      Transform *pResult,
      float radians
   );

   static inline void RotateZ(
      Matrix2x2 *pResult,
      float radians
   );
   static inline void RotateZ(
      Matrix *pResult,
      float radians
   );
   static inline void RotateZ(
      Transform *pResult,
      float radians
   );

   static inline void Scale(
      Matrix *pResult,
      const Vector &scale
   );
   
   static inline void ScaleVector(
      Vector *pResult,
      const Vector &vector,
      float scale
   );

   static inline void Perspective(
      Matrix *pProjection,
      float fovX,
      float aspect,
      float nearZ,
      float farZ
   );

   static inline void Orthogonal(
      Matrix *pProjection,
      float width,
      float height,
      float nearZ,
      float farZ
   );
};
