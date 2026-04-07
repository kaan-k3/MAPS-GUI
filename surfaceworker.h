#pragma once

#include <QObject>
#include <QVector3D>
#include <vector>
#include <utility>
#include "nurbs.h"

class SurfaceWorker : public QObject
{
    Q_OBJECT

public:
    explicit SurfaceWorker(QObject* parent = nullptr);
    std::string fileText;                // raw IGES text
    std::vector<int> nurbsPointers;      // entity 128 pointers from D-section
    std::vector<NurbsSurface> surfaces;  // OR parsed surfaces
    bool needsParsing = false;           // true = parse from fileText first

    struct BodyInfo {
        QString name;
        float x = 0, y = 0, angle = 0;
        int firstSurface = 0;   // index into surfaces vector
        int surfaceCount = 0;
    };
    std::vector<BodyInfo> bodyInfos;

signals:
    void progress(const QString& message);

    void finished(
        std::vector<std::vector<QVector3D>> patchPoints,
        std::vector<std::vector<QVector3D>> patchNormals,
        std::vector<std::pair<int,int>> patchDims,
        std::vector<QVector3D> allPts);

public slots:
    void process();
};

