#pragma once
#include "nurbs.h"
#include <vector>
#include <cmath>


inline int findSpan(int n, int p, double u, const std::vector<double>& U)
{
    // n = number of control points - 1
    // p = degree
    // Special case: u at the end
    if (u >= U[n + 1])
        return n;

    // Binary search
    int low = p;
    int high = n + 1;
    int mid = (low + high) / 2;

    while (u < U[mid] || u >= U[mid + 1])
    {
        if (u < U[mid])
            high = mid;
        else
            low = mid;
        mid = (low + high) / 2;
    }

    return mid;
}



inline void basisFunctions(int span, double u, int p, const std::vector<double>& U,
                           std::vector<double>& N)
{
    N.resize(p + 1);
    std::vector<double> left(p + 1);
    std::vector<double> right(p + 1);

    N[0] = 1.0;

    for (int j = 1; j <= p; ++j)
    {
        left[j]  = u - U[span + 1 - j];
        right[j] = U[span + j] - u;
        double saved = 0.0;

        for (int r = 0; r < j; ++r)
        {
            double temp = N[r] / (right[r + 1] + left[j - r]);
            N[r] = saved + right[r + 1] * temp;
            saved = left[j - r] * temp;
        }
        N[j] = saved;
    }
}


// Evaluate NURBS surface at (u, v)
// Only evaluates the (p+1)*(q+1) non-zero basis functions
// instead of all nu*nv. For degree 3 with 200 control points,
// this evaluates 16 terms instead of 40000.


inline bool evalSurfaceTry(const NurbsSurface& s, double u, double v, QVector3D& out)
{
    const int nu = s.K1 + 1;   // number of control points in u
    const int nv = s.K2 + 1;   // number of control points in v
    const int degU = s.M1;
    const int degV = s.M2;

    // Validate
    if (nu < 1 || nv < 1 || degU < 0 || degV < 0)
        return false;
    if ((int)s.U.size() < nu + degU + 1 || (int)s.V.size() < nv + degV + 1)
        return false;

    // Find knot spans
    int spanU = findSpan(nu - 1, degU, u, s.U);
    int spanV = findSpan(nv - 1, degV, v, s.V);

    // Compute non-zero basis functions (only p+1 values each)
    std::vector<double> Nu, Nv;
    basisFunctions(spanU, u, degU, s.U, Nu);
    basisFunctions(spanV, v, degV, s.V, Nv);

    // Evaluate: only loop over the (degU+1)*(degV+1) non-zero terms
    QVector3D num(0, 0, 0);
    double den = 0.0;

    for (int i = 0; i <= degU; ++i)
    {
        double NuVal = Nu[i];
        if (std::abs(NuVal) < 1e-15) continue;

        int ci = spanU - degU + i;  // control point index in u
        if (ci < 0 || ci >= nu) continue;

        for (int j = 0; j <= degV; ++j)
        {
            double NvVal = Nv[j];
            if (std::abs(NvVal) < 1e-15) continue;

            int cj = spanV - degV + j;  // control point index in v
            if (cj < 0 || cj >= nv) continue;

            // column-major order: V varies fastest
            int idx = cj * nu + ci;

            if (idx >= (int)s.W.size() || idx >= (int)s.P.size())
                continue;

            double w = s.W[idx];
            double B = NuVal * NvVal * w;

            num += s.P[idx] * float(B);
            den += B;
        }
    }

    if (std::abs(den) < 1e-10) return false;
    out = num / float(den);

    if (!std::isfinite(out.x()) || !std::isfinite(out.y()) || !std::isfinite(out.z()))
        return false;

    return true;
}
