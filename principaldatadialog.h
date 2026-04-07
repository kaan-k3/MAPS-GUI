#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>

class PrincipalDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrincipalDataDialog(QWidget* parent = nullptr);

    struct PrincipalData {
        double length = 0.0;        // sl — ship length (m)
        double beam = 0.0;          // sb — beam (m)
        double draft = 0.0;         // st — draft (m)
        double blockCoeff = 0.0;    // cb — block coefficient
        double displacement = 0.0;  // vol — displacement volume (m^3), 0 if unknown

        double xg = 0.0;            // center of gravity x (m)
        double yg = 0.0;            // center of gravity y (m)
        double zg = 0.0;            // center of gravity z (m)

        double rxx = 0.0;           // roll radius of gyration (m)
        double ryy = 0.0;           // pitch radius of gyration (m)
        double rzz = 0.0;           // yaw radius of gyration (m)

        // Cross terms (usually 0)
        double rxy = 0.0, rxz = 0.0;
        double ryx = 0.0, ryz = 0.0;
        double rzx = 0.0, rzy = 0.0;

        double dampv44 = 0.05;      // viscous roll damping factor (fraction of critical)
        double gmt = 0.0;           // transverse GM (m)
    };

    PrincipalData getData() const;
    void setData(const PrincipalData& d);

    // Generate the &principaldata block for the .in file
    QString generateBlock() const;

private:
    // Dimensions
    QDoubleSpinBox* m_length = nullptr;
    QDoubleSpinBox* m_beam = nullptr;
    QDoubleSpinBox* m_draft = nullptr;
    QDoubleSpinBox* m_blockCoeff = nullptr;
    QDoubleSpinBox* m_displacement = nullptr;

    // COG
    QDoubleSpinBox* m_xg = nullptr;
    QDoubleSpinBox* m_yg = nullptr;
    QDoubleSpinBox* m_zg = nullptr;

    // Radii of gyration
    QDoubleSpinBox* m_rxx = nullptr;
    QDoubleSpinBox* m_ryy = nullptr;
    QDoubleSpinBox* m_rzz = nullptr;

    // with cross terms
    QDoubleSpinBox* m_rxy = nullptr;
    QDoubleSpinBox* m_rxz = nullptr;
    QDoubleSpinBox* m_ryx = nullptr;
    QDoubleSpinBox* m_ryz = nullptr;
    QDoubleSpinBox* m_rzx = nullptr;
    QDoubleSpinBox* m_rzy = nullptr;

    // Damping stuff
    QDoubleSpinBox* m_dampv44 = nullptr;
    QDoubleSpinBox* m_gmt = nullptr;

    QPushButton* m_okBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QPushButton* m_estimateBtn = nullptr;

    void setupUI();
    void applyDarkStyle();

    QDoubleSpinBox* makeSpinBox(double min, double max, int decimals, double value);

private slots:
    void onEstimateRadii();
};

