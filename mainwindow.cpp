#include "mainwindow.h"
#include "surfacecontroldialog.h"
#include "theme.h"
#include "viewerwidget.h"
#include "surfaceworker.h"
#include "nurbs.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QScrollBar>
#include <QTextEdit>
#include <QRegularExpression>
#include "iges_parsing.h"
#include "resultsviewer.h"

QColor MainWindow::defaultPatchColor(int index) const
{
    static const QColor palette[] = {
        QColor( 90, 140, 220),   // steel blue
        QColor( 70, 160, 180),   // teal
        QColor(100, 170, 120),   // sage green
        QColor(180, 130,  90),   // tan
        QColor(160, 100, 160),   // muted purple
        QColor(200, 150,  80),   // gold
        QColor( 80, 180, 200),   // cyan
        QColor(170, 110, 110),   // dusty rose
        QColor(110, 160,  80),   // olive
        QColor(140, 120, 190),   // lavender
    };
    const int n = sizeof(palette) / sizeof(palette[0]);
    return palette[index % n];
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("MAPS0 Desktop Client");
    m_viewer = new ViewerWidget(this);
    setCentralWidget(m_viewer);
    applyDarkTheme();
    createMenus();
    createToolBar();
    createDockPanel();

    m_assembler = new ProjectAssembler(this);
    connect(m_assembler, &ProjectAssembler::mapsOutput,
            this, &MainWindow::onMapsOutput);
    connect(m_assembler, &ProjectAssembler::mapsFinished,
            this, &MainWindow::onMapsFinished);
    connect(m_assembler, &ProjectAssembler::mapsError,
            this, &MainWindow::onMapsOutput);

    m_outputDock = new QDockWidget("MAPS Output", this);
    m_outputDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_outputLog = new QTextEdit();
    m_outputLog->setReadOnly(true);
    m_outputLog->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #00cc66; "
        "font-family: 'Consolas', 'Courier New', monospace; font-size: 11px; "
        "border: none; }");
    m_outputDock->setWidget(m_outputLog);
    addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);
    m_outputDock->hide();

    createStatusBar();

    connect(m_viewer, &ViewerWidget::patchClicked, this, &MainWindow::onPatchClicked);

    showMaximized();
}

MainWindow::~MainWindow()
{
    if (m_workerThread)
    {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void MainWindow::applyDarkTheme()
{
    qApp->setStyleSheet(darkMainWindowStyle());
}

void MainWindow::createMenus()
{
    QMenuBar* mb = menuBar();

    QMenu* fileMenu = mb->addMenu("&File");

    m_actLoadIGES = fileMenu->addAction("&Open IGES...");
    m_actLoadIGES->setShortcut(QKeySequence("Ctrl+O"));
    connect(m_actLoadIGES, &QAction::triggered, this, &MainWindow::onLoadIGES);

    m_actSave = fileMenu->addAction("&Save");
    m_actSave->setShortcut(QKeySequence("Ctrl+S"));
    m_actSave->setEnabled(false);
    connect(m_actSave, &QAction::triggered, this, &MainWindow::onSave);

    fileMenu->addSeparator();

    m_actLoadGauss = fileMenu->addAction("Load &Gauss Points (.geom)...");
    connect(m_actLoadGauss, &QAction::triggered, this, &MainWindow::onLoadGauss);

    QAction* actLoadBodies = fileMenu->addAction("Load All &Bodies...");
    actLoadBodies->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(actLoadBodies, &QAction::triggered, this, &MainWindow::onLoadAllBodies);

    fileMenu->addSeparator();

    m_actExportScreenshot = fileMenu->addAction("Export &Screenshot...");
    m_actExportScreenshot->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(m_actExportScreenshot, &QAction::triggered, this, &MainWindow::onExportScreenshot);

    fileMenu->addSeparator();

    m_actExit = fileMenu->addAction("E&xit");
    m_actExit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(m_actExit, &QAction::triggered, this, &QWidget::close);


    QMenu* viewMenu = mb->addMenu("&View");

    m_actSurface = viewMenu->addAction("&Surface");
    m_actSurface->setCheckable(true);
    m_actSurface->setChecked(true);
    m_actSurface->setShortcut(QKeySequence("S"));
    connect(m_actSurface, &QAction::toggled, this, &MainWindow::onToggleSurface);

    m_actWireframe = viewMenu->addAction("&Wireframe");
    m_actWireframe->setCheckable(true);
    m_actWireframe->setShortcut(QKeySequence("W"));
    connect(m_actWireframe, &QAction::toggled, this, &MainWindow::onToggleWireframe);

    m_actControlNet = viewMenu->addAction("Control &Net");
    m_actControlNet->setCheckable(true);
    m_actControlNet->setShortcut(QKeySequence("N"));
    connect(m_actControlNet, &QAction::toggled, this, &MainWindow::onToggleControlNet);

    m_actControlPts = viewMenu->addAction("Control &Points");
    m_actControlPts->setCheckable(true);
    m_actControlPts->setShortcut(QKeySequence("P"));
    connect(m_actControlPts, &QAction::toggled, this, &MainWindow::onToggleControlPts);

    m_actNormals = viewMenu->addAction("N&ormals");
    m_actNormals->setCheckable(true);
    m_actNormals->setShortcut(QKeySequence("O"));
    connect(m_actNormals, &QAction::toggled, this, &MainWindow::onToggleNormals);

    m_actGaussPts = viewMenu->addAction("&Gauss Points");
    m_actGaussPts->setCheckable(true);
    m_actGaussPts->setShortcut(QKeySequence("G"));
    connect(m_actGaussPts, &QAction::toggled, this, &MainWindow::onToggleGaussPts);

    m_actGaussNormals = viewMenu->addAction("Gauss &Normals (from .geom)");
    m_actGaussNormals->setCheckable(true);
    connect(m_actGaussNormals, &QAction::toggled, this, &MainWindow::onToggleGaussNormals);

    viewMenu->addSeparator();

    m_actPerspective = viewMenu->addAction("Pers&pective");
    m_actPerspective->setCheckable(true);
    m_actPerspective->setChecked(true);
    connect(m_actPerspective, &QAction::toggled, this, &MainWindow::onTogglePerspective);

    m_actFitView = viewMenu->addAction("&Fit to View");
    m_actFitView->setShortcut(QKeySequence("F"));
    connect(m_actFitView, &QAction::triggered, this, &MainWindow::onFitView);


    QMenu* geomMenu = mb->addMenu("&Geometry");

    m_actFlipAll = geomMenu->addAction("Flip Normals (&All)");
    connect(m_actFlipAll, &QAction::triggered, this, &MainWindow::onFlipAll);

    m_actFlipSelected = geomMenu->addAction("Flip Normals (&Selected)");
    connect(m_actFlipSelected, &QAction::triggered, this, &MainWindow::onFlipSelected);

    geomMenu->addSeparator();

    m_actResetNormals = geomMenu->addAction("&Reset All Normals");
    m_actResetNormals->setEnabled(false);
    connect(m_actResetNormals, &QAction::triggered, this, &MainWindow::onResetNormals);


    QMenu* analysisMenu = mb->addMenu("&Analysis");

    QAction* actSurfCtrl = analysisMenu->addAction("&Surface Control (.srfctr)...");
    connect(actSurfCtrl, &QAction::triggered, this, &MainWindow::onSurfaceControl);

    QAction* actPrincipal = analysisMenu->addAction("&Principal Data...");
    connect(actPrincipal, &QAction::triggered, this, &MainWindow::onPrincipalData);

    QAction* actWaves = analysisMenu->addAction("&Wave Settings...");
    connect(actWaves, &QAction::triggered, this, &MainWindow::onWaveSettings);

    QAction* actRunConfig = analysisMenu->addAction("Run &Configuration...");
    connect(actRunConfig, &QAction::triggered, this, &MainWindow::onRunConfig);

    analysisMenu->addSeparator();

    QAction* actGenProject = analysisMenu->addAction("&Generate Project File (.in)...");
    connect(actGenProject, &QAction::triggered, this, &MainWindow::onGenerateProject);

    QAction* actRunMAPS = analysisMenu->addAction("&Run MAPS0...");
    connect(actRunMAPS, &QAction::triggered, this, &MainWindow::onRunMAPS);

    QMenu* resultsMenu = mb->addMenu("&Results");
    QAction* actResults = resultsMenu->addAction("&Open Results Viewer...");
    connect(actResults, &QAction::triggered, this, [this]() {
        ResultsViewer* rv = new ResultsViewer(this);
        rv->setAttribute(Qt::WA_DeleteOnClose);
        rv->show();
    });


}
void MainWindow::createToolBar()
{
    QToolBar* tb = addToolBar("View");
    tb->setMovable(false);
    tb->setIconSize(QSize(16, 16));

    // Display mode toggles
    tb->addAction(m_actSurface);
    tb->addAction(m_actWireframe);
    tb->addSeparator();

    // Overlay toggles
    tb->addAction(m_actControlNet);
    tb->addAction(m_actControlPts);
    tb->addAction(m_actNormals);
    tb->addAction(m_actGaussPts);
    tb->addAction(m_actGaussNormals);
    tb->addSeparator();

    // Camera
    tb->addAction(m_actPerspective);
    tb->addAction(m_actFitView);
}

void MainWindow::createDockPanel()
{
    m_patchDock = new QDockWidget("Surfaces", this);
    m_patchDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_patchDock->setMinimumWidth(220);

    m_patchList = new QTreeWidget();
    m_patchList->setHeaderLabels({"", "Surface", "Info"});
    m_patchList->setColumnWidth(0, 24);
    m_patchList->setColumnWidth(1, 100);
    m_patchList->setRootIsDecorated(false);
    m_patchList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_patchList, &QTreeWidget::itemClicked,
            this, &MainWindow::onPatchListClicked);
    connect(m_patchList, &QTreeWidget::itemDoubleClicked,
            this, [this](QTreeWidgetItem* item, int col) {

                int idx = -1;
                if (item->parent() && (col == 0 || col == 1))
                    idx = item->data(1, Qt::UserRole).toInt();
                else if (!item->parent() && !m_multibodyLoaded && col == 0)
                    idx = m_patchList->indexOfTopLevelItem(item);
                if (idx >= 0)
                    onPatchColorChangeRequested(idx);
            });

    m_patchDock->setWidget(m_patchList);
    addDockWidget(Qt::RightDockWidgetArea, m_patchDock);
}

void MainWindow::rebuildPatchList(int patchCount)
{
    m_patchList->clear();
    m_patchList->setColumnWidth(0, 24);
    m_patchColors.resize(patchCount);

    for (int i = 0; i < patchCount; ++i)
    {
        if (!m_patchColors[i].isValid())
            m_patchColors[i] = defaultPatchColor(i);

        QTreeWidgetItem* item = new QTreeWidgetItem();
        QPixmap px(14, 14);
        px.fill(m_patchColors[i]);
        item->setIcon(0, QIcon(px));

        item->setText(1, QString("Surface %1").arg(i));
        item->setText(2, "");

        m_patchList->addTopLevelItem(item);
    }

    std::vector<QVector3D> colorVec(patchCount);
    for (int i = 0; i < patchCount; ++i)
    {
        colorVec[i] = QVector3D(m_patchColors[i].redF(),
                                m_patchColors[i].greenF(),
                                m_patchColors[i].blueF());
    }
    m_viewer->setPatchColors(colorVec);
}

void MainWindow::createStatusBar()
{
    m_statusFile     = new QLabel("No file loaded");
    m_statusPatches  = new QLabel("");
    m_statusSelected = new QLabel("");

    statusBar()->addWidget(m_statusFile, 1);
    statusBar()->addPermanentWidget(m_statusPatches);
    statusBar()->addPermanentWidget(m_statusSelected);
}

void MainWindow::updateStatusBar()
{
    if (m_currentFile.isEmpty())
        m_statusFile->setText("No file loaded");
    else
        m_statusFile->setText(QFileInfo(m_currentFile).fileName());

    int np = m_patchList->topLevelItemCount();
    m_statusPatches->setText(np > 0 ? QString("%1 patches").arg(np) : "");

    int sel = m_viewer->getSelectedPatch();
    m_statusSelected->setText(sel >= 0 ? QString("Selected: Surface %1").arg(sel) : "");
}


void MainWindow::onLoadIGES()
{
    // Warn about unsaved normal edits
    if (m_hasUnsavedFlips)
    {
        auto result = QMessageBox::warning(this, "Unsaved Changes",
                                           "You have unsaved normal edits. Loading a new file will discard them.\n\nContinue?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes) return;
        m_hasUnsavedFlips = false;
    }

    QString path = QFileDialog::getOpenFileName(
        this, "Open IGES File", QString(),
        "IGES Files (*.igs *.iges);;All Files (*.*)");
    if (path.isEmpty()) return;

    m_currentFile = path;
    statusBar()->showMessage("Reading file...");

    // Read file
    const std::string text = readFileToString(path.toStdString());

    // D-section scan for entity 128 pointers
    auto pointers = scanDSectionForEntity128(text);

    if (pointers.empty())
    {
        QMessageBox::warning(this, "Error", "No NURBS surfaces found.");
        return;
    }

    // Kill any existing worker
    if (m_workerThread)
    {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
    }

    m_worker = new SurfaceWorker();
    m_worker->fileText = text;
    m_worker->nurbsPointers = pointers;
    m_worker->needsParsing = true;

    m_workerThread = new QThread();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &SurfaceWorker::process);
    connect(m_worker, &SurfaceWorker::progress, this, &MainWindow::onWorkerProgress);
    connect(m_worker, &SurfaceWorker::finished, this, &MainWindow::onWorkerFinished);
    connect(m_worker, &SurfaceWorker::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    statusBar()->showMessage(QString("Processing %1 surfaces...").arg(pointers.size()));
    m_workerThread->start();
}

void MainWindow::onWorkerProgress(const QString& msg)
{
    statusBar()->showMessage(msg);
}

void MainWindow::onWorkerFinished(const std::vector<std::vector<QVector3D>>& patchPoints,
                                  const std::vector<std::vector<QVector3D>>& patchNormals,
                                  const std::vector<std::pair<int,int>>& patchDims,
                                  const std::vector<QVector3D>& allPts)
{
    if (allPts.empty())
    {
        statusBar()->showMessage("Error: no valid surface points.");
        return;
    }

    const bool isMultiBody = m_worker && !m_worker->bodyInfos.empty();

    if (isMultiBody)
    {
        // if multi body...
        const auto& bodyInfos = m_worker->bodyInfos;
        m_surfaces = m_worker->surfaces;

        // Color palettes per body are different
        static const QColor bodyPalettes[][3] = {
            { QColor(90, 140, 220),  QColor(70, 160, 180),  QColor(100, 170, 120) },
            { QColor(200, 120, 80),  QColor(180, 130, 90),  QColor(160, 100, 160) },
            { QColor(110, 180, 80),  QColor(80, 180, 200),  QColor(140, 120, 190) },
            { QColor(200, 150, 80),  QColor(170, 110, 110), QColor(90, 170, 170)  },
        };

        std::vector<ViewerWidget::BodyGroup> bodyGroups;
        std::vector<ViewerWidget::ControlNet> allNets;
        m_patchColors.clear();
        m_allPoints.clear();

        // Set all patches first
        m_viewer->setSurfacePatches(patchPoints, patchNormals, patchDims);

        int patchOffset = 0;
        for (int bi = 0; bi < (int)bodyInfos.size(); ++bi)
        {
            const auto& info = bodyInfos[bi];


            int bodyPatchCount = 0;
            for (int si = info.firstSurface;
                 si < info.firstSurface + info.surfaceCount && si < (int)patchPoints.size();
                 ++si)
            {
                bodyPatchCount++;
            }


            ViewerWidget::BodyGroup bg;
            bg.name = info.name;
            bg.firstPatch = patchOffset;
            bg.patchCount = bodyPatchCount;
            bg.modelMatrix.translate(info.x, info.y, 0.0f);
            bg.modelMatrix.rotate(info.angle, 0, 0, 1);
            bodyGroups.push_back(bg);


            int paletteIdx = bi % 4;
            for (int p = 0; p < bodyPatchCount; ++p)
                m_patchColors.push_back(bodyPalettes[paletteIdx][p % 3]);


            QMatrix4x4 model;
            model.translate(info.x, info.y, 0.0f);
            model.rotate(info.angle, 0, 0, 1);

            for (int pi = patchOffset; pi < patchOffset + bodyPatchCount; ++pi)
            {
                for (const auto& pt : patchPoints[pi])
                {
                    if (isFiniteVec(pt))
                        m_allPoints.push_back(model.map(pt));
                }
            }


            QMatrix4x4 ctrlModel;
            ctrlModel.translate(info.x, info.y, 0.0f);
            ctrlModel.rotate(info.angle, 0, 0, 1);

            for (int si = info.firstSurface;
                 si < info.firstSurface + info.surfaceCount && si < (int)m_surfaces.size();
                 ++si)
            {
                const auto& surf = m_surfaces[si];
                ViewerWidget::ControlNet net;
                net.nu = surf.K1 + 1;
                net.nv = surf.K2 + 1;
                net.points.resize(surf.P.size());
                for (int pi = 0; pi < (int)surf.P.size(); ++pi)
                    net.points[pi] = ctrlModel.map(surf.P[pi]);
                allNets.push_back(std::move(net));
            }

            patchOffset += bodyPatchCount;
        }


        m_viewer->setBodyGroups(bodyGroups);
        m_viewer->setControlNets(allNets);

        std::vector<QVector3D> colorVec(m_patchColors.size());
        for (int i = 0; i < (int)m_patchColors.size(); ++i)
        {
            colorVec[i] = QVector3D(m_patchColors[i].redF(),
                                    m_patchColors[i].greenF(),
                                    m_patchColors[i].blueF());
        }
        m_viewer->setPatchColors(colorVec);

        if (!m_allPoints.empty())
            m_viewer->fitToPoints(m_allPoints);

        rebuildPatchListMultiBody();

        m_patchFlipped.assign(patchPoints.size(), false);
        m_hasUnsavedFlips = false;
        m_multibodyLoaded = true;

        // Disablec geometry editing in multi-body mode
        m_actFlipAll->setEnabled(false);
        m_actFlipSelected->setEnabled(false);
        m_actResetNormals->setEnabled(false);
        m_actSave->setEnabled(false);
        m_actLoadGauss->setEnabled(false);


        m_viewer->setShowControlPoints(false);
        m_actControlPts->blockSignals(true);
        m_actControlPts->setChecked(false);
        m_actControlPts->blockSignals(false);

        updateStatusBar();
        setWindowTitle("MAPS0 Desktop Client: Multi-Body View");
        statusBar()->showMessage(
            QString("Loaded %1 bodies, %2 total patches")
                .arg(bodyGroups.size()).arg(patchPoints.size()), 5000);

        m_worker = nullptr;
    }
    else
    {
       // single body
        if (m_worker)
        {
            m_surfaces = m_worker->surfaces;

            std::vector<ViewerWidget::ControlNet> nets;
            for (const auto& surf : m_surfaces)
            {
                ViewerWidget::ControlNet net;
                net.nu = surf.K1 + 1;
                net.nv = surf.K2 + 1;
                net.points = surf.P;
                nets.push_back(std::move(net));
            }
            m_viewer->setControlNets(nets);
        }

        m_allPoints = allPts;
        m_viewer->fitToPoints(allPts);
        m_viewer->setSurfacePatches(patchPoints, patchNormals, patchDims);

        m_viewer->setShowControlPoints(false);
        m_actControlPts->blockSignals(true);
        m_actControlPts->setChecked(false);
        m_actControlPts->blockSignals(false);

        // Reset multi-body state when loading single IGES
        m_multibodyLoaded = false;
        m_viewer->setBodyGroups({});
        m_patchList->setRootIsDecorated(false);

        // Re-enable geometry editing
        m_actFlipAll->setEnabled(true);
        m_actFlipSelected->setEnabled(true);
        m_actResetNormals->setEnabled(false);
        m_actLoadGauss->setEnabled(true);

        // Auto-populate body1 with the currently loaded file
        if (m_bodies.empty())
            m_bodies.push_back({"body1", 0, 0, 0, ""});
        m_bodies[0].igesPath = m_currentFile;
        m_bodies[0].name = QFileInfo(m_currentFile).baseName();

        rebuildPatchList((int)patchPoints.size());

        m_patchFlipped.assign(patchPoints.size(), false);
        m_hasUnsavedFlips = false;
        m_actSave->setEnabled(false);
        m_actResetNormals->setEnabled(false);

        for (int i = 0; i < (int)patchDims.size() && i < m_patchList->topLevelItemCount(); ++i)
        {
            auto* item = m_patchList->topLevelItem(i);
            item->setText(2, QString("%1×%2").arg(patchDims[i].first).arg(patchDims[i].second));
        }

        updateStatusBar();
        statusBar()->showMessage("Ready", 3000);

        m_worker = nullptr;
    }
}

void MainWindow::onLoadGauss()
{
    if (m_multibodyLoaded)
    {
        QMessageBox::information(this, "Load Gauss Points",
                                 "Gauss point loading is not available in multi-body mode.\n"
                                 "Load a single IGES file first.");
        return;
    }

    QString path = QFileDialog::getOpenFileName(
        this, "Load Gauss Points", QString(),
        "MAPS Geometry (*.geom);;Text Files (*.txt *.dat);;All Files (*.*)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    std::vector<QVector3D> points;
    std::vector<QVector3D> normals;
    QTextStream in(&file);

    bool isGeomFormat = path.endsWith(".geom", Qt::CaseInsensitive);

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('!')) continue;

        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (isGeomFormat && parts.size() >= 7)
        {
            bool ok1, ok2, ok3, ok4, ok5, ok6;
            float x  = parts[1].toFloat(&ok1);
            float y  = parts[2].toFloat(&ok2);
            float z  = parts[3].toFloat(&ok3);
            float nx = parts[4].toFloat(&ok4);
            float ny = parts[5].toFloat(&ok5);
            float nz = parts[6].toFloat(&ok6);
            if (ok1 && ok2 && ok3)
                points.push_back(QVector3D(x, y, z));
            if (ok4 && ok5 && ok6)
                normals.push_back(QVector3D(nx, ny, nz));
        }
        else if (parts.size() >= 3)
        {
            QStringList commaParts;
            if (line.contains(','))
                commaParts = line.split(',', Qt::SkipEmptyParts);
            else
                commaParts = parts;

            bool ok1, ok2, ok3;
            float x = commaParts[0].toFloat(&ok1);
            float y = commaParts[1].toFloat(&ok2);
            float z = commaParts[2].toFloat(&ok3);
            if (ok1 && ok2 && ok3)
                points.push_back(QVector3D(x, y, z));
        }
    }

    if (!points.empty())
    {
        m_viewer->setGaussPoints(points);
        m_viewer->setShowGaussPoints(true);
        m_actGaussPts->setChecked(true);

        if (normals.size() == points.size())
        {
            m_viewer->setGaussNormals(normals);
            m_viewer->setShowGaussNormals(true);
            m_actGaussNormals->setChecked(true);
            statusBar()->showMessage(
                QString("Loaded %1 Gauss points with normals from %2")
                    .arg(points.size()).arg(QFileInfo(path).fileName()), 5000);
        }
        else
        {
            statusBar()->showMessage(
                QString("Loaded %1 Gauss points from %2")
                    .arg(points.size()).arg(QFileInfo(path).fileName()), 3000);
        }
    }
}

void MainWindow::onLoadAllBodies()
{
    // Make sure bodies are configured
    if (m_bodies.empty())
    {
        QMessageBox::information(this, "Load All Bodies",
                                 "Configure bodies in Analysis > Run Configuration first.\n"
                                 "Each body needs a Name, position (X, Y, Heading), and IGES file.");
        return;
    }

    // Warn about unsaved normal edits
    if (m_hasUnsavedFlips)
    {
        auto result = QMessageBox::warning(this, "Unsaved Changes",
                                           "You have unsaved normal edits. Loading bodies will discard them.\n\nContinue?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes) return;
        m_hasUnsavedFlips = false;
    }

    // Validate body names
    QStringList names;
    for (int i = 0; i < (int)m_bodies.size(); ++i)
    {
        QString name = m_bodies[i].name.trimmed();
        if (name.isEmpty())
        {
            QMessageBox::warning(this, "Load All Bodies",
                                 QString("Body %1 has an empty name. All bodies need unique names.").arg(i + 1));
            return;
        }
        if (names.contains(name))
        {
            QMessageBox::warning(this, "Load All Bodies",
                                 QString("Duplicate body name '%1'. All bodies need unique names.").arg(name));
            return;
        }
        names.append(name);
    }

    // Check IGES files
    QStringList missing;
    int validCount = 0;
    for (const auto& b : m_bodies)
    {
        if (b.igesPath.isEmpty())
            missing.append(QString("  %1: no file selected").arg(b.name));
        else if (!QFile::exists(b.igesPath))
            missing.append(QString("  %1: file not found (%2)").arg(b.name, b.igesPath));
        else
            validCount++;
    }

    if (validCount == 0)
    {
        QMessageBox::warning(this, "Load All Bodies",
                             "No valid IGES files found:\n" + missing.join("\n") +
                                 "\n\nSet IGES file paths in Analysis > Run Configuration > Bodies tab.");
        return;
    }

    if (!missing.isEmpty())
    {
        auto result = QMessageBox::warning(this, "Load All Bodies",
                                           QString("Some bodies have missing IGES files:\n%1\n\n"
                                                   "These bodies will be skipped. Continue?").arg(missing.join("\n")),
                                           QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes) return;
    }

    statusBar()->showMessage("Loading bodies...");
    QApplication::processEvents();

    // Clear existing data
    m_viewer->clearAll();
    m_surfaces.clear();
    m_allPoints.clear();
    m_patchColors.clear();
    m_patchFlipped.clear();

    // Kill any existing worker
    if (m_workerThread)
    {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
    }

    // Create worker and pack all body surfaces into it
    m_worker = new SurfaceWorker();
    m_worker->needsParsing = false;  // we parse here, worker just evaluates

    int totalSurfaces = 0;

    for (int bodyIdx = 0; bodyIdx < (int)m_bodies.size(); ++bodyIdx)
    {
        const auto& body = m_bodies[bodyIdx];
        if (body.igesPath.isEmpty() || !QFile::exists(body.igesPath))
        {
            statusBar()->showMessage(
                QString("Skipping body '%1': no IGES file").arg(body.name));
            QApplication::processEvents();
            continue;
        }

        statusBar()->showMessage(
            QString("Parsing body %1/%2: %3...")
                .arg(bodyIdx + 1).arg(m_bodies.size()).arg(body.name));
        QApplication::processEvents();

        // read file
        const std::string text = readFileToString(body.igesPath.toStdString());

        // scan d section
        auto pointers = scanDSectionForEntity128(text);
        if (pointers.empty()) continue;

        // parse
        std::vector<NurbsSurface> bodySurfaces;
        for (int ptr : pointers)
        {
            std::vector<std::string> tokens;
            if (!::parsePSectionEntity(text, ptr, tokens)) continue;
            NurbsSurface s;
            if (!::buildSurfaceFromTokens(tokens, s)) continue;
            bodySurfaces.push_back(std::move(s));
        }

        if (bodySurfaces.empty()) continue;

        // append to worker
        SurfaceWorker::BodyInfo info;
        info.name = body.name;
        info.x = body.x;
        info.y = body.y;
        info.angle = body.angle;
        info.firstSurface = totalSurfaces;
        info.surfaceCount = (int)bodySurfaces.size();
        m_worker->bodyInfos.push_back(info);

        m_worker->surfaces.insert(m_worker->surfaces.end(),
                                  std::make_move_iterator(bodySurfaces.begin()),
                                  std::make_move_iterator(bodySurfaces.end()));
        totalSurfaces += info.surfaceCount;
    }

    if (m_worker->surfaces.empty())
    {
        delete m_worker;
        m_worker = nullptr;
        QMessageBox::warning(this, "Load All Bodies", "No surfaces were loaded.");
        return;
    }

    m_workerThread = new QThread();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &SurfaceWorker::process);
    connect(m_worker, &SurfaceWorker::progress, this, &MainWindow::onWorkerProgress);
    connect(m_worker, &SurfaceWorker::finished, this, &MainWindow::onWorkerFinished);
    connect(m_worker, &SurfaceWorker::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    statusBar()->showMessage(QString("Evaluating %1 surfaces across %2 bodies...")
                                 .arg(totalSurfaces).arg(m_worker->bodyInfos.size()));
    m_workerThread->start();
}

void MainWindow::rebuildPatchListMultiBody()
{
    m_patchList->clear();
    m_patchList->setRootIsDecorated(true);
    m_patchList->setColumnWidth(0, 60);

    const auto& groups = m_viewer->getBodyGroups();

    for (int gIdx = 0; gIdx < (int)groups.size(); ++gIdx)
    {
        const auto& bg = groups[gIdx];


        QTreeWidgetItem* bodyItem = new QTreeWidgetItem();
        bodyItem->setText(1, QString("%1").arg(bg.name));
        bodyItem->setText(2, QString("%1 patches").arg(bg.patchCount));
        bodyItem->setFlags(bodyItem->flags() & ~Qt::ItemIsSelectable);

        // Set body color swatch to the first patch's color
        if (bg.firstPatch < (int)m_patchColors.size())
        {
            QPixmap px(14, 14);
            px.fill(m_patchColors[bg.firstPatch]);
            bodyItem->setIcon(0, QIcon(px));
        }

        m_patchList->addTopLevelItem(bodyItem);

        // Child items for each patch
        for (int p = 0; p < bg.patchCount; ++p)
        {
            int idx = bg.firstPatch + p;
            QTreeWidgetItem* patchItem = new QTreeWidgetItem();

            if (idx < (int)m_patchColors.size())
            {
                QPixmap px(14, 14);
                px.fill(m_patchColors[idx]);
                patchItem->setIcon(1, QIcon(px));  // column 1 so tree indent doesn't hide it
            }

            patchItem->setText(1, QString("Surface %1").arg(p));
            patchItem->setData(1, Qt::UserRole, idx);  // store global patch index

            bodyItem->addChild(patchItem);
        }

        bodyItem->setExpanded(true);
    }
}

void MainWindow::onExportScreenshot()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Export Screenshot", "screenshot.png",
        "PNG Images (*.png);;JPEG Images (*.jpg)");
    if (path.isEmpty()) return;

    QImage img = m_viewer->grabFramebuffer();
    img.save(path);
    statusBar()->showMessage("Screenshot saved: " + path, 3000);
}

void MainWindow::onToggleSurface(bool on)     { m_viewer->setShowSurface(on); }
void MainWindow::onToggleWireframe(bool on)   { m_viewer->setWireframe(on); }
void MainWindow::onToggleControlNet(bool on)  { m_viewer->setShowControlNet(on); }
void MainWindow::onToggleControlPts(bool on)  { m_viewer->setShowControlPoints(on); }
void MainWindow::onToggleNormals(bool on)     { m_viewer->setShowNormals(on); }
void MainWindow::onToggleGaussPts(bool on)    { m_viewer->setShowGaussPoints(on); }
void MainWindow::onToggleGaussNormals(bool on){ m_viewer->setShowGaussNormals(on); }
void MainWindow::onTogglePerspective(bool on) { m_viewer->setUsePerspective(on); }

void MainWindow::onFitView()
{
    if (!m_allPoints.empty())
        m_viewer->fitToPoints(m_allPoints);
}

void MainWindow::onFlipAll()
{
    if (m_surfaces.empty()) return;

    if (m_multibodyLoaded)
    {
        QMessageBox::information(this, "Flip Normals",
                                 "Normal editing is disabled in multi-body mode.\n"
                                 "Load a single IGES file (File > Open IGES) to edit normals.");
        return;
    }

    auto result = QMessageBox::question(this, "Flip All Normals",
                                        "Flip normals on all patches?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    // Create backup on first flip
    if (!m_hasUnsavedFlips && !m_currentFile.isEmpty())
    {
        QFileInfo fi(m_currentFile);
        m_backupFile = fi.absolutePath() + "/" + fi.baseName() + "_backup.igs";
        QFile::copy(m_currentFile, m_backupFile);
    }

    bool current = m_viewer->isFlipNormals();
    m_viewer->setFlipNormals(!current);

    // Mark all patches as flipped
    for (int i = 0; i < (int)m_patchFlipped.size(); ++i)
        m_patchFlipped[i] = !m_patchFlipped[i];

    m_hasUnsavedFlips = true;
    m_actSave->setEnabled(true);
    m_actResetNormals->setEnabled(true);
    setWindowTitle("MAPS0 Desktop Client (Unsaved Changes)");  // unsaved changes
}

void MainWindow::onFlipSelected()
{
    if (m_multibodyLoaded)
    {
        QMessageBox::information(this, "Flip Normals",
                                 "Normal editing is disabled in multi-body mode.\n"
                                 "Load a single IGES file (File > Open IGES) to edit normals.");
        return;
    }

    int sel = m_viewer->getSelectedPatch();
    if (sel < 0)
    {
        QMessageBox::information(this, "Flip Selected",
                                 "No patch selected. Click a patch first.");
        return;
    }

    auto result = QMessageBox::question(this, "Flip Selected Normal",
                                        QString("Flip normals on Surface %1?").arg(sel),
                                        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    // Create backup on first flip
    if (!m_hasUnsavedFlips && !m_currentFile.isEmpty())
    {
        QFileInfo fi(m_currentFile);
        m_backupFile = fi.absolutePath() + "/" + fi.baseName() + "_backup.igs";
        QFile::copy(m_currentFile, m_backupFile);
    }

    m_viewer->flipPatchNormals(sel);

    if (sel < (int)m_patchFlipped.size())
        m_patchFlipped[sel] = !m_patchFlipped[sel];

    m_hasUnsavedFlips = true;
    m_actSave->setEnabled(true);
    m_actResetNormals->setEnabled(true);
    setWindowTitle("MAPS0 Desktop Client (Unsaved Changes)");
}

void MainWindow::onSave()
{
    if (m_multibodyLoaded)
    {
        QMessageBox::information(this, "Save",
                                 "Saving is disabled in multi-body mode.\n"
                                 "Load a single IGES file to edit and save geometry.");
        return;
    }
    if (!m_hasUnsavedFlips || m_currentFile.isEmpty()) return;

    // Build output surfaces with flips applied
    std::vector<NurbsSurface> outputSurfaces = m_surfaces;
    for (int i = 0; i < (int)m_patchFlipped.size() && i < (int)outputSurfaces.size(); ++i)
    {
        if (m_patchFlipped[i])
            flipSurfaceV(outputSurfaces[i]);
    }

    if (writeIgesFile(m_currentFile.toStdString(), outputSurfaces))
    {
        m_hasUnsavedFlips = false;
        m_actSave->setEnabled(false);
        setWindowTitle("MAPS0 Desktop Client");
        statusBar()->showMessage("Saved to " + m_currentFile, 5000);
    }
    else
    {
        QMessageBox::warning(this, "Error", "Failed to save IGES file.");
    }
}
void MainWindow::onResetNormals()
{
    if (m_multibodyLoaded) return;

    auto result = QMessageBox::question(this, "Reset Normals",
                                        "Reset all normals to original orientation?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    // Reset visual state
    m_viewer->setFlipNormals(false);
    for (int i = 0; i < (int)m_patchFlipped.size(); ++i)
    {
        if (m_patchFlipped[i])
        {
            m_viewer->flipPatchNormals(i);
            m_patchFlipped[i] = false;
        }
    }

    // Restore backup if it exists
    if (!m_backupFile.isEmpty() && QFile::exists(m_backupFile))
    {
        QFile::remove(m_currentFile);
        QFile::rename(m_backupFile, m_currentFile);
        m_backupFile.clear();
    }

    // Delete the knormal file
    if (!m_currentFile.isEmpty())
    {
        QFileInfo fi(m_currentFile);
        QString knormalPath = fi.absolutePath() + "/" + fi.baseName() + ".knormal";
        QFile::remove(knormalPath);
    }

    m_hasUnsavedFlips = false;
    m_actSave->setEnabled(false);
    m_actResetNormals->setEnabled(false);
    setWindowTitle("MAPS0 Desktop Client");
    statusBar()->showMessage("Normals reset to original", 3000);
}

void MainWindow::onPatchListClicked(QTreeWidgetItem* item, int column)
{
    if (!item) return;

    int idx = -1;
    if (item->parent())
    {
        // Child item in multi-body mode
        idx = item->data(1, Qt::UserRole).toInt();
    }
    else if (!m_multibodyLoaded)
    {
        // Flat list (single body mode)
        idx = m_patchList->indexOfTopLevelItem(item);
    }

    if (idx >= 0)
    {
        // Select patch in viewer
        m_viewer->setSelectedPatch(idx);
        updateStatusBar();
    }
}

void MainWindow::onPatchColorChangeRequested(int patchIndex)
{
    if (patchIndex < 0 || patchIndex >= (int)m_patchColors.size()) return;

    QColor newColor = QColorDialog::getColor(
        m_patchColors[patchIndex], this, "Choose Patch Color");

    if (!newColor.isValid()) return;

    m_patchColors[patchIndex] = newColor;

    QPixmap px(14, 14);
    px.fill(newColor);

    if (m_multibodyLoaded)
    {
        // Find child item by global index and update icon on column 1
        for (int g = 0; g < m_patchList->topLevelItemCount(); ++g)
        {
            QTreeWidgetItem* bodyItem = m_patchList->topLevelItem(g);
            for (int c = 0; c < bodyItem->childCount(); ++c)
            {
                QTreeWidgetItem* child = bodyItem->child(c);
                if (child->data(1, Qt::UserRole).toInt() == patchIndex)
                {
                    child->setIcon(1, QIcon(px));
                    break;
                }
            }
        }
    }
    else
    {
        QTreeWidgetItem* item = m_patchList->topLevelItem(patchIndex);
        if (item)
            item->setIcon(0, QIcon(px));
    }

    std::vector<QVector3D> colorVec(m_patchColors.size());
    for (int i = 0; i < (int)m_patchColors.size(); ++i)
    {
        colorVec[i] = QVector3D(m_patchColors[i].redF(),
                                m_patchColors[i].greenF(),
                                m_patchColors[i].blueF());
    }
    m_viewer->setPatchColors(colorVec);
}

void MainWindow::onPatchClicked(int patchIndex)
{
    if (m_multibodyLoaded)
    {
        // Find the child item with matching global index
        for (int g = 0; g < m_patchList->topLevelItemCount(); ++g)
        {
            QTreeWidgetItem* bodyItem = m_patchList->topLevelItem(g);
            for (int c = 0; c < bodyItem->childCount(); ++c)
            {
                QTreeWidgetItem* child = bodyItem->child(c);
                if (child->data(1, Qt::UserRole).toInt() == patchIndex)
                {
                    m_patchList->setCurrentItem(child);
                    updateStatusBar();
                    return;
                }
            }
        }
        m_patchList->clearSelection();
    }
    else
    {

        if (patchIndex >= 0 && patchIndex < m_patchList->topLevelItemCount())
            m_patchList->setCurrentItem(m_patchList->topLevelItem(patchIndex));
        else
            m_patchList->clearSelection();
    }
    updateStatusBar();
}

void MainWindow::onSurfaceControl()
{
    if (m_surfaces.empty())
    {
        QMessageBox::information(this, "Surface Control",
                                 "Load an IGES file first.");
        return;
    }

    // Get the sampled patch points from the viewer for waterline detection
    // (m_viewer stores m_surfacePatches internally)

    SurfaceControlDialog dlg(
        (int)m_patchColors.size(),
        m_patchFlipped,
        m_viewer->getSurfacePatches(),
        this);

    dlg.exec();
}

void MainWindow::onPrincipalData()
{
    PrincipalDataDialog dlg(this);
    dlg.setData(m_principalData);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_principalData = dlg.getData();
        statusBar()->showMessage("Principal data updated", 3000);
    }
}

void MainWindow::onWaveSettings()
{
    WaveSettingsDialog dlg(this);
    dlg.setData(m_waveData);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_waveData = dlg.getData();
        statusBar()->showMessage("Wave settings updated", 3000);
    }
}


void MainWindow::onRunConfig()
{
    RunConfigDialog dlg(this);
    dlg.setRunControls(m_runControls);
    dlg.setBodies(m_bodies);
    dlg.setRestrainedModes(m_restrainedModes);
    dlg.setWallData(m_wallData);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_runControls = dlg.getRunControls();
        m_bodies = dlg.getBodies();
        m_restrainedModes = dlg.getRestrainedModes();
        m_optionalFiles = dlg.getOptionalFiles();
        m_wallData = dlg.getWallData();
        statusBar()->showMessage("Run configuration updated", 3000);
    }
}


void MainWindow::onGenerateProject()
{
    // Validate minimum requirements
    if (m_bodies.empty())
    {
        QMessageBox::warning(this, "Generate Project",
                             "Configure at least one body in Run Configuration first.");
        return;
    }
    if (m_principalData.length <= 0)
    {
        QMessageBox::warning(this, "Generate Project",
                             "Set Principal Data (hull dimensions) first.");
        return;
    }
    if (m_waveData.headings.empty())
    {
        QMessageBox::warning(this, "Generate Project",
                             "Set Wave Settings (at least one heading) first.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Save Project File", "",
        "MAPS Project Files (*.in);;All Files (*.*)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".in", Qt::CaseInsensitive)) path += ".in";

    // Feed everything to the assembler
    QFileInfo fi(path);
    m_assembler->setProjectName(fi.baseName());
    m_assembler->setRunControls(m_runControls);
    m_assembler->setBodies(m_bodies);
    m_assembler->setRestrainedModes(m_restrainedModes);
    m_assembler->setPrincipalData(m_principalData);
    m_assembler->setWaveData(m_waveData);
    m_assembler->setOptionalFiles(m_optionalFiles);
    m_assembler->setWallData(m_wallData);

    if (m_assembler->writeProjectFile(path))
    {
        statusBar()->showMessage("Project file saved: " + path, 5000);
        QMessageBox::information(this, "Project Generated",
                                 QString("Project file saved to:\n%1\n\n"
                                         "Make sure the .igs and .srfctr files are in the same directory "
                                         "with matching body names.").arg(path));
    }
    else
    {
        QMessageBox::warning(this, "Error", "Failed to write project file.");
    }
}

void MainWindow::onRunMAPS()
{
    if (m_assembler->isRunning())
    {
        auto result = QMessageBox::question(this, "MAPS Running",
                                            "MAPS is currently running. Stop it?",
                                            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
            m_assembler->stopMAPS();
        return;
    }

    QString mapsExe = QFileDialog::getOpenFileName(
        this, "Select MAPS0 Executable", "",
        "Executables (*.exe);;All Files (*.*)");
    if (mapsExe.isEmpty()) return;

    QString projectFile = QFileDialog::getOpenFileName(
        this, "Select Project File (.in)", "",
        "MAPS Project Files (*.in);;All Files (*.*)");
    if (projectFile.isEmpty()) return;


    m_outputDock->show();
    m_outputLog->clear();
    m_outputLog->append("=== Starting MAPS0 ===");
    m_outputLog->append("Executable: " + mapsExe);
    m_outputLog->append("Project: " + projectFile);
    m_outputLog->append("");

    statusBar()->showMessage("Running MAPS0...");
    m_assembler->runMAPS(mapsExe, projectFile);
}

void MainWindow::onStopMAPS()
{
    m_assembler->stopMAPS();
}

void MainWindow::onMapsOutput(const QString& line)
{
    if (m_outputLog)
    {
        m_outputLog->append(line);
        m_outputLog->verticalScrollBar()->setValue(
            m_outputLog->verticalScrollBar()->maximum());
    }
}

void MainWindow::onMapsFinished(int exitCode, const QString& summary)
{
    if (m_outputLog)
    {
        m_outputLog->append("");
        m_outputLog->append("=== " + summary + " ===");
    }

    if (exitCode == 0)
        statusBar()->showMessage("MAPS0 completed successfully", 5000);
    else
        statusBar()->showMessage("MAPS0 finished with errors", 5000);
}
