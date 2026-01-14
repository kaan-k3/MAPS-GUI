#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include <TopoDS_Shape.hxx>

// Forward declarations
class OcctViewerWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui = nullptr;

    OcctViewerWidget* m_viewer = nullptr;  // 3D viewer
    TopoDS_Shape m_shape;                  // loaded IGES shape

    bool loadIges(const QString& path);
};

#endif // MAINWINDOW_H
