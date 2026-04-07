#include "surfaceworker.h"
#include "nurbs_evaluation.h"
#include <cmath>
#include <limits>
#include "iges_parsing.h"


SurfaceWorker::SurfaceWorker(QObject* parent)
    : QObject(parent)
{
}

void SurfaceWorker::process()
{

        if (needsParsing && !fileText.empty())
    {
        emit progress("Parsing IGES entities...");
        surfaces.clear();

        for (int i = 0; i < (int)nurbsPointers.size(); ++i)
        {
            emit progress(QString("Parsing surface %1 of %2...")
                              .arg(i + 1).arg(nurbsPointers.size()));

            std::vector<std::string> tokens;
            if (!parsePSectionEntity(fileText, nurbsPointers[i], tokens)) continue;
            NurbsSurface s;
            if (!buildSurfaceFromTokens(tokens, s)) continue;
            surfaces.push_back(std::move(s));
        }

        fileText.clear();  // free memory
    }



    struct PatchData {
        std::vector<QVector3D> points;
        std::vector<QVector3D> normals;
        int uSamples;
        int vSamples;
    };

    std::vector<PatchData> allPatches;
    std::vector<QVector3D> allPts;

    for (int sIdx = 0; sIdx < (int)surfaces.size(); ++sIdx)
    {
        emit progress("Sampling surface " + QString::number(sIdx + 1)
                      + " of " + QString::number(surfaces.size()) + "...");

        const auto& surf = surfaces[sIdx];
        const int nu = surf.K1 + 1;
        const int nv = surf.K2 + 1;
        const int degU = surf.M1;
        const int degV = surf.M2;

        // sampling based on control point density
        int uSamples, vSamples;
        if (nu > 200 || nv > 200) {
            uSamples = 30; vSamples = 20;
        } else if (nu > 50 || nv > 50) {
            uSamples = 35; vSamples = 25;
        } else {
            uSamples = std::min(40, nu * 3);
            vSamples = std::min(30, nv * 3);
        }

        const int mU = (int)surf.U.size() - 1;
        const int mV = (int)surf.V.size() - 1;
        if (degU < 0 || degV < 0 || degU >= (int)surf.U.size() || degV >= (int)surf.V.size())
            continue;
        if (mU - degU < 0 || mV - degV < 0)
            continue;

        const double uMin = surf.U[degU];
        const double uMax = surf.U[mU - degU];
        const double vMin = surf.V[degV];
        const double vMax = surf.V[mV - degV];

        if (!(uMax > uMin) || !(vMax > vMin))
            continue;

        PatchData patch;
        patch.uSamples = uSamples;
        patch.vSamples = vSamples;
        patch.points.resize(uSamples * vSamples);
        patch.normals.resize(uSamples * vSamples);

        std::vector<QVector3D> validPts;
        validPts.reserve(uSamples * vSamples);

        const float NaN = std::numeric_limits<float>::quiet_NaN();
        const QVector3D nanVec(NaN, NaN, NaN);

        // evaluate all surface points
        for (int i = 0; i < uSamples; ++i)
        {
            const double u = uMin + (uMax - uMin) * i / (double)(uSamples - 1);
            for (int j = 0; j < vSamples; ++j)
            {
                const double v = vMin + (vMax - vMin) * j / (double)(vSamples - 1);
                int idx = i * vSamples + j;

                QVector3D pt;
                if (evalSurfaceTry(surf, u, v, pt) && isFiniteVec(pt))
                {
                    patch.points[idx] = pt;
                    validPts.push_back(pt);
                }
                else
                {
                    patch.points[idx] = nanVec;
                }
            }
        }

        // compute per-vertex normals from neighboring points
        for (int i = 0; i < uSamples; ++i)
        {
            for (int j = 0; j < vSamples; ++j)
            {
                int idx = i * vSamples + j;

                if (!isFiniteVec(patch.points[idx]))
                {
                    patch.normals[idx] = nanVec;
                    continue;
                }

                // Get neighbors for finite differences
                int ip = std::min(i + 1, uSamples - 1);
                int im = std::max(i - 1, 0);
                int jp = std::min(j + 1, vSamples - 1);
                int jm = std::max(j - 1, 0);

                const QVector3D& pUp = patch.points[ip * vSamples + j];
                const QVector3D& pUm = patch.points[im * vSamples + j];
                const QVector3D& pVp = patch.points[i * vSamples + jp];
                const QVector3D& pVm = patch.points[i * vSamples + jm];

                if (!isFiniteVec(pUp) || !isFiniteVec(pUm) ||
                    !isFiniteVec(pVp) || !isFiniteVec(pVm))
                {
                    patch.normals[idx] = QVector3D(0, 0, 1); // safe fallback
                    continue;
                }

                QVector3D du = pUp - pUm;
                QVector3D dv = pVp - pVm;
                QVector3D n = QVector3D::crossProduct(du, dv);

                float len = n.length();
                if (len > 1e-6f)
                    n /= len;
                else
                    n = QVector3D(0, 0, 1);

                patch.normals[idx] = n;
            }

        }

        allPatches.push_back(std::move(patch));
        allPts.insert(allPts.end(), validPts.begin(), validPts.end());

    }

    // Package results
    std::vector<std::vector<QVector3D>> patchPoints;
    std::vector<std::vector<QVector3D>> patchNormals;
    std::vector<std::pair<int,int>> patchDims;

    for (const auto& p : allPatches)
    {
        patchPoints.push_back(p.points);
        patchNormals.push_back(p.normals);
        patchDims.push_back({p.uSamples, p.vSamples});
    }

    emit finished(patchPoints, patchNormals, patchDims, allPts);
}
