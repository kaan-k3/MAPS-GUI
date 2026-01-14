#include "OcctViewerWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <Aspect_DisplayConnection.hxx>
#include <AIS_Shape.hxx>

#include <WNT_Window.hxx>   // Windows-specific

#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>


OcctViewerWidget::OcctViewerWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    initOcct();
}

void OcctViewerWidget::initOcct()
{
    Handle(Aspect_DisplayConnection) display = new Aspect_DisplayConnection();
    m_driver = new OpenGl_GraphicDriver(display);

    m_viewer = new V3d_Viewer(m_driver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_context = new AIS_InteractiveContext(m_viewer);

    m_view = m_viewer->CreateView();

    Handle(WNT_Window) window = new WNT_Window((Aspect_Handle)winId());
    m_view->SetWindow(window);

    if (!window->IsMapped())
        window->Map();

    m_view->SetBackgroundColor(Quantity_NOC_GRAY20);
    m_view->TriedronDisplay(
        Aspect_TOTP_LEFT_LOWER,
        Quantity_NOC_WHITE,
        0.08,
        V3d_ZBUFFER
        );

    m_view->MustBeResized();
    m_view->Redraw();
}

void OcctViewerWidget::displayShape(const TopoDS_Shape& shape, bool fitAll)
{
    if (shape.IsNull()) return;

    m_context->RemoveAll(false);
    Handle(AIS_Shape) ais = new AIS_Shape(shape);

    // Make shaded
    m_context->Display(ais, false);
    m_context->SetDisplayMode(ais, AIS_Shaded, false);

    // Turn on face boundaries (edges)
    ais->Attributes()->SetFaceBoundaryDraw(true);
    ais->Attributes()->SetFaceBoundaryAspect(
        new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.0)
        );

    if (fitAll) m_view->FitAll();
    m_view->Redraw();
}

void OcctViewerWidget::resizeEvent(QResizeEvent*)
{
    if (!m_view.IsNull())
        m_view->MustBeResized();
}

void OcctViewerWidget::paintEvent(QPaintEvent*)
{
    if (!m_view.IsNull())
        m_view->Redraw();
}

void OcctViewerWidget::mousePressEvent(QMouseEvent* e)
{
    m_lastPos = e->pos();

    if (e->button() == Qt::LeftButton)
        m_nav = NavMode::Rotate;
    else if (e->button() == Qt::MiddleButton)
        m_nav = NavMode::Pan;
}

void OcctViewerWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_view.IsNull()) return;

    QPoint p = e->pos();

    if (m_nav == NavMode::Rotate)
        m_view->Rotation(p.x(), p.y());
    else if (m_nav == NavMode::Pan)
        m_view->Pan(p.x() - m_lastPos.x(), m_lastPos.y() - p.y());

    m_lastPos = p;
}

void OcctViewerWidget::mouseReleaseEvent(QMouseEvent*)
{
    m_nav = NavMode::None;
}

void OcctViewerWidget::wheelEvent(QWheelEvent* e)
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
