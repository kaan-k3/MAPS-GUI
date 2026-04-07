#pragma once
#include <vector>
#include <cmath>
#include <QVector3D>

struct NurbsSurface
{
    int K1 = 0, K2 = 0;   // upper indices
    int M1 = 0, M2 = 0;   // degrees (for polynomial)

    std::vector<double> U;      // U knot vector; size K1+M1+2
    std::vector<double> V;      // V knot vector; size K2+M2+2
    std::vector<double> W;      // Weight; size (K1+1)(K2+1)

    std::vector<QVector3D> P;   // control points, size (K1+1)*(K2+1)
};

inline bool isFiniteVec(const QVector3D& v)
{
    return std::isfinite(v.x()) && std::isfinite(v.y()) && std::isfinite(v.z());
}
