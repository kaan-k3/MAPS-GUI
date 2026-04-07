#include "viewerwidget.h"
#include <QOpenGLFunctions>
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QPolygonF>
#include <QtMath>
#include <cmath>
#include <QColor>
#include <QDebug>
#include "nurbs.h"


// Shader source code

static const char* vertexShaderSrc = R"(
    attribute highp vec3 aPos;
    attribute highp vec3 aNorm;
    uniform highp mat4 uMVP;
    uniform highp mat4 uModelView;
    uniform highp mat3 uNormalMatrix;
    varying highp vec3 vNormal;
    varying highp vec3 vFragPos;
    void main()
    {
        gl_Position = uMVP * vec4(aPos, 1.0);
        vNormal = normalize(uNormalMatrix * aNorm);
        vFragPos = vec3(uModelView * vec4(aPos, 1.0));
    }
)";

static const char* fragmentShaderSrc = R"(
    varying highp vec3 vNormal;
    varying highp vec3 vFragPos;
    uniform highp vec3 uLightDir;
    uniform highp vec3 uBaseColor;
    uniform highp float uAmbient;
    void main()
    {
        highp vec3 norm = normalize(vNormal);
        highp float diff = abs(dot(norm, normalize(uLightDir)));
        highp float intensity = uAmbient + (1.0 - uAmbient) * diff;
        highp vec3 color = uBaseColor * intensity;
        gl_FragColor = vec4(color, 1.0);
    }
)";

static const char* flatVertSrc = R"(
    attribute highp vec3 aPos;
    attribute highp vec2 aUV;
    uniform highp mat4 uMVP;
    uniform highp float uIsPoint;
    uniform highp vec2 uScreenSize;
    uniform highp float uPointPixels;
    varying highp vec2 vQuadUV;
    void main() {
        highp vec4 clip = uMVP * vec4(aPos, 1.0);
        if (uIsPoint > 0.5) {
            highp vec2 offset = aUV * uPointPixels / uScreenSize * clip.w;
            clip.xy += offset;
        }
        gl_Position = clip;
        vQuadUV = aUV;
    }
)";

static const char* flatFragSrc = R"(
    uniform highp vec3 uColor;
    uniform highp float uIsPoint;
    varying highp vec2 vQuadUV;
    void main() {
        if (uIsPoint > 0.5) {
            highp float dist = length(vQuadUV);
            if (dist > 0.7) discard;
        }
        gl_FragColor = vec4(uColor, 1.0);
    }
)";



static inline bool isPointVisible(const QVector4D& clip, int viewW, int viewH, float margin = 200.0f)
{
    // Behind the near plane or degenerate w
    if (clip.w() < 0.01f) return false;

    float ndcX = clip.x() / clip.w();
    float ndcY = clip.y() / clip.w();

    float sx = (ndcX * 0.5f + 0.5f) * viewW;
    float sy = (1.0f - (ndcY * 0.5f + 0.5f)) * viewH;

    return sx > -margin && sx < viewW + margin &&
           sy > -margin && sy < viewH + margin;
}


ViewerWidget::ViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    m_ctrlNetVbo(QOpenGLBuffer::VertexBuffer),
    m_ctrlNetIbo(QOpenGLBuffer::IndexBuffer),
    m_ctrlPtsVbo(QOpenGLBuffer::VertexBuffer),
    m_ctrlPtQuadVbo(QOpenGLBuffer::VertexBuffer)
{
}

ViewerWidget::~ViewerWidget()
{
    makeCurrent();
    destroySurfaceBuffers();
    if (m_ctrlNetVbo.isCreated()) m_ctrlNetVbo.destroy();
    if (m_ctrlNetIbo.isCreated()) m_ctrlNetIbo.destroy();
    if (m_ctrlPtsVbo.isCreated()) m_ctrlPtsVbo.destroy();
    if (m_ctrlPtQuadVbo.isCreated()) m_ctrlPtQuadVbo.destroy();
    delete m_shader;
    delete m_flatShader;
    doneCurrent();
}


void ViewerWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader = new QOpenGLShaderProgram(this);
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSrc))
        qDebug() << "Vertex shader error:" << m_shader->log();
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSrc))
        qDebug() << "Fragment shader error:" << m_shader->log();
    if (!m_shader->link())
        qDebug() << "Shader link error:" << m_shader->log();

    m_flatShader = new QOpenGLShaderProgram(this);
    m_flatShader->addShaderFromSourceCode(QOpenGLShader::Vertex, flatVertSrc);
    m_flatShader->addShaderFromSourceCode(QOpenGLShader::Fragment, flatFragSrc);
    if (!m_flatShader->link())
        qDebug() << "Flat shader link error:" << m_flatShader->log();
}

void ViewerWidget::resizeGL(int w, int h)
{
    const qreal dpr = devicePixelRatioF();
    glViewport(0, 0, int(w * dpr), int(h * dpr));
}


QMatrix4x4 ViewerWidget::buildProjectionMatrix() const
{
    float aspect = (float)width() / std::max(1, height());
    QMatrix4x4 proj;
    if (m_usePerspective)
        proj.perspective(45.0f, aspect, 1.0f, m_cameraDistance * 10.0f);
    else
    {
        float h = m_cameraDistance * 0.5f;
        proj.ortho(-h * aspect, h * aspect, -h, h, 1.0f, m_cameraDistance * 10.0f);
    }
    return proj;
}

QMatrix4x4 ViewerWidget::buildModelViewMatrix() const
{
    QMatrix4x4 mv;
    mv.translate(0, 0, -m_cameraDistance);
    mv.scale(m_zoom);
    mv.rotate(m_rotX, 1, 0, 0);
    mv.rotate(m_rotY, 0, 1, 0);
    mv.scale(m_scale);
    mv.translate(-m_center);
    return mv;
}



void ViewerWidget::destroySurfaceBuffers()
{
    for (auto* pg : m_patchGPUs)
    {
        if (pg->vbo.isCreated()) pg->vbo.destroy();
        if (pg->ibo.isCreated()) pg->ibo.destroy();
        if (pg->wfIbo.isCreated()) pg->wfIbo.destroy();
        delete pg;
    }
    m_patchGPUs.clear();
}

void ViewerWidget::buildSurfaceBuffers()
{
    makeCurrent();
    destroySurfaceBuffers();

    for (int pIdx = 0; pIdx < (int)m_surfacePatches.size(); ++pIdx)
    {
        const auto& patch = m_surfacePatches[pIdx];
        int pU = 0, pV = 0;
        if (pIdx < (int)m_patchDims.size()) {
            pU = m_patchDims[pIdx].first;
            pV = m_patchDims[pIdx].second;
        }
        if (pU * pV != (int)patch.size() || pU < 2 || pV < 2)
            continue;

        bool hasNormals = (pIdx < (int)m_surfaceNormals.size() &&
                           (int)m_surfaceNormals[pIdx].size() == pU * pV);


        std::vector<Vertex> verts(pU * pV);
        for (int i = 0; i < pU; ++i)
        {
            for (int j = 0; j < pV; ++j)
            {
                int idx = i * pV + j;
                const QVector3D& pos = patch[idx];
                Vertex& v = verts[idx];
                v.px = pos.x(); v.py = pos.y(); v.pz = pos.z();

                if (hasNormals && isFiniteVec(m_surfaceNormals[pIdx][idx]))
                {
                    const QVector3D& n = m_surfaceNormals[pIdx][idx];
                    v.nx = n.x(); v.ny = n.y(); v.nz = n.z();
                }
                else
                {
                    QVector3D norm(0, 0, 1);
                    int ip = std::min(i + 1, pU - 1);
                    int im = std::max(i - 1, 0);
                    int jp = std::min(j + 1, pV - 1);
                    int jm = std::max(j - 1, 0);
                    QVector3D du = patch[ip * pV + j] - patch[im * pV + j];
                    QVector3D dv = patch[i * pV + jp] - patch[i * pV + jm];
                    norm = QVector3D::crossProduct(du, dv);
                    float len = norm.length();
                    if (len > 1e-6f) norm /= len;
                    v.nx = norm.x(); v.ny = norm.y(); v.nz = norm.z();
                }
            }
        }
        std::vector<GLuint> triIdx;
        triIdx.reserve((pU - 1) * (pV - 1) * 6);
        for (int i = 0; i < pU - 1; ++i)
        {
            for (int j = 0; j < pV - 1; ++j)
            {
                GLuint i0 = i * pV + j;
                GLuint i1 = (i + 1) * pV + j;
                GLuint i2 = (i + 1) * pV + (j + 1);
                GLuint i3 = i * pV + (j + 1);

                if (!isFiniteVec(patch[i0]) || !isFiniteVec(patch[i1]) ||
                    !isFiniteVec(patch[i2]) || !isFiniteVec(patch[i3]))
                    continue;


                triIdx.push_back(i0);
                triIdx.push_back(i1);
                triIdx.push_back(i2);

                triIdx.push_back(i0);
                triIdx.push_back(i2);
                triIdx.push_back(i3);
            }
        }

        std::vector<GLuint> wireIdx;
        wireIdx.reserve((pU - 1) * (pV - 1) * 8);
        for (int i = 0; i < pU - 1; ++i)
        {
            for (int j = 0; j < pV - 1; ++j)
            {
                GLuint i0 = i * pV + j;
                GLuint i1 = (i + 1) * pV + j;
                GLuint i2 = (i + 1) * pV + (j + 1);
                GLuint i3 = i * pV + (j + 1);

                if (!isFiniteVec(patch[i0]) || !isFiniteVec(patch[i1]) ||
                    !isFiniteVec(patch[i2]) || !isFiniteVec(patch[i3]))
                    continue;

                wireIdx.push_back(i0); wireIdx.push_back(i1);
                wireIdx.push_back(i1); wireIdx.push_back(i2);
                wireIdx.push_back(i2); wireIdx.push_back(i3);
                wireIdx.push_back(i3); wireIdx.push_back(i0);
            }
        }

        if (triIdx.empty()) continue;

        PatchGPU* pg = new PatchGPU();


        pg->vbo.create();
        pg->vbo.bind();
        pg->vbo.allocate(verts.data(), (int)(verts.size() * sizeof(Vertex)));
        pg->vbo.release();

        pg->ibo.create();
        pg->ibo.bind();
        pg->ibo.allocate(triIdx.data(), (int)(triIdx.size() * sizeof(GLuint)));
        pg->ibo.release();
        pg->triCount = (int)triIdx.size();

        pg->wfIbo.create();
        pg->wfIbo.bind();
        pg->wfIbo.allocate(wireIdx.data(), (int)(wireIdx.size() * sizeof(GLuint)));
        pg->wfIbo.release();
        pg->lineCount = (int)wireIdx.size();

        m_patchGPUs.push_back(pg);
    }

    doneCurrent();
    qDebug() << "Built GPU buffers for" << m_patchGPUs.size() << "patches";
}

void ViewerWidget::buildControlNetBuffers()
{
    makeCurrent();

    if (m_ctrlNetVbo.isCreated()) m_ctrlNetVbo.destroy();
    if (m_ctrlNetIbo.isCreated()) m_ctrlNetIbo.destroy();
    if (m_ctrlPtsVbo.isCreated()) m_ctrlPtsVbo.destroy();
    m_ctrlNetLineCount = 0;
    m_ctrlPtsCount = 0;

    if (m_controlNets.empty()) { doneCurrent(); return; }

    std::vector<float> allVerts;       // x,y,z packed
    std::vector<GLuint> lineIndices;

    for (const auto& net : m_controlNets)
    {
        int baseIdx = (int)(allVerts.size() / 3);

        for (const auto& p : net.points)
        {
            allVerts.push_back(p.x());
            allVerts.push_back(p.y());
            allVerts.push_back(p.z());
        }

        // U-direction lines
        for (int iv = 0; iv < net.nv; ++iv)
            for (int iu = 0; iu < net.nu - 1; ++iu)
            {
                int idx0 = iv * net.nu + iu;
                int idx1 = iv * net.nu + iu + 1;
                if (idx1 >= (int)net.points.size()) continue;
                lineIndices.push_back(baseIdx + idx0);
                lineIndices.push_back(baseIdx + idx1);
            }

        // V-direction lines
        for (int iu = 0; iu < net.nu; ++iu)
            for (int iv = 0; iv < net.nv - 1; ++iv)
            {
                int idx0 = iv * net.nu + iu;
                int idx1 = (iv + 1) * net.nu + iu;
                if (idx1 >= (int)net.points.size()) continue;
                lineIndices.push_back(baseIdx + idx0);
                lineIndices.push_back(baseIdx + idx1);
            }
    }

    if (allVerts.empty()) { doneCurrent(); return; }

    // Points VBO
    m_ctrlPtsVbo.create();
    m_ctrlPtsVbo.bind();
    m_ctrlPtsVbo.allocate(allVerts.data(), (int)(allVerts.size() * sizeof(float)));
    m_ctrlPtsVbo.release();
    m_ctrlPtsCount = (int)(allVerts.size() / 3);

    // Net VBO
    m_ctrlNetVbo.create();
    m_ctrlNetVbo.bind();
    m_ctrlNetVbo.allocate(allVerts.data(), (int)(allVerts.size() * sizeof(float)));
    m_ctrlNetVbo.release();

    // Net IBO
    m_ctrlNetIbo.create();
    m_ctrlNetIbo.bind();
    m_ctrlNetIbo.allocate(lineIndices.data(), (int)(lineIndices.size() * sizeof(GLuint)));
    m_ctrlNetIbo.release();
    m_ctrlNetLineCount = (int)lineIndices.size();

    {
        std::vector<float> quadVerts;  // x,y,z, u,v per vertex

        for (const auto& net : m_controlNets)
        {
            for (const auto& p : net.points)
            {
                if (!isFiniteVec(p)) continue;

                float x = p.x(), y = p.y(), z = p.z();


                // Triangle 1
                quadVerts.push_back(x); quadVerts.push_back(y); quadVerts.push_back(z);
                quadVerts.push_back(-1.0f); quadVerts.push_back(-1.0f);
                quadVerts.push_back(x); quadVerts.push_back(y); quadVerts.push_back(z);
                quadVerts.push_back(1.0f); quadVerts.push_back(-1.0f);
                quadVerts.push_back(x); quadVerts.push_back(y); quadVerts.push_back(z);
                quadVerts.push_back(1.0f); quadVerts.push_back(1.0f);
                // Triangle 2
                quadVerts.push_back(x); quadVerts.push_back(y); quadVerts.push_back(z);
                quadVerts.push_back(-1.0f); quadVerts.push_back(-1.0f);
                quadVerts.push_back(x); quadVerts.push_back(y); quadVerts.push_back(z);
                quadVerts.push_back(1.0f); quadVerts.push_back(1.0f);
                quadVerts.push_back(x); quadVerts.push_back(y); quadVerts.push_back(z);
                quadVerts.push_back(-1.0f); quadVerts.push_back(1.0f);
            }
        }

        if (m_ctrlPtQuadVbo.isCreated()) m_ctrlPtQuadVbo.destroy();
        if (!quadVerts.empty())
        {
            m_ctrlPtQuadVbo.create();
            m_ctrlPtQuadVbo.bind();
            m_ctrlPtQuadVbo.allocate(quadVerts.data(), (int)(quadVerts.size() * sizeof(float)));
            m_ctrlPtQuadVbo.release();
        }
        m_ctrlPtQuadCount = (int)(quadVerts.size() / 5);
    }
    doneCurrent();
}




void ViewerWidget::drawSurfaceGL()
{
    if (!m_shader || m_patchGPUs.empty()) return;

    QMatrix4x4 view = buildModelViewMatrix();
    QMatrix4x4 proj = buildProjectionMatrix();

    m_shader->bind();
    m_shader->setUniformValue("uLightDir", QVector3D(0.4f, 0.5f, 0.8f).normalized());
    m_shader->setUniformValue("uAmbient", 0.25f);

    int posLoc = m_shader->attributeLocation("aPos");
    int normLoc = m_shader->attributeLocation("aNorm");


    auto drawPatchRange = [&](int first, int count, const QMatrix4x4& model) {
        QMatrix4x4 mv = view * model;
        QMatrix4x4 mvp = proj * mv;
        QMatrix3x3 normalMat = mv.normalMatrix();

        m_shader->setUniformValue("uMVP", mvp);
        m_shader->setUniformValue("uModelView", mv);
        m_shader->setUniformValue("uNormalMatrix", normalMat);

        int last = std::min(first + count, (int)m_patchGPUs.size());
        for (int i = first; i < last; ++i)
        {
            PatchGPU* pg = m_patchGPUs[i];

            if (i == m_selectedPatch)
                m_shader->setUniformValue("uBaseColor", QVector3D(0.9f, 0.55f, 0.15f));
            else if (i < (int)m_patchColors.size())
                m_shader->setUniformValue("uBaseColor", m_patchColors[i]);
            else
                m_shader->setUniformValue("uBaseColor", QVector3D(0.35f, 0.55f, 0.85f));

            pg->vbo.bind();
            m_shader->enableAttributeArray(posLoc);
            m_shader->setAttributeBuffer(posLoc, GL_FLOAT, 0, 3, sizeof(Vertex));
            m_shader->enableAttributeArray(normLoc);
            m_shader->setAttributeBuffer(normLoc, GL_FLOAT, 3 * sizeof(float), 3, sizeof(Vertex));

            if (m_wireframe)
            {
                pg->wfIbo.bind();
                glDrawElements(GL_LINES, pg->lineCount, GL_UNSIGNED_INT, nullptr);
                pg->wfIbo.release();
            }
            else
            {
                pg->ibo.bind();
                glDrawElements(GL_TRIANGLES, pg->triCount, GL_UNSIGNED_INT, nullptr);
                pg->ibo.release();
            }

            m_shader->disableAttributeArray(posLoc);
            m_shader->disableAttributeArray(normLoc);
            pg->vbo.release();
        }
    };

    if (m_bodyGroups.empty())
    {
        QMatrix4x4 identity;
        drawPatchRange(0, (int)m_patchGPUs.size(), identity);
    }
    else
    {
        for (const auto& bg : m_bodyGroups)
            drawPatchRange(bg.firstPatch, bg.patchCount, bg.modelMatrix);
    }

    m_shader->release();
}

void ViewerWidget::drawNormals(QPainter& painter)
{
    if (!m_showNormals || m_surfacePatches.empty()) return;

    painter.setPen(QPen(QColor(255, 100, 100), 2));

    const int maxPerDir = 3;
    auto modelForPatch = [&](int pIdx) -> QMatrix4x4 {
        for (const auto& bg : m_bodyGroups)
        {
            if (pIdx >= bg.firstPatch && pIdx < bg.firstPatch + bg.patchCount)
                return bg.modelMatrix;
        }
        return QMatrix4x4(); // identity
    };

    int vw = width();
    int vh = height();

    for (int pIdx = 0; pIdx < (int)m_surfacePatches.size(); ++pIdx)
    {
        const auto& patch = m_surfacePatches[pIdx];

        if (pIdx >= (int)m_patchDims.size()) continue;
        int patchU = m_patchDims[pIdx].first;
        int patchV = m_patchDims[pIdx].second;

        if ((int)patch.size() != patchU * patchV || patchU < 2 || patchV < 2) continue;


        QMatrix4x4 model = modelForPatch(pIdx);
        QMatrix4x4 normalMvp = m_bodyGroups.empty()
            ? m_cachedMVP
            : m_cachedProj * m_cachedView * model;

        auto proj = [&](const QVector3D& p) -> QPointF {
            return m_bodyGroups.empty() ? projectPoint(p)
                                        : projectPointBody(p, model);
        };

        if (pIdx == m_selectedPatch)
        {
            painter.setPen(QPen(QColor(255, 165, 0), 2));
        }
        else if (pIdx < (int)m_patchColors.size())
        {
            QColor c(m_patchColors[pIdx].x() * 255,
                     m_patchColors[pIdx].y() * 255,
                     m_patchColors[pIdx].z() * 255);

            painter.setPen(QPen(c.lighter(150), 2));
        }
        else
        {
            painter.setPen(QPen(QColor(255, 100, 100), 2));
        }


        bool hasNorms = (pIdx < (int)m_surfaceNormals.size() &&
                         (int)m_surfaceNormals[pIdx].size() == patchU * patchV);

        int stepsU = std::min(maxPerDir, patchU - 2);
        int stepsV = std::min(maxPerDir, patchV - 2);
        if (stepsU < 1) stepsU = 1;
        if (stepsV < 1) stepsV = 1;

        for (int si = 0; si <= stepsU; ++si)
        {

            int i = 1 + si * (patchU - 3) / std::max(1, stepsU);
            i = std::max(1, std::min(i, patchU - 2));

            for (int sj = 0; sj <= stepsV; ++sj)
            {
                int j = 1 + sj * (patchV - 3) / std::max(1, stepsV);
                j = std::max(1, std::min(j, patchV - 2));

                int idx = i * patchV + j;
                const QVector3D& p = patch[idx];
                if (!isFiniteVec(p)) continue;

               // skip points behind camera or off-screen
                QVector4D clip = normalMvp * QVector4D(p, 1.0f);
                if (!isPointVisible(clip, vw, vh))
                    continue;

                QVector3D n;
                if (hasNorms && isFiniteVec(m_surfaceNormals[pIdx][idx]))
                {
                    n = m_surfaceNormals[pIdx][idx];
                }
                else
                {
                    int ip = std::min(i + 1, patchU - 1), im = std::max(i - 1, 0);
                    int jp = std::min(j + 1, patchV - 1), jm = std::max(j - 1, 0);
                    QVector3D du = patch[ip * patchV + j] - patch[im * patchV + j];
                    QVector3D dv = patch[i * patchV + jp] - patch[i * patchV + jm];
                    n = QVector3D::crossProduct(du, dv);
                    float len = n.length();
                    if (len < 1e-6f) continue;
                    n /= len;
                }

                if (m_flipNormals) n = -n;

                QPointF a = proj(p);
                QMatrix4x4 viewForNorm = m_bodyGroups.empty()
                    ? m_cachedView
                    : m_cachedView * model;
                QVector4D nView4 = viewForNorm * QVector4D(n, 0.0f);
                float vx = nView4.x();
                float vy = -nView4.y();  // flip Y for screen coords
                float vz = nView4.z();
                float screenMag = std::sqrt(vx * vx + vy * vy);

                if (screenMag < 0.01f) continue;

                float ux = vx / screenMag;
                float uy = vy / screenMag;
                float foreshorten = screenMag /
                    std::sqrt(vx * vx + vy * vy + vz * vz);
                float arrowLen = 40.0f * std::max(foreshorten, 0.25f);

                QPointF b(a.x() + ux * arrowLen, a.y() + uy * arrowLen);

                painter.drawLine(a, b);

                // Arrow head
                if (arrowLen > 5.0f)
                {
                    QVector2D dir(ux, uy);
                    QVector2D perp(-uy, ux);
                    float arrSz = 6.0f;
                    QPointF w1 = b - (dir * arrSz).toPointF() + (perp * arrSz * 0.4f).toPointF();
                    QPointF w2 = b - (dir * arrSz).toPointF() - (perp * arrSz * 0.4f).toPointF();
                    painter.drawLine(b, w1);
                    painter.drawLine(b, w2);
                }
            }
        }
    }
}

void ViewerWidget::setPatchColors(const std::vector<QVector3D>& colors)
{
    m_patchColors = colors;
    update();
}

void ViewerWidget::flipPatchNormals(int patchIndex)
{
    if (patchIndex < 0 || patchIndex >= (int)m_surfaceNormals.size())
        return;

    for (auto& n : m_surfaceNormals[patchIndex])
        n = -n;
    buildSurfaceBuffers();
    update();
}

void ViewerWidget::drawControlNetGL()
{
    if (!m_flatShader) return;

    QMatrix4x4 mvp = buildProjectionMatrix() * buildModelViewMatrix();

    m_flatShader->bind();
    m_flatShader->setUniformValue("uMVP", mvp);

    int posLoc = m_flatShader->attributeLocation("aPos");

    if (m_showControlNet && m_ctrlNetLineCount > 0)
    {
        m_flatShader->setUniformValue("uColor", QVector3D(1.0f, 0.78f, 0.0f));  // yellow
        m_flatShader->setUniformValue("uIsPoint", 0.0f);

        m_ctrlNetVbo.bind();
        m_flatShader->enableAttributeArray(posLoc);
        m_flatShader->setAttributeBuffer(posLoc, GL_FLOAT, 0, 3, 3 * sizeof(float));

        m_ctrlNetIbo.bind();
        glDrawElements(GL_LINES, m_ctrlNetLineCount, GL_UNSIGNED_INT, nullptr);
        m_ctrlNetIbo.release();

        m_flatShader->disableAttributeArray(posLoc);
        m_ctrlNetVbo.release();
    }


    if (m_showControlPoints && m_ctrlPtQuadVbo.isCreated() && m_ctrlPtQuadCount > 0)
    {
        m_flatShader->setUniformValue("uColor", QVector3D(1.0f, 0.4f, 0.0f));
        m_flatShader->setUniformValue("uIsPoint", 1.0f);
        m_flatShader->setUniformValue("uScreenSize", QVector2D((float)width(), (float)height()));
        m_flatShader->setUniformValue("uPointPixels", 2.0f);  // 4 pixel radius

        int uvLoc = m_flatShader->attributeLocation("aUV");

        m_ctrlPtQuadVbo.bind();
        m_flatShader->enableAttributeArray(posLoc);
        m_flatShader->setAttributeBuffer(posLoc, GL_FLOAT, 0, 3, 5 * sizeof(float));
        m_flatShader->enableAttributeArray(uvLoc);
        m_flatShader->setAttributeBuffer(uvLoc, GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));

        glDrawArrays(GL_TRIANGLES, 0, m_ctrlPtQuadCount);

        m_flatShader->disableAttributeArray(posLoc);
        m_flatShader->disableAttributeArray(uvLoc);
        m_ctrlPtQuadVbo.release();
    }
}

void ViewerWidget::drawGaussPoints(QPainter& painter)
{
    if (!m_showGaussPoints || m_gaussPoints.empty()) return;

    bool hasNormals = m_showGaussNormals &&
                      ((int)m_gaussNormals.size() == (int)m_gaussPoints.size());
    static bool loggedOnce = false;
    if (m_showGaussNormals && !hasNormals && !loggedOnce)
    {
        qDebug() << "Gauss normals: showGaussNormals=" << m_showGaussNormals
                 << "points=" << m_gaussPoints.size()
                 << "normals=" << m_gaussNormals.size()
                 << "(size mismatch — no arrows will draw)";
        loggedOnce = true;
    }
    if (hasNormals) loggedOnce = false;

    int vw = width();
    int vh = height();

    // Arrow length in pixels
    const float desiredPx = 35.0f;

    for (int i = 0; i < (int)m_gaussPoints.size(); ++i)
    {
        const auto& p = m_gaussPoints[i];
        if (!isFiniteVec(p)) continue;

        // skip points behind camera or off-screen
        QVector4D clip = m_cachedMVP * QVector4D(p, 1.0f);
        if (!isPointVisible(clip, vw, vh))
            continue;

        QPointF sp = projectPoint(p);

        painter.setPen(QPen(QColor(0, 255, 100), 1));
        painter.setBrush(QColor(0, 255, 100));
        painter.drawEllipse(sp, 3, 3);

        // draw normal arrow
        if (hasNormals && isFiniteVec(m_gaussNormals[i]))
        {
            float nLen = m_gaussNormals[i].length();
            if (nLen < 1e-8f) continue;
            QVector3D nDir = m_gaussNormals[i] / nLen;

            // transform normal direction into view space (w=0 for direction)
            QVector4D nView4 = m_cachedView * QVector4D(nDir, 0.0f);

            float vx = nView4.x();
            float vy = -nView4.y();
            float vz = nView4.z();  // depth component (toward/away from camera)
            float screenMag = std::sqrt(vx * vx + vy * vy);

            if (screenMag < 0.01f)
            {
                painter.setPen(QPen(QColor(255, 160, 60), 1.5));
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(sp, 4, 4);
                continue;
            }

            // Normalize to get screen-space direction
            float ux = vx / screenMag;
            float uy = vy / screenMag;
            float foreshorten = std::sqrt(vx * vx + vy * vy) /
                                std::sqrt(vx * vx + vy * vy + vz * vz);
            float arrowLen = desiredPx * std::max(foreshorten, 0.25f);

            QPointF sp2(sp.x() + ux * arrowLen,
                        sp.y() + uy * arrowLen);

            // Skip if the arrow tip is way off-screen
            if (sp2.x() < -150 || sp2.x() > vw + 150 ||
                sp2.y() < -150 || sp2.y() > vh + 150)
                continue;

            // Shaft
            painter.setPen(QPen(QColor(255, 160, 60), 1.5));
            painter.drawLine(sp, sp2);

            // Arrowhead triangle
            if (arrowLen > 6.0f)
            {
                double headSize = std::min((double)arrowLen * 0.22, 7.0);
                QPointF unit(ux, uy);
                QPointF perp(-uy, ux);

                QPointF base = sp2 - unit * headSize;
                QPolygonF head;
                head << sp2
                     << (base + perp * headSize * 0.5)
                     << (base - perp * headSize * 0.5);

                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(255, 160, 60));
                painter.drawPolygon(head);
            }
        }
    }
}

void ViewerWidget::drawOverlays(QPainter& painter)
{
    drawNormals(painter);
    drawGaussPoints(painter);
}

// main render loop

void ViewerWidget::paintGL()
{
    m_cachedView = buildModelViewMatrix();
    m_cachedProj = buildProjectionMatrix();
    m_cachedMVP  = m_cachedProj * m_cachedView;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (m_showSurface)
        drawSurfaceGL();
    glDisable(GL_DEPTH_TEST);
    // Control net/points via GPU
    drawControlNetGL();

    // Remaining overlays via QPainter (normals, gauss points)
    const qreal dpr = devicePixelRatioF();
    QOpenGLPaintDevice device(QSize(int(width() * dpr), int(height() * dpr)));
    device.setDevicePixelRatio(dpr);

    QPainter painter(&device);
    painter.setRenderHint(QPainter::Antialiasing);

    drawNormals(painter);
    drawGaussPoints(painter);

    painter.end();
}



QPointF ViewerWidget::projectPoint(const QVector3D& p) const
{
    QVector4D clip = m_cachedMVP * QVector4D(p, 1.0f);

    if (std::abs(clip.w()) < 1e-6f)
        return QPointF(-9999, -9999);

    float ndcX = clip.x() / clip.w();
    float ndcY = clip.y() / clip.w();

    float screenX = (ndcX * 0.5f + 0.5f) * width();
    float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * height();

    return QPointF(screenX, screenY);
}


QPointF ViewerWidget::projectPointBody(const QVector3D& p, const QMatrix4x4& model) const
{
    QMatrix4x4 mvp = m_cachedProj * m_cachedView * model;
    QVector4D clip = mvp * QVector4D(p, 1.0f);

    if (std::abs(clip.w()) < 1e-6f)
        return QPointF(-9999, -9999);

    float ndcX = clip.x() / clip.w();
    float ndcY = clip.y() / clip.w();

    float screenX = (ndcX * 0.5f + 0.5f) * width();
    float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * height();

    return QPointF(screenX, screenY);
}


void ViewerWidget::setSurfacePatches(const std::vector<std::vector<QVector3D>>& patches,
                                     const std::vector<std::vector<QVector3D>>& normals,
                                     const std::vector<std::pair<int,int>>& dims)
{
    m_surfacePatches = patches;
    m_surfaceNormals = normals;
    m_patchDims = dims;
    m_showSurface = true;

    buildSurfaceBuffers();

    qDebug() << "Received" << patches.size() << "patches with per-vertex normals";
    update();
}

void ViewerWidget::setShowControlPoints(bool on)
{
    m_showControlPoints = on;
    update();
}

void ViewerWidget::setControlPoints(const std::vector<QVector3D>& points)
{
    m_controlPoints = points;
}

void ViewerWidget::setControlNets(const std::vector<ControlNet>& nets)
{
    m_controlNets = nets;
    buildControlNetBuffers();
    update();
}

void ViewerWidget::setShowNormals(bool on)
{
    m_showNormals = on;
    update();
}

void ViewerWidget::setFlipNormals(bool on)
{
    m_flipNormals = on;
    update();
}

void ViewerWidget::setShowSurface(bool on)
{
    m_showSurface = on;
    update();
}

void ViewerWidget::setUsePerspective(bool on)
{
    m_usePerspective = on;
    update();
}

void ViewerWidget::fitToPoints(const std::vector<QVector3D>& points)
{
    if (points.empty()) return;

    QVector3D mn = points[0], mx = points[0];
    for (const auto& p : points)
    {
        if (!std::isfinite(p.x()) || !std::isfinite(p.y()) || !std::isfinite(p.z()))
            continue;
        mn.setX(std::min(mn.x(), p.x()));
        mn.setY(std::min(mn.y(), p.y()));
        mn.setZ(std::min(mn.z(), p.z()));
        mx.setX(std::max(mx.x(), p.x()));
        mx.setY(std::max(mx.y(), p.y()));
        mx.setZ(std::max(mx.z(), p.z()));
    }

    m_center = (mn + mx) * 0.5f;
    QVector3D extent = mx - mn;
    m_sceneRadius = 0.5f * extent.length();
    if (m_sceneRadius < 1.0f) m_sceneRadius = 1.0f;

    float targetSize = 0.45f * std::min(width(), height());
    m_scale = targetSize / m_sceneRadius;
    m_cameraDistance = targetSize * 1.5f;

    m_zoom = 1.0f;
    m_rotX = 20.0f;
    m_rotY = -30.0f;

    qDebug() << "Fit: center" << m_center << "extent" << extent << "radius" << m_sceneRadius;
    update();
}

void ViewerWidget::setGaussPoints(const std::vector<QVector3D>& points)
{
    m_gaussPoints = points;
    update();
}

void ViewerWidget::setGaussNormals(const std::vector<QVector3D>& normals)
{
    m_gaussNormals = normals;
    update();
}

void ViewerWidget::setShowGaussPoints(bool on)
{
    m_showGaussPoints = on;
    update();
}

void ViewerWidget::setShowGaussNormals(bool on)
{
    m_showGaussNormals = on;
    update();
}

void ViewerWidget::setBodyGroups(const std::vector<BodyGroup>& groups)
{
    m_bodyGroups = groups;
    buildSurfaceBuffers();
    update();
}

void ViewerWidget::clearAll()
{
    makeCurrent();
    destroySurfaceBuffers();
    doneCurrent();

    m_surfacePatches.clear();
    m_surfaceNormals.clear();
    m_patchDims.clear();
    m_patchColors.clear();
    m_bodyGroups.clear();
    m_gaussPoints.clear();
    m_gaussNormals.clear();
    m_controlNets.clear();
    m_controlPoints.clear();
    m_selectedPatch = -1;

    update();
}

void ViewerWidget::appendBodyPatches(
    const std::vector<std::vector<QVector3D>>& patches,
    const std::vector<std::vector<QVector3D>>& normals,
    const std::vector<std::pair<int,int>>& dims)
{
    m_surfacePatches.insert(m_surfacePatches.end(), patches.begin(), patches.end());
    m_surfaceNormals.insert(m_surfaceNormals.end(), normals.begin(), normals.end());
    m_patchDims.insert(m_patchDims.end(), dims.begin(), dims.end());

    buildSurfaceBuffers();
    update();
}




void ViewerWidget::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y();
    m_zoom *= (delta > 0) ? 1.1f : 0.9f;
    m_zoom = std::max(0.1f, std::min(20.0f, m_zoom));
    update();
}

void ViewerWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    m_mousePressed = true;
    m_mouseMoved = false;

    if (event->button() == Qt::RightButton)
    {
        if (m_selectedPatch != -1)
        {
            m_selectedPatch = -1;
            update();
            emit patchClicked(-1);
        }
    }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mouseMoved = true;
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    m_rotY += delta.x() * 0.5f;
    m_rotX += delta.y() * 0.5f;
    update();
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_mouseMoved)
    {
        QPointF clickPos = event->pos();
        int hitPatch = pickPatch(clickPos);

        if (hitPatch != m_selectedPatch)
        {
            m_selectedPatch = hitPatch;
            update();
            emit patchClicked(hitPatch);
        }
    }
    m_mousePressed = false;
}

int ViewerWidget::pickPatch(const QPointF& clickPos)
{

    auto modelForPatch = [&](int pIdx) -> QMatrix4x4 {
        for (const auto& bg : m_bodyGroups)
        {
            if (pIdx >= bg.firstPatch && pIdx < bg.firstPatch + bg.patchCount)
                return bg.modelMatrix;
        }
        return QMatrix4x4(); // identity
    };

    for (int pIdx = (int)m_surfacePatches.size() - 1; pIdx >= 0; --pIdx)
    {
        const auto& patch = m_surfacePatches[pIdx];
        if (pIdx >= (int)m_patchDims.size()) continue;
        int pU = m_patchDims[pIdx].first;
        int pV = m_patchDims[pIdx].second;

        if ((int)patch.size() != pU * pV) continue;

        QMatrix4x4 model = modelForPatch(pIdx);
        auto proj = [&](const QVector3D& p) -> QPointF {
            return m_bodyGroups.empty() ? projectPoint(p)
                                        : projectPointBody(p, model);
        };

        int stepU = std::max(1, pU / 20);
        int stepV = std::max(1, pV / 20);

        for (int i = 0; i < pU - 1; i += stepU)
        {
            for (int j = 0; j < pV - 1; j += stepV)
            {
                int idx0 = i * pV + j;
                int idx1 = (i + 1) * pV + j;
                int idx2 = (i + 1) * pV + (j + 1);
                int idx3 = i * pV + (j + 1);

                const QVector3D& p0 = patch[idx0];
                const QVector3D& p1 = patch[idx1];
                const QVector3D& p2 = patch[idx2];
                const QVector3D& p3 = patch[idx3];

                if (!isFiniteVec(p0) || !isFiniteVec(p1) || !isFiniteVec(p2) || !isFiniteVec(p3))
                    continue;

                QPolygonF quad;
                quad << proj(p0) << proj(p1)
                     << proj(p2) << proj(p3);

                if (quad.containsPoint(clickPos, Qt::OddEvenFill))
                    return pIdx;
            }
        }
    }
    return -1;
}
