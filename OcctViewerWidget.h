#ifndef OCCTVIEWERWIDGET_H
#define OCCTVIEWERWIDGET_H

#endif // OCCTVIEWERWIDGET_H

#pragma once

#include <QWidget>
#include <QPoint>

#include <TopoDS_Shape.hxx>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <OpenGl_GraphicDriver.hxx>

class OcctViewerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OcctViewerWidget(QWidget* parent = nullptr);

    void displayShape(const TopoDS_Shape& shape, bool fitAll = true);

protected:
    QPaintEngine* paintEngine() const override { return nullptr; }
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    void initOcct();

private:
    Handle(OpenGl_GraphicDriver) m_driver;
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;

    QPoint m_lastPos;
    enum class NavMode { None, Rotate, Pan };
    NavMode m_nav = NavMode::None;
};
