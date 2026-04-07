#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVector3D>
#include <vector>

class SurfaceControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SurfaceControlDialog(int patchCount,
                                  const std::vector<bool>& flippedPatches,
                                  const std::vector<std::vector<QVector3D>>& patchPoints,
                                  QWidget* parent = nullptr);

    struct PatchControl {
        int ngauss_u = 8;
        int ngauss_v = 8;
        int khalf = 0;
        int knormal = 0;
        int wlindex = 0;
        // Lid Gauss points
        int ngaussfsu = 10;
        int ngaussfsv = 10;
    };

    int getSurfaceSymmetry() const;
    double getXRatio() const;
    double getYRatio() const;
    double getZRatio() const;
    double getXShift() const;
    double getYShift() const;
    double getZShift() const;
    std::vector<PatchControl> getPatchControls() const;
    bool exportToFile(const QString& path) const;

private:
    int m_patchCount;

    QComboBox*      m_symmetryCombo = nullptr;
    QDoubleSpinBox* m_xRatio = nullptr;
    QDoubleSpinBox* m_yRatio = nullptr;
    QDoubleSpinBox* m_zRatio = nullptr;
    QDoubleSpinBox* m_xShift = nullptr;
    QDoubleSpinBox* m_yShift = nullptr;
    QDoubleSpinBox* m_zShift = nullptr;

    QTableWidget* m_table = nullptr;

    QPushButton* m_exportBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QPushButton* m_applyAllBtn = nullptr;
    QPushButton* m_autoWlBtn = nullptr;

    QSpinBox* m_bulkGaussU = nullptr;
    QSpinBox* m_bulkGaussV = nullptr;

    std::vector<bool> m_autoWaterline;

    void setupUI();
    void applyDarkStyle();
    void populateTable(const std::vector<bool>& flippedPatches);
    void detectWaterline(const std::vector<std::vector<QVector3D>>& patchPoints);

private slots:
    void onExport();
    void onApplyBulkGauss();
    void onSetAllWaterline(bool checked);
    void onAutoDetectWaterline();
};


