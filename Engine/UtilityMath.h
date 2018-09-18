#pragma once

class Color
{
public:
    Color(){}

    Color(int red, int green, int blue, int alpha)
    {
        r = red; g = green; b = blue; a = alpha;
    }

    int r;
    int g;
    int b;
    int a;
};

class Vector2
{
public:
    float x;
    float y;

    Vector2( void ) {}

    Vector2(
        float _x,
        float _y
        )
    {
        x = _x;
        y = _y;
    }

    void Set(
        float _x,
        float _y
        )
    {
        x = _x;
        y = _y;
    }

    Vector2 operator * (const float rhs) const
    {
        return Vector2( x * rhs, y * rhs );
    }

    Vector2 operator * (const Vector2 &rhs) const
    {
        return Vector2( rhs.x * x, rhs.y * y );
    }

    Vector2 operator - (const Vector2 &rhs) const
    {
        return Vector2( x - rhs.x, y - rhs.y );
    }

    Vector2 operator - ( void ) const
    {
        return Vector2( - x, - y );
    }

    Vector2 operator + (const Vector2 &rhs) const
    {
        return Vector2( x + rhs.x, y + rhs.y );
    }

    bool operator == (const Vector2 &rhs) const
    {
        return 0 == memcmp(this, &rhs, sizeof(Vector2));
    }

    bool operator != (const Vector2 &rhs) const
    {
        return 0 != memcmp(this, &rhs, sizeof(Vector2));
    }

    void operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
    }

    void operator += (const Vector2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }

    void operator -= (const Vector2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }
};

class Vector
{
public:
    float x;
    float y;
    float z;
    float w;

    Vector( void ) {}

    Vector(
        float _x,
        float _y,
        float _z,
        float _w
        )
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vector(
        float _x,
        float _y,
        float _z
        )
    {
        x = _x;
        y = _y;
        z = _z;
        w = 0;
    }

    Vector(
        float _x,
        float _y
        )
    {
        x = _x;
        y = _y;
        z = 0;
        w = 0;
    }

    void Set(
        float _x,
        float _y,
        float _z,
        float _w
        )
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vector(
        const Vector2 &vector2
        )
    {
        *this = Vector( vector2.x, vector2.y );
    }

    Vector operator * (const float rhs) const
    {
        return Vector( x * rhs, y * rhs, z * rhs, w * rhs );
    }

    Vector operator * (const Vector &rhs) const
    {
        return Vector( rhs.x * x, rhs.y * y, rhs.z * z, rhs.w * w );
    }

    friend Vector operator *(float lhs, const Vector &rhs)
    {
        return rhs * lhs;
    }

    Vector operator - (const Vector &rhs) const
    {
        return Vector( x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w );
    }

    Vector operator - ( void ) const
    {
        return Vector( - x, - y, - z, - w );
    }

    Vector operator + (const Vector &rhs) const
    {
        return Vector( x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w );
    }

    bool operator == (const Vector &rhs) const
    {
        return 0 == memcmp(this, &rhs, sizeof(Vector));
    }

    bool operator != (const Vector &rhs) const
    {
        return 0 != memcmp(this, &rhs, sizeof(Vector));
    }

    void operator += (const Vector &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
    }

    void operator -= (const Vector &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
    }
};

class Matrix2x2
{
    friend class Math;

private:
    Vector2 row[ 2 ];

public:
    Matrix2x2( void ) {}

    Matrix2x2(
        const Vector2 &right,
        const Vector2 &up
        )
    {
        SetRight( right );
        SetUp( up );
    }

    void SetRight(
        const Vector2 &vector
        )
    {
        SetRow( 0, vector );
    }

    void SetUp(
        const Vector2 &vector
        )
    {
        SetRow( 1, vector );
    }

    void SetRow( 
        int i, 
        const Vector2 &vector
        )
    {
        row[ i ] = vector;
    }

    void GetRight( Vector2 *pVector ) const { GetRow(0, pVector); }
    void GetUp   ( Vector2 *pVector ) const { GetRow(1, pVector); }

    void GetRow( int i, Vector2 *pVector ) const { *pVector = row[ i ]; }
};

class Quaternion;

class Matrix
{
    friend class Math;
    friend class Quaternion;
    friend class Transform;

private:
    Vector row[ 4 ];

public:
    Matrix( void ) {}

    Matrix( float value )
    {
        SetRow( 0, Vector(value, value, value, value) );
        SetRow( 1, Vector(value, value, value, value) );
        SetRow( 2, Vector(value, value, value, value) );
        SetRow( 3, Vector(value, value, value, value) );
    }

    Matrix(
        const Vector &right,
        const Vector &up,
        const Vector &look,
        const Vector &translation
        )
    {
        SetRight( right );
        SetUp( up );
        SetLook( look );
        SetTranslation( translation );
    }

    Matrix(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
        )
    {
        SetRow( 0, Vector(m00, m01, m02, m03) );
        SetRow( 1, Vector(m10, m11, m12, m13) );
        SetRow( 2, Vector(m20, m21, m22, m23) );
        SetRow( 3, Vector(m30, m31, m32, m33) );
    }

    void SetRight(
        const Vector &vector
        )
    {
        SetRow( 0, vector );
        row[ 0 ].w = 0;
    }

    void SetUp(
        const Vector &vector
        )
    {
        SetRow( 1, vector );
        row[ 1 ].w = 0;
    }

    void SetLook(
        const Vector &vector
        )
    {
        SetRow( 2, vector );
        row[ 2 ].w = 0;
    }

    void SetTranslation(
        const Vector &vector
        )
    {
        SetRow( 3, vector );
        row[ 3 ].w = 1;
    }

    void SetRow( 
        int i, 
        const Vector &vector
        )
    {
        row[ i ] = vector;
    }

    void GetRow(
        int i,
        Vector *pVector
        ) const
    {
        *pVector = row[ i ];
    }

    void GetRight( Vector *pVector )       const { GetRow(0, pVector); pVector->w = 0.0f; }
    void GetUp( Vector *pVector )          const { GetRow(1, pVector); pVector->w = 0.0f; }
    void GetLook( Vector *pVector )        const { GetRow(2, pVector); pVector->w = 0.0f; }
    void GetTranslation( Vector *pVector ) const { GetRow(3, pVector); pVector->w = 1.0f; }

    Vector GetRight( void )       const { Vector v; GetRight(&v);       return v; }
    Vector GetUp( void )          const { Vector v; GetUp(&v);          return v; }
    Vector GetLook( void )        const { Vector v; GetLook(&v);        return v; }
    Vector GetTranslation( void ) const { Vector v; GetTranslation(&v); return v; }

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

    Matrix operator * (const float rhs) const
    {
        Matrix m;

        m.row[ 0 ] = row[ 0 ] * rhs;
        m.row[ 1 ] = row[ 1 ] * rhs;
        m.row[ 2 ] = row[ 2 ] * rhs;
        m.row[ 3 ] = row[ 3 ] * rhs;

        return m;
    }

    void operator += (const Matrix &rhs)
    {
        row[ 0 ] += rhs.row[ 0 ];
        row[ 1 ] += rhs.row[ 1 ];
        row[ 2 ] += rhs.row[ 2 ];
        row[ 3 ] += rhs.row[ 3 ];
    }
};

class Transform;

class Quaternion
{

    friend class Matrix;
    friend class Math;

public:
    Vector quaternion;

    public:
        Quaternion( void ) {}

        Quaternion(
            float x,
            float y,
            float z,
            float w
            )
        {
            Set( x, y, z, w );
        }


        Quaternion(
            const Vector &look,
            const Vector &up
            )
        {
            Set( look, up );
        }

        void Set(
            float x,
            float y,
            float z,
            float w
            )
        {
            quaternion = Vector( x, y, z, w );
        }

        inline void Set(
            const Transform &transform
            );

        inline void Set(
            const Vector &look,
            const Vector &up
            );

        void operator += (const Quaternion &rhs)
        {
            quaternion.x += rhs.quaternion.x;
            quaternion.y += rhs.quaternion.y;
            quaternion.z += rhs.quaternion.z;
            quaternion.w += rhs.quaternion.w;
        }

        Quaternion operator * (const Quaternion &rhs) const
        {
            Quaternion q;

            q.quaternion.w = ( quaternion.w * rhs.quaternion.w - quaternion.x * rhs.quaternion.x - 
                quaternion.y * rhs.quaternion.y - quaternion.z * rhs.quaternion.z );
            q.quaternion.x = ( quaternion.w * rhs.quaternion.x + quaternion.x * rhs.quaternion.w + 
                quaternion.y * rhs.quaternion.z - quaternion.z * rhs.quaternion.y );
            q.quaternion.y = ( quaternion.w * rhs.quaternion.y + quaternion.y * rhs.quaternion.w + 
                quaternion.z * rhs.quaternion.x - quaternion.x * rhs.quaternion.z );
            q.quaternion.z = ( quaternion.w * rhs.quaternion.z + quaternion.z * rhs.quaternion.w + 
                quaternion.x * rhs.quaternion.y - quaternion.y * rhs.quaternion.x );
            return q;
        }

        Quaternion operator * (const float rhs) const
        {
            Quaternion q( quaternion.x * rhs, quaternion.y * rhs, 
                quaternion.z * rhs, quaternion.w * rhs );
            return q;
        }
    };

    class Transform
    {
    private:
        Matrix transform;
        Vector scale;

    public:
        Transform( void ) 
        {}

        Transform(
            const Transform &_transform
            )
        {
            transform = _transform.transform;
            scale = _transform.scale;
        }

        Transform(
            const Matrix &matrix
            );

        Transform(const Vector &look, const Vector &up, const Vector &translation)
        {
            transform.SetOrientation(look, up);
            transform.SetTranslation(translation);

            SetScale( Vector(1, 1, 1, 1) );
        }

        Transform(const Quaternion &rotation, const Vector &translation, const Vector &scale)
        {
            transform.SetOrientation(rotation);
            transform.SetTranslation(translation);

            SetScale( scale );
        }

        void Set(
            const Matrix &noScaleMatrix, 
            const Vector &_scale
            )
        {
            transform = noScaleMatrix;
            SetScale( _scale );
        }

        void GetRight( Vector *pRight )             const { transform.GetRight(pRight); }
        void GetUp( Vector *pUp )                   const { transform.GetUp(pUp); }
        void GetLook( Vector *pLook )               const { transform.GetLook(pLook); }
        void GetTranslation( Vector *pTranslation ) const { transform.GetTranslation(pTranslation); }
        void GetScale( Vector *pScale )             const { *pScale = scale; }

        Vector GetRight( )       const { return transform.GetRight(); }
        Vector GetUp( )          const { return transform.GetUp(); }
        Vector GetLook( )        const { return transform.GetLook(); }
        Vector GetTranslation( ) const { return transform.GetTranslation(); }
        Vector GetScale( )       const { return scale; }

        Quaternion GetOrientation( ) const { return Quaternion(transform.GetLook(), transform.GetUp()); }

        inline Transform GetInverse() const;

        inline void SetIdentity();

        void SetRight(
            const Vector &right
            ) 
        { transform.SetRight(right); }

        void SetUp(
            const Vector &up
            ) 
        { transform.SetUp(up); }

        void SetLook(
            const Vector &look
            ) 
        { transform.SetLook(look); }

        void SetTranslation(
            const Vector &translation
            ) 
        { transform.SetTranslation(translation); }

        inline bool IsScaled( void ) const 
        { 
            const float high = 1.0f + FLT_EPSILON;
            const float low  = 1.0f - FLT_EPSILON;

            bool result = scale.x > high || scale.x < low || 
                scale.y > high || scale.y < low || 
                scale.z > high || scale.z < low || 
                scale.w > high || scale.w < low;

            return result;
        }

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

        Transform operator * (const float rhs) const
        {
            Transform t;

            t.transform.row[ 0 ] = transform.row[ 0 ] * rhs;
            t.transform.row[ 1 ] = transform.row[ 1 ] * rhs;
            t.transform.row[ 2 ] = transform.row[ 2 ] * rhs;
            t.transform.row[ 3 ] = transform.row[ 3 ] * rhs;

            t.SetScale( scale );
            return t;
        }

        void operator += (const Transform &rhs)
        {
            transform.row[ 0 ] += rhs.transform.row[ 0 ];
            transform.row[ 1 ] += rhs.transform.row[ 1 ];
            transform.row[ 2 ] += rhs.transform.row[ 2 ];
            transform.row[ 3 ] += rhs.transform.row[ 3 ];
        }

        bool operator == (const Transform &rhs) const
        {
            return transform.row[ 0 ] == rhs.transform.row[ 0 ] &&
                transform.row[ 1 ] == rhs.transform.row[ 1 ] &&
                transform.row[ 2 ] == rhs.transform.row[ 2 ] &&
                transform.row[ 3 ] == rhs.transform.row[ 3 ] &&
                scale == rhs.scale;
        }

        bool operator != (const Transform &rhs) const
        {
            return false == (*this == rhs);
        }
    };


    class Math
    {
    private:
        static Vector  sm_ZeroVector;
        static Vector  sm_OneVector;
        static Vector2 sm_ZeroVector2;
        static Vector2 sm_OneVector2;

        static Matrix    sm_IdentityMatrix;
        static Matrix    sm_ZeroMatrix;
        static Matrix2x2 sm_IdentityMatrix2x2;
        static Transform sm_IdentityTransform;

        static Quaternion sm_IdentityQuaternion;

        static float sm_PI;

    public:
        static Vector  ZeroVector( void )  { return sm_ZeroVector; }
        static Vector  OneVector( void )   { return sm_OneVector; }
        static Vector2 ZeroVector2( void ) { return sm_ZeroVector2; }
        static Vector2 OneVector2( void )  { return sm_OneVector2; }

        static Matrix    IdentityMatrix( void )      { return sm_IdentityMatrix; }
        static Matrix    ZeroMatrix( void )          { return sm_ZeroMatrix; }
        static Matrix2x2 IdentityMatrix2x2( void )   { return sm_IdentityMatrix2x2; }
        static Transform IdentityTransform( void )   { return sm_IdentityTransform; }

        static Quaternion IdentityQuaternion( void ) { return sm_IdentityQuaternion; }

        static float PI() { return sm_PI; }

        static void Initialize( void );

        static inline float DegreesToRadians(
            float degrees
            );

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

        static inline float Sign(
            float value
            );

        static inline float Sign(
            int value
            );

        static inline void CosSin(
            float *pCos,
            float *pSin,
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

        static inline float Magnitude4(
            const Vector &vector
            );

        static inline float Magnitude(
            const Vector2 &vector
            );

        static inline float MagnitudeSq(
            const Vector &vector
            );

        static inline float MagnitudeSq4(
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

        static inline float DotProduct(
            const Quaternion &qA,
            const Quaternion &qB
            );

        static inline void CrossProduct(
            Vector *pResult,
            const Vector &vectorA,
            const Vector &vectorB
            );

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

        static inline void Ceil(
            Vector *pResult,
            const Vector &vector
            );

        static inline void Abs(
            Vector2 *pResult,
            const Vector2 &vector
            );

        static inline int FloatToInt(
            float value
            );

        static inline bool FloatTest(
            float valueA,
            float valueB,
            float range
            );

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

        static inline double Min(
            double valueA,
            double valueB
            );

        static inline double Max(
            double valueA,
            double valueB
            );

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

        static inline float Lerp(
            float startValue,
            float endValue,
            float time
            );

        static inline void Lerp(
            float *pResult,
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

        static inline float Normalize(
            Vector *pResult,
            const Vector &result
            );

        static inline float Normalize4(
            Vector *pResult,
            const Vector &result
            );

        static inline float NormalizePlane(
            Vector *pResult,
            const Vector &result
            );

        static inline float Normalize(
            Vector2 *pResult,
            const Vector2 &result
            );

        static inline float Normalize(
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

        static inline void Rotate(
            Vector2 *pResult,
            const Vector2 &vector,
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

        static inline void TransformPosition(
            Vector2 *pResult,
            const Vector2 &vector,
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
            const ::Matrix &matrix
            );

        static inline void Invert(
            ::Transform *pResult,
            const ::Transform &transform
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

        static inline void PerspectiveWH(
            Matrix *pProjection,
            float width,
            float height,
            float nearZ,
            float farZ
            );

        static inline void PerspectiveOffCenter(
           Matrix *pProjection,
           float left,
           float right,
           float top,
           float bottom,
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

#include "UtilityMath.inl"
