#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QListWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QPen>
#include <vector>
#include <QString>

class ResultsViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ResultsViewer(QWidget* parent = nullptr);

    struct MotionBlock {
        QString bodyName;
        double speed = 0;
        double heading = 0;
        std::vector<double> freq;
        std::vector<double> period;
        std::vector<double> rao[6];
        std::vector<double> phase[6];
    };

    struct MatrixBlock {
        QString bodyName;
        double speed = 0;
        double heading = 0;
        std::vector<double> freq;
        std::vector<double> period;
        std::vector<std::vector<double>> matrix6x6;
    };

    struct MotionFile {
        QString filePath;
        std::vector<MotionBlock> blocks;
    };

    struct MatrixFile {
        QString filePath;
        std::vector<MatrixBlock> blocks;
    };

    bool loadMotionFile(const QString& path);
    bool loadAddedMassFile(const QString& path);
    bool loadDampingFile(const QString& path);
    bool loadWaveForceFile(const QString& path);

private:
    QTabWidget* m_tabs = nullptr;

    std::vector<MotionFile> m_motionFiles;
    std::vector<MatrixFile> m_addedMassFiles;
    std::vector<MotionFile> m_dampingFiles;
    std::vector<MotionFile> m_waveForceFiles;


    QComboBox*   m_motionViewMode = nullptr;
    QComboBox*   m_motionDataType = nullptr;
    QListWidget* m_motionDatasets = nullptr;
    QWidget*     m_motionChartContainer = nullptr;
    QVBoxLayout* m_motionChartLayout = nullptr;
    std::vector<QCheckBox*> m_motionModeChecks;


    QListWidget* m_admsDatasets = nullptr;
    QComboBox*   m_admsViewMode = nullptr;
    QWidget*     m_admsChartContainer = nullptr;
    QVBoxLayout* m_admsChartLayout = nullptr;
    std::vector<QCheckBox*> m_admsModeChecks;


    QListWidget* m_dampDatasets = nullptr;
    QComboBox*   m_dampViewMode = nullptr;
    QComboBox*   m_dampDataType = nullptr;
    QWidget*     m_dampChartContainer = nullptr;
    QVBoxLayout* m_dampChartLayout = nullptr;
    std::vector<QCheckBox*> m_dampModeChecks;


    QListWidget* m_waveDatasets = nullptr;
    QComboBox*   m_waveViewMode = nullptr;
    QComboBox*   m_waveDataType = nullptr;
    QWidget*     m_waveChartContainer = nullptr;
    QVBoxLayout* m_waveChartLayout = nullptr;
    std::vector<QCheckBox*> m_waveModeChecks;

    static constexpr int NUM_MODES = 6;
    QColor modeColor(int i) const;
    QColor datasetColor(int i) const;
    QString modeName(int i) const;
    QString modeUnit(bool isPhase, int i) const;

    void setupUI();
    void applyDarkStyle();

    QWidget* createMotionTab();
    QWidget* createAddedMassTab();
    QWidget* createDampingTab();
    QWidget* createWaveForceTab();

    // chart builder (motion, damping, wave forces)
    void rebuildGenericCharts(const std::vector<MotionFile>& files,
                              QListWidget* datasets,
                              QComboBox* viewMode,
                              QComboBox* dataType,
                              QVBoxLayout* chartLayout,
                              const std::vector<QCheckBox*>& modeChecks,
                              const QString& title,
                              const QStringList& modeLabels,
                              const QStringList& unitLabels);

    void rebuildMotionCharts();
    void rebuildAddedMassCharts();
    void rebuildDampingCharts();
    void rebuildWaveForceCharts();

    void styleChart(QChart* chart);
    void styleAxis(QValueAxis* axis);
    void clearLayout(QVBoxLayout* layout);

    void refreshMotionDatasets();
    void refreshAddedMassDatasets();
    void refreshDampingDatasets();
    void refreshWaveForceDatasets();
    QString blockLabel(const QString& body, double speed, double heading) const;

    std::vector<double> parseDataLine(const QString& line) const;
    std::vector<MotionBlock> parseMotionBlocks(const QString& content) const;
    std::vector<MatrixBlock> parseMatrixBlocks(const QString& content, int numDataCols) const;

    QPen datasetPen(int datasetIdx, const QColor& color, qreal width = 2.0) const;

private slots:
    void onLoadMotion();
    void onLoadAddedMass();
    void onLoadDamping();
    void onLoadWaveForce();
    void onRemoveMotion();
    void onRemoveAddedMass();
    void onRemoveDamping();
    void onRemoveWaveForce();
    void onMotionSettingsChanged();
    void onAdmsSettingsChanged();
    void onDampSettingsChanged();
    void onWaveSettingsChanged();
};


