#include "EnginePch.h"

#include "BezierSpline.h"

BezierSpline::BezierSpline()
{
    m_NumControlPoints = 0;
    m_NumCurves = 0;
    m_TotalLength = 0.0f;

    m_pControlPoints = NULL;
    m_pLengthCache = NULL;
}

BezierSpline::~BezierSpline()
{
    Uninit();
}

void BezierSpline::Init(int numControlPoints)
{
    Debug::Assert(Condition(numControlPoints >= 4 && (numControlPoints - 1) % 3 == 0), "BezierSpline::Init - invalid number of control points %d", numControlPoints);

    Uninit();

    m_NumControlPoints = numControlPoints;
    m_NumCurves = (numControlPoints - 1) / 3;

    m_pControlPoints = new Vector[m_NumControlPoints];
    m_pLengthCache = NULL;
}

void BezierSpline::Uninit()
{
    delete [] m_pControlPoints;
    delete [] m_pLengthCache;

    m_NumControlPoints = 0;
    m_NumCurves = 0;
    m_TotalLength = 0.0f;

    m_pControlPoints = NULL;
    m_pLengthCache = NULL;
}

void BezierSpline::operator = (const BezierSpline &rhs)
{
    if (rhs.m_NumControlPoints != m_NumControlPoints)
    {
        delete [] m_pControlPoints;
        delete [] m_pLengthCache;

        m_pControlPoints = new Vector[rhs.m_NumControlPoints];
        m_pLengthCache = new float[rhs.m_NumCurves];
    }

    m_NumControlPoints = rhs.m_NumControlPoints;
    m_NumCurves = rhs.m_NumCurves;
    m_TotalLength = rhs.m_TotalLength;
    memcpy(m_pControlPoints, rhs.m_pControlPoints, sizeof(Vector) * m_NumControlPoints);
    memcpy(m_pLengthCache, rhs.m_pLengthCache, sizeof(float) * m_NumCurves);
}

void BezierSpline::UpdateCache()
{
    Vector firstCP;
    Vector fourthCP;
    Vector diff;

    m_TotalLength = 0.0f;
    m_pLengthCache = new float[m_NumCurves];

    firstCP = m_pControlPoints[0];

    // Currently calculates the linear distance between the first and 4th control point of each curve
    for (int i = 0; i < m_NumCurves; i++)
    {
        firstCP = m_pControlPoints[i * 3];
        fourthCP = m_pControlPoints[(i + 1) * 3];
        diff = fourthCP - firstCP;

        m_pLengthCache[i] = Math::Sqrt(Math::DotProduct(diff, diff)) + m_TotalLength;
        m_TotalLength = m_pLengthCache[i];
    }
}

Vector BezierSpline::GetPosition(float t) const
{
    float curveStartParam = 0.0f;
    float curveEndParam;
    int closestCurve = 0;

    t = Math::Max(0.0f, Math::Min(t, 1.0f));

    for (int i = 0; i < m_NumCurves; i++)
    {
        curveEndParam = m_pLengthCache[i] / m_TotalLength;

        if (t >= curveStartParam && t <= curveEndParam)
        {
            closestCurve = i;
            break;
        }

        curveStartParam = curveEndParam;
    }

    float curveParam = (t - curveStartParam) / (curveEndParam - curveStartParam);

    return GetPosition(closestCurve, curveParam);
}

Vector BezierSpline::GetPosition(int curveIndex, float t) const
{
    int cpIndex = curveIndex * 3;
    Vector cp1 = m_pControlPoints[cpIndex++];
    Vector cp2 = m_pControlPoints[cpIndex++];
    Vector cp3 = m_pControlPoints[cpIndex++];
    Vector cp4 = m_pControlPoints[cpIndex];

    float ax, ay, az, bx, by, bz, cx, cy, cz;
    float tSquared, tCubed;

    // Calculate the polynomial coefficients
    cx = 3.0f * (cp2.x - cp1.x);
    bx = 3.0f * (cp3.x - cp2.x) - cx;
    ax = cp4.x - cp1.x - cx - bx;

    cy = 3.0f * (cp2.y - cp1.y);
    by = 3.0f * (cp3.y - cp2.y) - cy;
    ay = cp4.y - cp1.y - cy - by;

    cz = 3.0f * (cp2.z - cp1.z);
    bz = 3.0f * (cp3.z - cp2.z) - cz;
    az = cp4.z - cp1.z - cz - bz;

    // Calculate the point at t
    tSquared = t * t;
    tCubed = tSquared * t;

    Vector position;

    position.x = (ax * tCubed) + (bx * tSquared) + (cx * t) + cp1.x;
    position.y = (ay * tCubed) + (by * tSquared) + (cy * t) + cp1.y;
    position.z = (az * tCubed) + (bz * tSquared) + (cz * t) + cp1.z;

    return position;
}

Quaternion BezierSpline::GetRotation(float t) const
{
    float curveStartParam = 0.0f;
    float curveEndParam;
    int closestCurve = 0;

    t = Math::Max(0.0f, Math::Min(t, 1.0f));

    for (int i = 0; i < m_NumCurves; i++)
    {
        curveEndParam = m_pLengthCache[i] / m_TotalLength;

        if (t >= curveStartParam && t <= curveEndParam)
        {
            closestCurve = i;
            break;
        }

        curveStartParam = curveEndParam;
    }

    float curveParam = (t - curveStartParam) / (curveEndParam - curveStartParam);

    return GetRotation(closestCurve, curveParam);
}

Quaternion BezierSpline::GetRotation(int curveIndex, float t) const
{
    static float coefs[] =
    {
        -1.0,  3.0, -3.0,  1.0,
        3.0, -6.0,  3.0,  0.0,
        -3.0,  3.0,  0.0,  0.0,
        1.0,  0.0,  0.0,  0.0
    };

    // For now, clamp to avoid t=0 and t=1 since this code cannot find a valid tangent in either case
    if (t < 0.01f) t = 0.01f;
    else if (t > 0.99f) t = 0.99f;

    int cpIndex = curveIndex * 3;
    Vector cp1 = m_pControlPoints[cpIndex++];
    Vector cp2 = m_pControlPoints[cpIndex++];
    Vector cp3 = m_pControlPoints[cpIndex++];
    Vector cp4 = m_pControlPoints[cpIndex];

    float b1;
    float b2;
    float b3;
    float b4;
    float t2 = t * t;

    t = t * 2.0f;
    t2 = t2 * 3.0f;

    b1 = coefs[ 0] * t2 + coefs[ 1] * t + coefs[ 2];
    b2 = coefs[ 4] * t2 + coefs[ 5] * t + coefs[ 6];
    b3 = coefs[ 8] * t2 + coefs[ 9] * t + coefs[10];
    b4 = coefs[12] * t2 + coefs[13] * t + coefs[14];

    Vector tangent = Vector(b1 * cp1.x + b2 * cp2.x + b3 * cp3.x + b4 * cp4.x, 
        b1 * cp1.y + b2 * cp2.y + b3 * cp3.y + b4 * cp4.y, 
        b1 * cp1.z + b2 * cp2.z + b3 * cp3.z + b4 * cp4.z);

    Quaternion rotation;
    Vector up = Vector(0, 1, 0);
    float tangentLength = Math::Normalize(&tangent, tangent);

    if (tangentLength > 0.0f)
    {
        if (Math::Abs(Math::DotProduct(tangent, up)) > 0.99f)
            up = Vector(1, 0, 0);

        Vector right;
        Math::CrossProduct(&right, up, tangent);
        Math::Normalize(&right, right);

        Math::CrossProduct(&up, tangent, right);

        rotation = Quaternion(tangent, up);
    }
    else
    {
        Debug::Assert(Condition(false), "Could not find tangent on BezierSpline");
        rotation = Math::IdentityQuaternion();
    }

    return rotation;
}

Transform BezierSpline::GetTransform(float t) const
{
    float curveStartParam = 0.0f;
    float curveEndParam;
    int closestCurve = 0;

    t = Math::Max(0.0f, Math::Min(t, 1.0f));

    for (int i = 0; i < m_NumCurves; i++)
    {
        curveEndParam = m_pLengthCache[i] / m_TotalLength;

        if (t >= curveStartParam && t <= curveEndParam)
        {
            closestCurve = i;
            break;
        }

        curveStartParam = curveEndParam;
    }

    float curveParam = (t - curveStartParam) / (curveEndParam - curveStartParam);

    return GetTransform(closestCurve, curveParam);
}

Transform BezierSpline::GetTransform(int curveIndex, float t) const
{
    Vector position = GetPosition(curveIndex, t);
    Quaternion rotation = GetRotation(curveIndex, t);

    return Transform(rotation, position, Vector(1, 1, 1, 1));
}

float BezierSpline::GetClosestParam(Vector toPosition, int iterations /* = 5*/) const
{
    iterations = Math::Max(0, Math::Min(iterations, 20));
    int steps = m_NumCurves * 10;
    int subSteps = 10;

    float minParam = GetClosestParam(toPosition, 0.0f, 1.0f, steps);
    float lastStepSize = 1.0f / steps;

    for (int i = 1; i < iterations; i++)
    {
        float startParam = Math::Max(0.0f, Math::Min(minParam - lastStepSize, 1.0f));
        float endParam = Math::Max(0.0f, Math::Min(minParam + lastStepSize, 1.0f));

        minParam = GetClosestParam(toPosition, startParam, endParam, subSteps);

        lastStepSize = (endParam - startParam) / subSteps;
    }

    return minParam;
}

Vector BezierSpline::GetClosestPosition(Vector toPosition, int iterations /* = 5*/) const
{
    return GetPosition(GetClosestParam(toPosition, iterations));
}

Quaternion BezierSpline::GetClosestRotation(Vector toPosition, int iterations /* = 5*/) const
{
    return GetRotation(GetClosestParam(toPosition, iterations));
}

Transform BezierSpline::GetClosestTransform(Vector toPosition, int iterations /* = 5*/) const
{
    return GetTransform(GetClosestParam(toPosition, iterations));
}

float BezierSpline::GetClosestParam(Vector toPosition, float startParam, float endParam, int steps) const
{
    float minDistance = FLT_MAX;
    float minParam = 0.0f;
    float stepSize = (endParam - startParam) / steps;
    float param;
    float distance;
    Vector position;

    for (int i = 0; i <= steps; i++)
    {
        param = startParam + i * stepSize;
        position = GetPosition(param);
        distance = Math::MagnitudeSq(position - toPosition);

        if (distance < minDistance)
        {
            minDistance = distance;
            minParam = param;
        }
    }

    return minParam;
}