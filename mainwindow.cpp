#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "OcctViewerWidget.h"
#include <QVBoxLayout>

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include <IGESControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <BRepBuilderAPI_Copy.hxx>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create viewer ONCE
    m_viewer = new OcctViewerWidget(ui->viewerHost);
    auto* layout = new QVBoxLayout(ui->viewerHost);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_viewer);

    // Button click -> file picker -> load IGES -> display
    connect(ui->btnOpenIges, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this,
            "Open IGES file",
            QString(),
            "IGES Files (*.igs *.iges);;All Files (*)"
            );

        if (path.isEmpty())
            return;

        if (!loadIges(path)) {
            QMessageBox::critical(this, "IGES Load Failed",
                                  "Could not load the IGES file.\nCheck the file and try again.");
            return;
        }

        m_viewer->displayShape(m_shape, true);
        // Optional:
        // QMessageBox::information(this, "Loaded", "IGES loaded successfully.");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::loadIges(const QString& path)
{
    IGESControl_Reader reader;

    IFSelect_ReturnStatus stat = reader.ReadFile(path.toStdString().c_str());
    if (stat != IFSelect_RetDone) {
        qWarning() << "IGES ReadFile failed with status:" << (int)stat;
        return false;
    }

    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        qWarning() << "IGES loaded but resulted in a null shape.";
        return false;
    }

    m_shape = BRepBuilderAPI_Copy(shape).Shape();

    qDebug() << "IGES loaded OK:" << path;
    return true;
}
