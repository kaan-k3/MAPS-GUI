#pragma once

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QGroupBox>
#include <QTabWidget>
#include <vector>

class RunConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunConfigDialog(QWidget* parent = nullptr);


    struct RunControls {
        int kfreq = 1;             // 1=list, 2=start+step
        int ktrim = 1;             // 1=auto trim
        double trimPrec = 1e-5;
        int kmterm = -1;           // -1=no m-terms (zero speed)
        int kirregular = 0;        // 0=no, 1=remove irregular freq
        int kdrift = 0;            // 0=no drift forces
        int kbodymotion = 1;       // 1=floating, 0=fixed
        int kforwardspeed = 0;     // 0=zero-speed GF, 1=forward-speed GF
        int kcatamaran = 0;        // 0=independent bodies
        int kfsintegral = 0;       // 0=no line integral
        int kload = 0;             // 0=no shear/bending
        int kdamp = 0;             // 0=no viscous roll damping input
        int kpoi = 0;              // 0=no points of interest
        int krandomsea = 0;        // 0=no statistical values
        int kexternalrestoring = 0;
        int kexternaldamping = 0;
        int kfieldelevation = 0;
        int kwalleffect = 0;
        int knurbssmooth = 0;
    };


    struct WallData {
        int numwall = 1;           // 1 or 2 walls
        int numimage = 1;          // number of images for wall representation
        std::vector<double> distancewall1;  // per-body distance to wall 1
        std::vector<double> distancewall2;  // per-body distance to wall 2 (if numwall=2)
    };


    struct BodyDef {
        QString name = "body1";
        double x = 0.0;           // x position in global coords (m)
        double y = 0.0;           // y position in global coords (m)
        double angle = 0.0;       // heading angle (deg)
        QString igesPath;          // path to IGES geometry file
    };


    struct OptionalFiles {
        QString massFile;
        QString exRestoreFile;
        QString exDampFile;
        QString poiFile;
        QString fieldFile;
    };

    RunControls getRunControls() const;
    std::vector<BodyDef> getBodies() const;
    std::vector<bool> getRestrainedModes() const;
    OptionalFiles getOptionalFiles() const;
    WallData getWallData() const;

    void setRunControls(const RunControls& rc);
    void setBodies(const std::vector<BodyDef>& bodies);
    void setRestrainedModes(const std::vector<bool>& modes);
    void setWallData(const WallData& wd);

    QString generateRunControlsBlock() const;
    QString generateBodiesBlock() const;
    QString generateRestrainedModesBlock() const;

private:
    QTabWidget* m_tabs = nullptr;
    QComboBox* m_bodyMotion = nullptr;
    QComboBox* m_mterm = nullptr;
    QCheckBox* m_autoTrim = nullptr;
    QDoubleSpinBox* m_trimPrec = nullptr;
    QCheckBox* m_irregFreq = nullptr;
    QCheckBox* m_driftForces = nullptr;
    QCheckBox* m_forwardSpeedGF = nullptr;
    QCheckBox* m_catamaran = nullptr;
    QCheckBox* m_fsIntegral = nullptr;
    QCheckBox* m_computeLoads = nullptr;
    QCheckBox* m_viscousDamp = nullptr;
    QCheckBox* m_randomSea = nullptr;
    QCheckBox* m_wallEffect = nullptr;
    QCheckBox* m_nurbsSmooth = nullptr;


    QGroupBox*      m_wallGroup = nullptr;
    QComboBox*      m_numWalls = nullptr;      // 1 or 2
    QSpinBox*       m_numImages = nullptr;
    QDoubleSpinBox* m_distWall1 = nullptr;     // single-body distance to wall 1
    QDoubleSpinBox* m_distWall2 = nullptr;     // single-body distance to wall 2


    QSpinBox*     m_numBodies = nullptr;
    QTableWidget* m_bodyTable = nullptr;


    QCheckBox* m_restrainSurge = nullptr;
    QCheckBox* m_restrainSway = nullptr;
    QCheckBox* m_restrainHeave = nullptr;
    QCheckBox* m_restrainRoll = nullptr;
    QCheckBox* m_restrainPitch = nullptr;
    QCheckBox* m_restrainYaw = nullptr;

    QLineEdit* m_massPath = nullptr;
    QLineEdit* m_exRestorePath = nullptr;
    QLineEdit* m_exDampPath = nullptr;
    QLineEdit* m_poiPath = nullptr;
    QLineEdit* m_fieldPath = nullptr;

    QPushButton* m_okBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;

    void setupUI();
    void applyDarkStyle();
    QWidget* createRunControlsTab();
    QWidget* createBodiesTab();
    QWidget* createModesTab();
    QWidget* createFilesTab();

    QPushButton* makeBrowseButton(QLineEdit* target, const QString& filter);

private slots:
    void onNumBodiesChanged(int n);
    void onComputeLoadsToggled(bool on);
    void onWallEffectToggled(bool on);
};

