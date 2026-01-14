#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "OcctViewerWidget.h" // viewport for OCCT, and QVBOX for placing the viewer inside a Widget
#include <QVBoxLayout>

#include <QFileDialog> // Qt utility headers used for: file selection dialogs, error / information popups, debug logging
#include <QMessageBox>
#include <QDebug>

#include <IGESControl_Reader.hxx> // OpenCASCADE headers for reading IGES files and managing CAD shapes
#include <IFSelect_ReturnStatus.hxx>
#include <BRepBuilderAPI_Copy.hxx>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)        // MainWindow Constructor for initializing the Qt-generated UI, creating and embedding the OpenCASCADE viewer widget and connecting UI actions (button clicks) to application logic
{
    ui->setupUi(this); // Build the widget hierarchy defined in mainwindow.ui. After this call, ui->btnOpenIges, ui->viewerHost, etc. are valid.

    // Create viewer ONCE and embed it into viewerHost (placeholder widget)
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

        if (path.isEmpty()) // User Cancel
            return;

        if (!loadIges(path)) { // Loading Logic
            QMessageBox::critical(this, "IGES Load Failed",
                                  "Could not load the IGES file.\nCheck the file and try again.");
            return;
        }

        m_viewer->displayShape(m_shape, true); // Display the loaded shape

    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::loadIges(const QString& path) // Reads an IGES file from disk and converts it into a TopoDS_Shape.
                                            // Returns True if loading and conversion succeed and false if it fails
{
    IGESControl_Reader reader; // Read file and return status code

    IFSelect_ReturnStatus stat = reader.ReadFile(path.toStdString().c_str());
    if (stat != IFSelect_RetDone) {
        qWarning() << "IGES ReadFile failed with status:" << (int)stat;
        return false;
    }

    reader.TransferRoots(); // Transfer to CAD geometry
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        qWarning() << "IGES loaded but resulted in a null shape."; // Validate Shape
        return false;
    }
//  Copy the shape to detach it from the reader's internal data.

    m_shape = BRepBuilderAPI_Copy(shape).Shape();

    qDebug() << "IGES loaded OK:" << path;
    return true;
}
