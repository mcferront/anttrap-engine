#pragma once

#include "EngineGlobal.h"

class BezierSpline
{
protected:
    int            m_NumControlPoints;
    int            m_NumCurves;
    float          m_TotalLength;

    Vector        *m_pControlPoints;
    float         *m_pLengthCache;

public:
    BezierSpline();
    ~BezierSpline();

    void operator = (const BezierSpline &rhs);

    void Init(int numControlPoints);
    void Uninit();

    void UpdateCache();

    float GetLength() const { return m_TotalLength; }

    int GetNumControlPoints() const { return m_NumControlPoints; }
    Vector* GetControlPoints() const { return m_pControlPoints; }

    int GetNumCurves() const { return m_NumCurves; }

    Vector GetPosition(float t) const;
    Vector GetPosition(int curveIndex, float t) const;

    Quaternion GetRotation(float t) const;
    Quaternion GetRotation(int curveIndex, float t) const;

    Transform GetTransform(float t) const;
    Transform GetTransform(int curveIndex, float t) const;

    float GetClosestParam(Vector toPosition, int iterations = 5) const;
    Vector GetClosestPosition(Vector toPosition, int iterations = 5) const;
    Quaternion GetClosestRotation(Vector toPosition, int iterations = 5) const;
    Transform GetClosestTransform(Vector toPosition, int iterations = 5) const;

private:
    float GetClosestParam(Vector toPosition, float startParam, float endParam, int steps) const;

};
