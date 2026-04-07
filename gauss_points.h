#pragma once
#include "nurbs.h"
#include "nurbs_evaluation.h"
#include <vector>
#include <cmath>
#include <QVector3D>

struct GaussRule
{
    std::vector<double> xi;      // sample positions in [-1, +1]
    std::vector<double> w;       // corresponding weights
};

inline GaussRule getGaussRule(int n)
// returns the standard Gauss-Legendre points and weights for 1 through 5 points
{
    GaussRule rule;

    switch (n)
    {
    case 1:
        rule.xi = {0.0};
        rule.w  = {2.0};
        break;

    case 2:
        rule.xi = {-1.0 / std::sqrt(3.0),
                   1.0 / std::sqrt(3.0)};
        rule.w  = {1.0, 1.0};
        break;

    case 3:
        rule.xi = {-std::sqrt(3.0 / 5.0),
                   0.0,
                   std::sqrt(3.0 / 5.0)};
        rule.w  = {5.0 / 9.0,
                  8.0 / 9.0,
                  5.0 / 9.0};
        break;

    case 4:
        rule.xi = {-0.8611363115940526,
                   -0.3399810435848563,
                   0.3399810435848563,
                   0.8611363115940526};
        rule.w  = { 0.3478548451374538,
                  0.6521451548625461,
                  0.6521451548625461,
                  0.3478548451374538};
        break;

    case 5:
        rule.xi = {-0.9061798459386640,
                   -0.5384693101056831,
                   0.0,
                   0.5384693101056831,
                   0.9061798459386640};
        rule.w  = { 0.2369268850561891,
                  0.4786286704993665,
                  0.5688888888888889,
                  0.4786286704993665,
                  0.2369268850561891};
        break;

    default:
        // Fall back to 4-point rule for unsupported n
        return getGaussRule(4);
    }

    return rule;
}



// Map a Gauss point from [-1, +1] to an arbitrary [a, b]

//   value = a + (b - a) * (xi + 1) / 2

// When xi = -1  →  value = a       (left edge)
// When xi = +1  →  value = b       (right edge)


inline double mapGaussPoint(double xi, double a, double b)
{
    return a + (b - a) * (xi + 1.0) / 2.0;
}
// extracts the element boundaries from a knot vector

inline std::vector<double> uniqueKnotSpans(const std::vector<double>& knots, int degree)
{
    std::vector<double> spans;

    int first = degree;
    int last  = (int)knots.size() - 1 - degree;

    if (first > last) return spans;

    spans.push_back(knots[first]);
    for (int i = first + 1; i <= last; ++i)
    {
        if (knots[i] > spans.back() + 1e-10)
            spans.push_back(knots[i]);
    }

    return spans;
}



// For each knot span (element) in U and V, we place n×n Gauss
// points. Each Gauss point position in [-1,+1] gets mapped to
// the knot span's [a, b] range, then the NURBS surface is
// evaluated at that (u, v) to get the 3D point.
// Returns a vector of 3D points on the surface.

inline std::vector<QVector3D> computeGaussPoints(const NurbsSurface& surf, int numGaussPoints)
{
    std::vector<QVector3D> points;

    GaussRule rule = getGaussRule(numGaussPoints);

    // Get the unique knot span boundaries in each direction
    std::vector<double> uSpans = uniqueKnotSpans(surf.U, surf.M1);
    std::vector<double> vSpans = uniqueKnotSpans(surf.V, surf.M2);

    if (uSpans.size() < 2 || vSpans.size() < 2)
        return points;

    int numUElements = (int)uSpans.size() - 1;
    int numVElements = (int)vSpans.size() - 1;

    // Reserve space: numUElements * numVElements * n * n
    points.reserve(numUElements * numVElements * numGaussPoints * numGaussPoints);

    // Loop over each element (knot span pair)
    for (int eU = 0; eU < numUElements; ++eU)
    {
        double uA = uSpans[eU];       // element left edge in u
        double uB = uSpans[eU + 1];   // element right edge in u

        for (int eV = 0; eV < numVElements; ++eV)
        {
            double vA = vSpans[eV];       // element left edge in v
            double vB = vSpans[eV + 1];   // element right edge in v

            // Place n×n Gauss points in this element
            for (int gi = 0; gi < numGaussPoints; ++gi)
            {
                double u = mapGaussPoint(rule.xi[gi], uA, uB);

                for (int gj = 0; gj < numGaussPoints; ++gj)
                {
                    double v = mapGaussPoint(rule.xi[gj], vA, vB);

                    QVector3D pt;
                    if (evalSurfaceTry(surf, u, v, pt))
                        points.push_back(pt);
                }
            }
        }
    }

    return points;
}
