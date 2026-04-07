#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <vector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <utility>

class ViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit ViewerWidget(QWidget *parent = nullptr);
    ~ViewerWidget();

    // Data types
    struct ControlNet {
        std::vector<QVector3D> points;
        int nu;
        int nv;
    };

    // Vertex with position + normal (used by VBO)
    struct Vertex {
        float px, py, pz;   // position
        float nx, ny, nz;   // normal
    };

    // Per-patch GPU data
    struct PatchGPU {
        QOpenGLBuffer vbo;      // vertex buffer
        QOpenGLBuffer ibo;      // index buffer (triangles)
        QOpenGLBuffer wfIbo;    // index buffer (wireframe lines)
        int triCount = 0;       // number of triangle indices
        int lineCount = 0;      // number of wireframe line indices

        PatchGPU() : vbo(QOpenGLBuffer::VertexBuffer),
            ibo(QOpenGLBuffer::IndexBuffer),
            wfIbo(QOpenGLBuffer::IndexBuffer) {}
    };

    // Multi-body group
    struct BodyGroup {
        QString name;
        int firstPatch = 0;       // index into m_patchGPUs / m_surfacePatches
        int patchCount = 0;
        QMatrix4x4 modelMatrix;
    };

    // Public setters
    void setShowControlPoints(bool on);
    void setControlPoints(const std::vector<QVector3D>& points);
    void setShowNormals(bool on);
    void setFlipNormals(bool on);
    void setPatchColors(const std::vector<QVector3D>& colors);
    const std::vector<std::vector<QVector3D>>& getSurfacePatches() const
    { return m_surfacePatches; }
    void setSurfacePatches(const std::vector<std::vector<QVector3D>>& patches,
                           const std::vector<std::vector<QVector3D>>& normals,
                           const std::vector<std::pair<int,int>>& dims);
    void appendBodyPatches(const std::vector<std::vector<QVector3D>>& patches,
                           const std::vector<std::vector<QVector3D>>& normals,
                           const std::vector<std::pair<int,int>>& dims);

    void setShowSurface(bool on);
    void setUsePerspective(bool on);
    void fitToPoints(const std::vector<QVector3D>& points);
    void setGaussPoints(const std::vector<QVector3D>& points);
    void setGaussNormals(const std::vector<QVector3D>& normals);
    void setShowGaussPoints(bool on);
    void setShowGaussNormals(bool on);
    void setWireframe(bool on) { m_wireframe = on; update(); }
    void setControlNets(const std::vector<ControlNet>& nets);
    void setShowControlNet(bool on) { m_showControlNet = on; update(); }

    // Multi-body
    void setBodyGroups(const std::vector<BodyGroup>& groups);
    const std::vector<BodyGroup>& getBodyGroups() const { return m_bodyGroups; }
    void clearAll();   // wipe patches

    int getSelectedPatch() const { return m_selectedPatch; }
    void setSelectedPatch(int idx) { m_selectedPatch = idx; update(); }
    bool isFlipNormals() const { return m_flipNormals; }
    void flipPatchNormals(int patchIndex);

signals:
    void patchClicked(int patchIndex);

private:

    // Control net GPU data
    QOpenGLBuffer m_ctrlNetVbo;
    QOpenGLBuffer m_ctrlNetIbo;
    int m_ctrlNetLineCount = 0;
    QOpenGLBuffer m_ctrlPtsVbo;
    int m_ctrlPtsCount = 0;
    void buildControlNetBuffers();
    QOpenGLBuffer m_ctrlPtQuadVbo;
    int m_ctrlPtQuadCount = 0;

    void drawControlNetGL();

    // Simple color shader
    QOpenGLShaderProgram* m_flatShader = nullptr;

    QMatrix4x4 m_cachedMVP;
    QMatrix4x4 m_cachedView;
    QMatrix4x4 m_cachedProj;
    QOpenGLShaderProgram* m_shader = nullptr;

    // GPU surface data
    std::vector<PatchGPU*> m_patchGPUs;
    void buildSurfaceBuffers();
    void destroySurfaceBuffers();

    std::vector<std::vector<QVector3D>> m_surfacePatches;
    std::vector<std::vector<QVector3D>> m_surfaceNormals;
    std::vector<std::pair<int,int>> m_patchDims;

    // Camera / view
    float m_cameraDistance = 600.0f;
    float m_zoom = 1.0f;
    float m_rotX = 0.0f;
    float m_rotY = 0.0f;
    float m_scale = 1.0f;
    QVector3D m_center = QVector3D(0,0,0);
    float m_sceneRadius = 1.0f;
    bool m_usePerspective = true;

    QMatrix4x4 buildModelViewMatrix() const;
    QMatrix4x4 buildProjectionMatrix() const;
    QPointF projectPoint(const QVector3D& p) const;
    QPointF projectPointBody(const QVector3D& p, const QMatrix4x4& model) const;


    QPoint m_lastMousePos;
    bool m_mousePressed = false;
    bool m_mouseMoved = false;
    int pickPatch(const QPointF& clickPos);

    // Display toggles
    bool m_showSurface = true;
    bool m_showNormals = false;
    bool m_flipNormals = false;
    bool m_showControlPoints = false;
    bool m_wireframe = false;
    bool m_showGaussPoints = false;
    bool m_showGaussNormals = false;
    bool m_showControlNet = false;
    int m_selectedPatch = -1;

    // Multi-body groups
    std::vector<BodyGroup> m_bodyGroups;

    // Overlay data
    std::vector<QVector3D> m_controlPoints;
    std::vector<QVector3D> m_gaussPoints;
    std::vector<QVector3D> m_gaussNormals;   // per-gauss-point normals from .geom
    std::vector<ControlNet> m_controlNets;
    std::vector<QVector3D> m_patchColors;

    // Drawing helpers
    void drawSurfaceGL();
    void drawOverlays(QPainter& painter);
    void drawNormals(QPainter& painter);
    void drawGaussPoints(QPainter& painter);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif
