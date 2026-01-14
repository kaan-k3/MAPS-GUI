#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include <TopoDS_Shape.hxx>

// Forward declarations
class OcctViewerWidget;
/*
 * Qt-generated UI classes live inside the Ui namespace.
 * The Ui::MainWindow class is auto-generated from mainwindow.ui and
 * is responsible only for constructing and laying out widgets.
 */

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


/* MainWindow
* This class represents the primary application window.
* It owns:
*  - the Qt-generated UI (buttons, layouts, placeholders)
*  - the embedded OpenCASCADE 3D viewer
*  - the currently loaded CAD model (TopoDS_Shape)
* UI layout is defined in mainwindow.ui.
* Application logic and behavior are implemented in mainwindow.cpp.
*/


class MainWindow : public QMainWindow
{
    Q_OBJECT // Enables Qt's meta-object system (signals, slots, events)

public:


    /*
     * Constructs the main application window.
     * The optional parent parameter is part of Qt's ownership system.
     */

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui = nullptr; // Pointer to the auto-generated UI helper class. This object creates and connects all widgets defined in the .ui file.

    OcctViewerWidget* m_viewer = nullptr;  // 3D viewer for the currently loaded CAD model. Stored as a TopoDS_Shape, which represents exact CAD topology (not a mesh).
    TopoDS_Shape m_shape;                  // loaded IGES shape

    bool loadIges(const QString& path); // Loads an IGES file from disk and stores the resulting shape. Returns true on success, false if loading or conversion fails.
};

#endif // MAINWINDOW_H
