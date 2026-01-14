
#pragma once

#include <QWidget>
#include <QPoint>

/*
 * OcctViewerWidget
 *
 * A custom Qt widget that embeds an OpenCASCADE (OCCT) 3D viewer.
 * This widget is responsible for:
 *  - creating and managing the OCCT rendering context
 *  - displaying CAD geometry (TopoDS_Shape)
 *  - handling mouse interaction (rotate, pan, zoom)
 *
 * It is designed to be placed inside a Qt layout like any other QWidget.
 */


#include <TopoDS_Shape.hxx>
/*
 * OpenCASCADE visualization headers.
 * These classes form the OCCT 3D visualization stack:
 *  - Graphic driver (OpenGL backend)
 *  - Viewer and view
 *  - Interactive context for displayed objects
 */
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <OpenGl_GraphicDriver.hxx>

class OcctViewerWidget : public QWidget
{
    Q_OBJECT  // Enables Qt event handling and meta-object features
public:
    explicit OcctViewerWidget(QWidget* parent = nullptr); // Constructs the OCCT viewer widget. The parent parameter integrates with Qt's ownership system.

    void displayShape(const TopoDS_Shape& shape, bool fitAll = true); // Displays a CAD shape in the viewer. Has automatic size adjustment to fit screen.

protected:
    // Disable Qt's paint engine for this widget.
    // OpenCASCADE performs its own OpenGL rendering.

    QPaintEngine* paintEngine() const override { return nullptr; }


    void resizeEvent(QResizeEvent* e) override; // Qt event handlers overridden to synchronize OCCT rendering with widget resizing and redraw events.
    void paintEvent(QPaintEvent* e) override;

// Mouse interaction handlers used to implement panning, zooming, rotating.

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    // Initialize visualization pipeline for OpenGL drivers, viewer, and interactive content.
    void initOcct();

private:
    // Visualisation Objects to handle the lifetime of the rendering system

    Handle(OpenGl_GraphicDriver) m_driver;
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;

    QPoint m_lastPos; // tracking mouse movement and navigation
    enum class NavMode { None, Rotate, Pan };
    NavMode m_nav = NavMode::None;
};
