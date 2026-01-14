#include "OcctViewerWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
// OpenCascade visualisation Headers
#include <Aspect_DisplayConnection.hxx> // describes connection to the OS display system
#include <AIS_Shape.hxx> // a displayable (interactive) wrapper around TopoDS_Shape
// WNT_Window binds OCCT rendering to a native Windows HWND.
#include <WNT_Window.hxx>   // Windows-specific

#include <Prs3d_LineAspect.hxx> // Used to draw face boundary edges (shaded + edges look).
#include <Prs3d_Drawer.hxx>


OcctViewerWidget::OcctViewerWidget(QWidget* parent)
    : QWidget(parent)
{
    /*
     * These Qt attributes are important when embedding an external OpenGL renderer:
     * - WA_NativeWindow: ensure the widget has a real native window handle (HWND on Windows)
     * - WA_PaintOnScreen: Qt should not try to paint the widget using its own paint engine
     * - WA_NoSystemBackground: avoid Qt clearing/filling the background (reduces flicker)
     * - setAutoFillBackground(false): same idea (Qt won't try to fill)
     */

    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    initOcct(); // Visualisation system initialization
}

void OcctViewerWidget::initOcct()
{
    // Create a display connection and OpenGL driver.
    // The driver is OCCT's backend that actually talks to OpenGL.
    Handle(Aspect_DisplayConnection) display = new Aspect_DisplayConnection();
    m_driver = new OpenGl_GraphicDriver(display);

    m_viewer = new V3d_Viewer(m_driver); //  Create the OCCT viewer and turn on default lighting.
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_context = new AIS_InteractiveContext(m_viewer); //  AIS_InteractiveContext manages displayed objects (AIS_Shape),

    m_view = m_viewer->CreateView(); // Create a view (camera + viewport).


    /*
     * Bind the OCCT view to this QWidget's native window handle.
     * winId() returns a platform-specific handle; on Windows this is an HWND.
     * WNT_Window wraps that handle so OCCT can render into it.
     */

    Handle(WNT_Window) window = new WNT_Window((Aspect_Handle)winId());
    m_view->SetWindow(window);

    if (!window->IsMapped()) // Ensure the OS window is mapped (visible) before drawing into it.
        window->Map();

    m_view->SetBackgroundColor(Quantity_NOC_GRAY20); // background color is gray
    m_view->TriedronDisplay( // Setup the xyz axis indicator bottom left
        Aspect_TOTP_LEFT_LOWER,
        Quantity_NOC_WHITE,
        0.08,
        V3d_ZBUFFER
        );

    m_view->MustBeResized(); // Finalize sizing and request an initial redraw.
    m_view->Redraw(); // MustBeResized syncs OCCT's internal viewport with the widget size.
}

void OcctViewerWidget::displayShape(const TopoDS_Shape& shape, bool fitAll)
{
    if (shape.IsNull()) return; // ignore empty shape

    m_context->RemoveAll(false); // Clear previously displayed objects, then wrap the CAD shape in AIS_Shape, which is the OCCT presentation/interaction layer for TopoDS_Shape.
    Handle(AIS_Shape) ais = new AIS_Shape(shape);

    // Make shaded
    m_context->Display(ais, false);
    m_context->SetDisplayMode(ais, AIS_Shaded, false);

    // Turn on face boundaries (edges) for a better shaded look
    ais->Attributes()->SetFaceBoundaryDraw(true);
    ais->Attributes()->SetFaceBoundaryAspect(
        new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.0)
        );

    if (fitAll) m_view->FitAll(); // zoom fit
    m_view->Redraw(); //redraw with changes
}

void OcctViewerWidget::resizeEvent(QResizeEvent*)
{
    if (!m_view.IsNull()) // When the Qt widget changes size, notify OCCT so the viewport updates.
        m_view->MustBeResized(); // MustBeResized recalculates view projection and internal buffers.
}

void OcctViewerWidget::paintEvent(QPaintEvent*)
{
    if (!m_view.IsNull())   // delegate painting to OCCT and not QT
        m_view->Redraw();
}

void OcctViewerWidget::mousePressEvent(QMouseEvent* e)
{
    m_lastPos = e->pos(); // Record initial press position so we can compute deltas during dragging.
    //  Choose navigation mode based on which mouse button is pressed.

    if (e->button() == Qt::LeftButton)
        m_nav = NavMode::Rotate;
    else if (e->button() == Qt::MiddleButton)
        m_nav = NavMode::Pan;
}

void OcctViewerWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_view.IsNull()) return;   // If OCCT view isn't initialized, ignore input.

    QPoint p = e->pos();

    /*
     * Convert mouse motion into OCCT camera manipulation:
     * - Rotation uses absolute cursor position (OCCT tracks internally)
     * - Pan uses relative delta (current - last)
     */

    if (m_nav == NavMode::Rotate)
        m_view->Rotation(p.x(), p.y());
    else if (m_nav == NavMode::Pan)
        m_view->Pan(p.x() - m_lastPos.x(), m_lastPos.y() - p.y());

    m_lastPos = p;
}

void OcctViewerWidget::mouseReleaseEvent(QMouseEvent*)
{
    m_nav = NavMode::None;  // Stop navigation mode when mouse button is released.
}

void OcctViewerWidget::wheelEvent(QWheelEvent* e)

/*
     * Wheel input is used for zooming.
     * angleDelta().y() is positive/negative depending on scroll direction.
*/

{
    int delta = e->angleDelta().y();
    if (delta == 0) return;

    QPoint p = e->position().toPoint();
    m_view->ZoomAtPoint(
        p.x(),
        p.y(),
        p.x() + (delta > 0 ? 20 : -20),
        p.y() + (delta > 0 ? 20 : -20)
        );
}
