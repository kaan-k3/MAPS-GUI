#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QTreeWidget>
#include <QAction>
#include <QLabel>
#include <QThread>
#include <QVector3D>
#include <QColor>
#include <vector>
#include "principaldatadialog.h"
#include "wavesettingsdialog.h"
#include "runconfigdialog.h"
#include "projectassembler.h"

class ViewerWidget;
class SurfaceWorker;
struct NurbsSurface;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // Core widgets
    ViewerWidget*   m_viewer = nullptr;
    QDockWidget*    m_patchDock = nullptr;
    QTreeWidget*    m_patchList = nullptr;

    // Worker thread
    QThread*        m_workerThread = nullptr;
    SurfaceWorker*  m_worker = nullptr;

    // Data
    std::vector<NurbsSurface> m_surfaces;
    std::vector<QColor>       m_patchColors;
    QString                   m_currentFile;
    std::vector<QVector3D> m_allPoints;  // stored for refit

    // Status bar labels
    QLabel* m_statusFile = nullptr;
    QLabel* m_statusPatches = nullptr;
    QLabel* m_statusSelected = nullptr;

    // Actions (kept as members for toolbar/menu sharing)
    // File
    QAction* m_actLoadIGES = nullptr;
    QAction* m_actLoadGauss = nullptr;
    QAction* m_actExportScreenshot = nullptr;
    QAction* m_actExit = nullptr;

    // View toggles
    QAction* m_actSurface = nullptr;
    QAction* m_actWireframe = nullptr;
    QAction* m_actControlNet = nullptr;
    QAction* m_actControlPts = nullptr;
    QAction* m_actNormals = nullptr;
    QAction* m_actGaussPts = nullptr;
    QAction* m_actGaussNormals = nullptr;
    QAction* m_actPerspective = nullptr;
    QAction* m_actFitView = nullptr;

    // Geometry
    QAction* m_actFlipAll = nullptr;
    QAction* m_actFlipSelected = nullptr;

    // UI Setup helpers
    void createMenus();
    void createToolBar();
    void createDockPanel();
    void createStatusBar();
    void applyDarkTheme();

    // Patch list helpers
    void rebuildPatchList(int patchCount);
    void rebuildPatchListMultiBody();
    QColor defaultPatchColor(int index) const;
    void updateStatusBar();

    // IGES parsing (static helpers)
    static bool parsePSectionEntity(const std::string& text, int startSeq,
                                    std::vector<std::string>& outTokens);
    static bool buildSurfaceFromTokens(const std::vector<std::string>& tokens,
                                       NurbsSurface& surfOut);
    // Normal flip tracking
    std::vector<bool> m_patchFlipped;   // per-patch flip state
    QString m_backupFile;               // path to backup .igs
    bool m_hasUnsavedFlips = false;

    // Additional actions
    QAction* m_actSave = nullptr;
    QAction* m_actResetNormals = nullptr;

    // Stored dialog data (persists between dialog opens)
    PrincipalDataDialog::PrincipalData m_principalData;
    WaveSettingsDialog::WaveData m_waveData;

    // Stored config data (persists between dialog opens)
    RunConfigDialog::RunControls m_runControls;
    std::vector<RunConfigDialog::BodyDef> m_bodies = {{"body1", 0, 0, 0, ""}};
    std::vector<bool> m_restrainedModes = {false, false, false, false, false, false};
    RunConfigDialog::OptionalFiles m_optionalFiles;
    RunConfigDialog::WallData m_wallData;

    // Multi-body tracking
    bool m_multibodyLoaded = false;

    // Project assembler
    ProjectAssembler* m_assembler = nullptr;

    // MAPS output dock
    QDockWidget* m_outputDock = nullptr;
    QTextEdit*   m_outputLog = nullptr;



private slots:
    // File
    void onLoadIGES();
    void onLoadGauss();
    void onLoadAllBodies();
    void onExportScreenshot();
    void onSave();
    void onResetNormals();
    void onSurfaceControl();
    void onPrincipalData();
    void onWaveSettings();
    void onRunConfig();
    void onGenerateProject();
    void onRunMAPS();
    void onStopMAPS();
    void onMapsOutput(const QString& line);
    void onMapsFinished(int exitCode, const QString& summary);

    // View
    void onToggleSurface(bool on);
    void onToggleWireframe(bool on);
    void onToggleControlNet(bool on);
    void onToggleControlPts(bool on);
    void onToggleNormals(bool on);
    void onToggleGaussPts(bool on);
    void onToggleGaussNormals(bool on);
    void onTogglePerspective(bool on);
    void onFitView();

    // Geometry
    void onFlipAll();
    void onFlipSelected();

    // Patch list
    void onPatchListClicked(QTreeWidgetItem* item, int column);
    void onPatchColorChangeRequested(int patchIndex);

    // Viewer signals
    void onPatchClicked(int patchIndex);

    // Worker signals
    void onWorkerProgress(const QString& msg);
    void onWorkerFinished(const std::vector<std::vector<QVector3D>>& patchPoints,
                          const std::vector<std::vector<QVector3D>>& patchNormals,
                          const std::vector<std::pair<int,int>>& patchDims,
                          const std::vector<QVector3D>& allPts);

};


