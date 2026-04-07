#include "runconfigdialog.h"
#include "theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QTextStream>
#include <qheaderview.h>

RunConfigDialog::RunConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Run Configuration");
    setMinimumSize(620, 520);
    resize(680, 560);

    applyDarkStyle();
    setupUI();
}

void RunConfigDialog::applyDarkStyle()
{
    setStyleSheet(darkDialogStyle());
}

void RunConfigDialog::setupUI()
{
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setSpacing(10);
    main->setContentsMargins(12, 12, 12, 12);

    m_tabs = new QTabWidget();
    m_tabs->addTab(createRunControlsTab(), "Run Controls");
    m_tabs->addTab(createBodiesTab(), "Bodies");
    m_tabs->addTab(createModesTab(), "Restrained Modes");
    m_tabs->addTab(createFilesTab(), "Optional Files");
    main->addWidget(m_tabs);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setObjectName("cancelBtn");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);
    m_okBtn = new QPushButton("OK");
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_okBtn);
    main->addLayout(btnLayout);
}

QWidget* RunConfigDialog::createRunControlsTab()
{
    QWidget* w = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout(w);
    vl->setSpacing(8);

    QGroupBox* solverGroup = new QGroupBox("Solver Settings");
    QGridLayout* sg = new QGridLayout(solverGroup);
    sg->setSpacing(8);

    sg->addWidget(new QLabel("Body Motion:"), 0, 0);
    m_bodyMotion = new QComboBox();
    m_bodyMotion->addItem("Floating (free motions)", 1);
    m_bodyMotion->addItem("Fixed body", 0);
    sg->addWidget(m_bodyMotion, 0, 1);

    sg->addWidget(new QLabel("M-terms:"), 1, 0);
    m_mterm = new QComboBox();
    m_mterm->addItem("None (zero speed) (-1)", -1);
    m_mterm->addItem("Simplified (forward speed) (0)", 0);
    m_mterm->addItem("Full double-body (forward speed) (1)", 1);
    sg->addWidget(m_mterm, 1, 1);

    m_autoTrim = new QCheckBox("Auto surface trimming at waterline");
    m_autoTrim->setChecked(true);
    sg->addWidget(m_autoTrim, 2, 0, 1, 2);

    sg->addWidget(new QLabel("Trim precision:"), 3, 0);
    m_trimPrec = new QDoubleSpinBox();
    m_trimPrec->setRange(1e-10, 1.0); m_trimPrec->setDecimals(8);
    m_trimPrec->setValue(1e-5);
    sg->addWidget(m_trimPrec, 3, 1);

    vl->addWidget(solverGroup);

    QGroupBox* optGroup = new QGroupBox("Computation Options");
    QGridLayout* og = new QGridLayout(optGroup);
    og->setSpacing(6);

    m_irregFreq = new QCheckBox("Remove irregular frequencies");
    og->addWidget(m_irregFreq, 0, 0);

    m_driftForces = new QCheckBox("Compute drift forces");
    og->addWidget(m_driftForces, 0, 1);

    m_forwardSpeedGF = new QCheckBox("Forward-speed Green function");
    og->addWidget(m_forwardSpeedGF, 1, 0);

    m_fsIntegral = new QCheckBox("Line integral (forward speed)");
    og->addWidget(m_fsIntegral, 1, 1);

    m_catamaran = new QCheckBox("Catamaran / multi-hull (rigid body)");
    og->addWidget(m_catamaran, 2, 0);

    m_computeLoads = new QCheckBox("Compute shear forces / bending moments");
    connect(m_computeLoads, &QCheckBox::toggled, this, &RunConfigDialog::onComputeLoadsToggled);
    og->addWidget(m_computeLoads, 2, 1);

    m_viscousDamp = new QCheckBox("Viscous roll damping (% critical)");
    og->addWidget(m_viscousDamp, 3, 0);

    m_randomSea = new QCheckBox("Random sea statistics");
    og->addWidget(m_randomSea, 3, 1);

    m_wallEffect = new QCheckBox("Wall effect (image method)");
    connect(m_wallEffect, &QCheckBox::toggled, this, &RunConfigDialog::onWallEffectToggled);
    og->addWidget(m_wallEffect, 4, 0);

    m_nurbsSmooth = new QCheckBox("NURBS surface smoothing");
    og->addWidget(m_nurbsSmooth, 4, 1);

    vl->addWidget(optGroup);

    m_wallGroup = new QGroupBox("Wall Configuration (§5.1.6)");
    QGridLayout* wg = new QGridLayout(m_wallGroup);
    wg->setSpacing(6);

    wg->addWidget(new QLabel("Number of walls:"), 0, 0);
    m_numWalls = new QComboBox();
    m_numWalls->addItem("1 wall", 1);
    m_numWalls->addItem("2 walls", 2);
    connect(m_numWalls, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) { m_distWall2->setEnabled(idx == 1); });
    wg->addWidget(m_numWalls, 0, 1);

    wg->addWidget(new QLabel("Number of images:"), 1, 0);
    m_numImages = new QSpinBox();
    m_numImages->setRange(1, 20);
    m_numImages->setValue(1);
    wg->addWidget(m_numImages, 1, 1);

    wg->addWidget(new QLabel("Distance to wall 1 (m):"), 2, 0);
    m_distWall1 = new QDoubleSpinBox();
    m_distWall1->setRange(0, 1e6); m_distWall1->setDecimals(3); m_distWall1->setValue(0);
    wg->addWidget(m_distWall1, 2, 1);

    wg->addWidget(new QLabel("Distance to wall 2 (m):"), 3, 0);
    m_distWall2 = new QDoubleSpinBox();
    m_distWall2->setRange(0, 1e6); m_distWall2->setDecimals(3); m_distWall2->setValue(0);
    m_distWall2->setEnabled(false);  // only enabled when numwall=2
    wg->addWidget(m_distWall2, 3, 1);

    QLabel* wallNote = new QLabel(
        "Wall 1 is parallel to the X-axis on the +Y side.\n"
        "Wall 2 (if present) is on the -Y side.\n"
        "For multi-body, distances are per-body (first body used here).");
    wallNote->setStyleSheet("color: #888888; font-size: 11px;");
    wallNote->setWordWrap(true);
    wg->addWidget(wallNote, 4, 0, 1, 2);

    m_wallGroup->setVisible(false);
    vl->addWidget(m_wallGroup);

    vl->addStretch();

    return w;
}

QWidget* RunConfigDialog::createBodiesTab()
{
    QWidget* w = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout(w);
    vl->setSpacing(8);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Number of bodies:"));
    m_numBodies = new QSpinBox();
    m_numBodies->setRange(1, 20);
    m_numBodies->setValue(1);
    connect(m_numBodies, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &RunConfigDialog::onNumBodiesChanged);
    topRow->addWidget(m_numBodies);
    topRow->addStretch();
    vl->addLayout(topRow);

    m_bodyTable = new QTableWidget(0, 5);
    m_bodyTable->setHorizontalHeaderLabels({"Name", "X (m)", "Y (m)", "Heading (deg)", "IGES File"});
    m_bodyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_bodyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_bodyTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_bodyTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_bodyTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_bodyTable->setColumnWidth(0, 80);
    m_bodyTable->setColumnWidth(1, 80);
    m_bodyTable->setColumnWidth(2, 80);
    m_bodyTable->setColumnWidth(3, 100);
    m_bodyTable->verticalHeader()->setDefaultSectionSize(30);


    onNumBodiesChanged(1);

    vl->addWidget(m_bodyTable);

    QLabel* note = new QLabel(
        "Body positions are in the global coordinate system.\n"
        "Heading is the angle of body x-axis relative to global X-axis (degrees).\n"
        "Body name is used for geometry/data file association.\n"
        "IGES file is the hull geometry used for multi-body visualization.");
    note->setStyleSheet("color: #888888; font-size: 11px;");
    note->setWordWrap(true);
    vl->addWidget(note);

    vl->addStretch();
    return w;
}

void RunConfigDialog::onNumBodiesChanged(int n)
{
    int oldRows = m_bodyTable->rowCount();
    m_bodyTable->setRowCount(n);

    for (int row = oldRows; row < n; ++row)
    {
        QLineEdit* name = new QLineEdit(QString("body%1").arg(row + 1));
        m_bodyTable->setCellWidget(row, 0, name);

        QDoubleSpinBox* xPos = new QDoubleSpinBox();
        xPos->setRange(-1e6, 1e6); xPos->setDecimals(3); xPos->setValue(0);
        m_bodyTable->setCellWidget(row, 1, xPos);

        QDoubleSpinBox* yPos = new QDoubleSpinBox();
        yPos->setRange(-1e6, 1e6); yPos->setDecimals(3); yPos->setValue(0);
        m_bodyTable->setCellWidget(row, 2, yPos);

        QDoubleSpinBox* angle = new QDoubleSpinBox();
        angle->setRange(-360, 360); angle->setDecimals(2); angle->setValue(0);
        m_bodyTable->setCellWidget(row, 3, angle);


        QWidget* igesWidget = new QWidget();
        QHBoxLayout* hl = new QHBoxLayout(igesWidget);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(2);
        QLineEdit* igesPath = new QLineEdit();
        igesPath->setPlaceholderText("Select .igs file...");
        igesPath->setObjectName("igesPath");
        QPushButton* browseBtn = new QPushButton("...");
        browseBtn->setFixedWidth(28);
        connect(browseBtn, &QPushButton::clicked, this, [igesPath, this]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Select IGES File", QString(),
                "IGES Files (*.igs *.iges);;All Files (*.*)");
            if (!path.isEmpty())
                igesPath->setText(path);
        });
        hl->addWidget(igesPath);
        hl->addWidget(browseBtn);
        m_bodyTable->setCellWidget(row, 4, igesWidget);
    }
}

QWidget* RunConfigDialog::createModesTab()
{
    QWidget* w = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout(w);
    vl->setSpacing(12);

    QLabel* desc = new QLabel(
        "Check the modes you want to RESTRAIN (lock). "
        "Unchecked modes are free to move.");
    desc->setWordWrap(true);
    vl->addWidget(desc);

    QGroupBox* modesGroup = new QGroupBox("Motion Modes");
    QGridLayout* mg = new QGridLayout(modesGroup);
    mg->setSpacing(12);

    m_restrainSurge = new QCheckBox("Surge (x-translation)");
    mg->addWidget(m_restrainSurge, 0, 0);
    m_restrainSway = new QCheckBox("Sway (y-translation)");
    mg->addWidget(m_restrainSway, 0, 1);
    m_restrainHeave = new QCheckBox("Heave (z-translation)");
    mg->addWidget(m_restrainHeave, 1, 0);
    m_restrainRoll = new QCheckBox("Roll (rotation about x)");
    mg->addWidget(m_restrainRoll, 1, 1);
    m_restrainPitch = new QCheckBox("Pitch (rotation about y)");
    mg->addWidget(m_restrainPitch, 2, 0);
    m_restrainYaw = new QCheckBox("Yaw (rotation about z)");
    mg->addWidget(m_restrainYaw, 2, 1);

    vl->addWidget(modesGroup);

    QLabel* note = new QLabel(
        "Typical setups:\n"
        "  Ship in head seas: Restrain surge, sway, yaw (free heave, roll, pitch)\n"
        "  Fixed structure: Restrain all 6 modes\n"
        "  Free floating: Restrain none");
    note->setStyleSheet("color: #888888; font-size: 11px;");
    note->setWordWrap(true);
    vl->addWidget(note);

    vl->addStretch();
    return w;
}

QPushButton* RunConfigDialog::makeBrowseButton(QLineEdit* target, const QString& filter)
{
    QPushButton* btn = new QPushButton("Browse...");
    btn->setObjectName("browseBtn");
    connect(btn, &QPushButton::clicked, this, [this, target, filter]() {
        QString path = QFileDialog::getOpenFileName(this, "Select File", "", filter);
        if (!path.isEmpty()) target->setText(path);
    });
    return btn;
}

QWidget* RunConfigDialog::createFilesTab()
{
    QWidget* w = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout(w);
    vl->setSpacing(8);

    QLabel* desc = new QLabel(
        "These files are optional. Browse to select if needed for your analysis.");
    desc->setWordWrap(true);
    vl->addWidget(desc);

    QGroupBox* filesGroup = new QGroupBox("Optional Input Files");
    QGridLayout* fg = new QGridLayout(filesGroup);
    fg->setSpacing(8);


    fg->addWidget(new QLabel("Mass Distribution (.mass):"), 0, 0);
    m_massPath = new QLineEdit();
    m_massPath->setPlaceholderText("Required only for shear forces / bending moments");
    m_massPath->setReadOnly(true);
    fg->addWidget(m_massPath, 0, 1);
    fg->addWidget(makeBrowseButton(m_massPath, "Mass Files (*.mass);;All Files (*.*)"), 0, 2);


    fg->addWidget(new QLabel("External Restoring (.exrestore):"), 1, 0);
    m_exRestorePath = new QLineEdit();
    m_exRestorePath->setPlaceholderText("6x6 stiffness matrix (e.g. mooring)");
    m_exRestorePath->setReadOnly(true);
    fg->addWidget(m_exRestorePath, 1, 1);
    fg->addWidget(makeBrowseButton(m_exRestorePath, "Restore Files (*.exrestore);;All Files (*.*)"), 1, 2);

    fg->addWidget(new QLabel("External Damping (.exdamp):"), 2, 0);
    m_exDampPath = new QLineEdit();
    m_exDampPath->setPlaceholderText("6x6 damping matrix");
    m_exDampPath->setReadOnly(true);
    fg->addWidget(m_exDampPath, 2, 1);
    fg->addWidget(makeBrowseButton(m_exDampPath, "Damping Files (*.exdamp);;All Files (*.*)"), 2, 2);

    fg->addWidget(new QLabel("Points of Interest (.poi):"), 3, 0);
    m_poiPath = new QLineEdit();
    m_poiPath->setPlaceholderText("Coordinates for motion output");
    m_poiPath->setReadOnly(true);
    fg->addWidget(m_poiPath, 3, 1);
    fg->addWidget(makeBrowseButton(m_poiPath, "POI Files (*.poi);;All Files (*.*)"), 3, 2);


    fg->addWidget(new QLabel("Field Points (.field):"), 4, 0);
    m_fieldPath = new QLineEdit();
    m_fieldPath->setPlaceholderText("Points for wave elevation computation");
    m_fieldPath->setReadOnly(true);
    fg->addWidget(m_fieldPath, 4, 1);
    fg->addWidget(makeBrowseButton(m_fieldPath, "Field Files (*.field);;All Files (*.*)"), 4, 2);

    vl->addWidget(filesGroup);
    vl->addStretch();
    return w;
}

void RunConfigDialog::onComputeLoadsToggled(bool on)
{
    // Highlight mass file field if loads are enabled but no file selected
    if (on && m_massPath->text().isEmpty())
    {
        m_massPath->setStyleSheet("border: 1px solid #cc6600;");
    }
    else
    {
        m_massPath->setStyleSheet("");
    }
}

void RunConfigDialog::onWallEffectToggled(bool on)
{
    m_wallGroup->setVisible(on);
}

RunConfigDialog::RunControls RunConfigDialog::getRunControls() const
{
    RunControls rc;
    rc.kbodymotion = m_bodyMotion->currentData().toInt();
    rc.kmterm = m_mterm->currentData().toInt();
    rc.ktrim = m_autoTrim->isChecked() ? 1 : 0;
    rc.trimPrec = m_trimPrec->value();
    rc.kirregular = m_irregFreq->isChecked() ? 1 : 0;
    rc.kdrift = m_driftForces->isChecked() ? 1 : 0;
    rc.kforwardspeed = m_forwardSpeedGF->isChecked() ? 1 : 0;
    rc.kcatamaran = m_catamaran->isChecked() ? 1 : 0;
    rc.kfsintegral = m_fsIntegral->isChecked() ? 1 : 0;
    rc.kload = m_computeLoads->isChecked() ? 1 : 0;
    rc.kdamp = m_viscousDamp->isChecked() ? 2 : 0;
    rc.krandomsea = m_randomSea->isChecked() ? 1 : 0;
    rc.kwalleffect = m_wallEffect->isChecked() ? 1 : 0;
    rc.knurbssmooth = m_nurbsSmooth->isChecked() ? 1 : 0;
    rc.kexternalrestoring = m_exRestorePath->text().isEmpty() ? 0 : 1;
    rc.kexternaldamping = m_exDampPath->text().isEmpty() ? 0 : 1;
    rc.kpoi = m_poiPath->text().isEmpty() ? 0 : 1;
    rc.kfieldelevation = m_fieldPath->text().isEmpty() ? 0 : 1;
    return rc;
}

std::vector<RunConfigDialog::BodyDef> RunConfigDialog::getBodies() const
{
    int n = m_numBodies->value();
    std::vector<BodyDef> bodies(n);
    for (int i = 0; i < n && i < m_bodyTable->rowCount(); ++i)
    {
        auto* nm = qobject_cast<QLineEdit*>(m_bodyTable->cellWidget(i, 0));
        auto* xp = qobject_cast<QDoubleSpinBox*>(m_bodyTable->cellWidget(i, 1));
        auto* yp = qobject_cast<QDoubleSpinBox*>(m_bodyTable->cellWidget(i, 2));
        auto* ag = qobject_cast<QDoubleSpinBox*>(m_bodyTable->cellWidget(i, 3));
        if (nm) bodies[i].name = nm->text();
        if (xp) bodies[i].x = xp->value();
        if (yp) bodies[i].y = yp->value();
        if (ag) bodies[i].angle = ag->value();

        QWidget* igesWidget = m_bodyTable->cellWidget(i, 4);
        if (igesWidget) {
            auto* igesLE = igesWidget->findChild<QLineEdit*>("igesPath");
            if (igesLE) bodies[i].igesPath = igesLE->text();
        }
    }
    return bodies;
}

std::vector<bool> RunConfigDialog::getRestrainedModes() const
{
    return {
        m_restrainSurge->isChecked(),
        m_restrainSway->isChecked(),
        m_restrainHeave->isChecked(),
        m_restrainRoll->isChecked(),
        m_restrainPitch->isChecked(),
        m_restrainYaw->isChecked()
    };
}

RunConfigDialog::OptionalFiles RunConfigDialog::getOptionalFiles() const
{
    OptionalFiles f;
    f.massFile = m_massPath->text();
    f.exRestoreFile = m_exRestorePath->text();
    f.exDampFile = m_exDampPath->text();
    f.poiFile = m_poiPath->text();
    f.fieldFile = m_fieldPath->text();
    return f;
}

void RunConfigDialog::setRunControls(const RunControls& rc)
{
    m_bodyMotion->setCurrentIndex(rc.kbodymotion == 0 ? 1 : 0);
    m_mterm->setCurrentIndex(rc.kmterm == -1 ? 0 : (rc.kmterm == 0 ? 1 : 2));
    m_autoTrim->setChecked(rc.ktrim == 1);
    m_trimPrec->setValue(rc.trimPrec);
    m_irregFreq->setChecked(rc.kirregular == 1);
    m_driftForces->setChecked(rc.kdrift == 1);
    m_forwardSpeedGF->setChecked(rc.kforwardspeed == 1);
    m_catamaran->setChecked(rc.kcatamaran == 1);
    m_fsIntegral->setChecked(rc.kfsintegral == 1);
    m_computeLoads->setChecked(rc.kload == 1);
    m_viscousDamp->setChecked(rc.kdamp == 2);
    m_randomSea->setChecked(rc.krandomsea == 1);
    m_wallEffect->setChecked(rc.kwalleffect == 1);
    m_nurbsSmooth->setChecked(rc.knurbssmooth == 1);
}

void RunConfigDialog::setBodies(const std::vector<BodyDef>& bodies)
{
    m_numBodies->setValue((int)bodies.size());
    onNumBodiesChanged((int)bodies.size());
    for (int i = 0; i < (int)bodies.size() && i < m_bodyTable->rowCount(); ++i)
    {
        auto* nm = qobject_cast<QLineEdit*>(m_bodyTable->cellWidget(i, 0));
        auto* xp = qobject_cast<QDoubleSpinBox*>(m_bodyTable->cellWidget(i, 1));
        auto* yp = qobject_cast<QDoubleSpinBox*>(m_bodyTable->cellWidget(i, 2));
        auto* ag = qobject_cast<QDoubleSpinBox*>(m_bodyTable->cellWidget(i, 3));
        if (nm) nm->setText(bodies[i].name);
        if (xp) xp->setValue(bodies[i].x);
        if (yp) yp->setValue(bodies[i].y);
        if (ag) ag->setValue(bodies[i].angle);

        QWidget* igesWidget = m_bodyTable->cellWidget(i, 4);
        if (igesWidget) {
            auto* igesLE = igesWidget->findChild<QLineEdit*>("igesPath");
            if (igesLE) igesLE->setText(bodies[i].igesPath);
        }
    }
}

void RunConfigDialog::setRestrainedModes(const std::vector<bool>& modes)
{
    if (modes.size() >= 6)
    {
        m_restrainSurge->setChecked(modes[0]);
        m_restrainSway->setChecked(modes[1]);
        m_restrainHeave->setChecked(modes[2]);
        m_restrainRoll->setChecked(modes[3]);
        m_restrainPitch->setChecked(modes[4]);
        m_restrainYaw->setChecked(modes[5]);
    }
}

RunConfigDialog::WallData RunConfigDialog::getWallData() const
{
    WallData wd;
    wd.numwall = m_numWalls->currentData().toInt();
    wd.numimage = m_numImages->value();
    wd.distancewall1 = {m_distWall1->value()};
    wd.distancewall2 = {m_distWall2->value()};
    return wd;
}

void RunConfigDialog::setWallData(const WallData& wd)
{
    m_numWalls->setCurrentIndex(wd.numwall == 2 ? 1 : 0);
    m_numImages->setValue(wd.numimage);
    if (!wd.distancewall1.empty()) m_distWall1->setValue(wd.distancewall1[0]);
    if (!wd.distancewall2.empty()) m_distWall2->setValue(wd.distancewall2[0]);
}

QString RunConfigDialog::generateRunControlsBlock() const
{
    auto rc = getRunControls();
    QString block;
    QTextStream out(&block);

    out << "&runcontrols\n";
    out << "1\n";                    // isolve
    out << rc.kfreq << "\n";
    out << "1\n";                    // kgeometry
    out << "2\n";                    // knurbformat (IGES)
    out << rc.ktrim << "\n";
    out << rc.trimPrec << "\n";
    out << "1\n";                    // koutput (dimensional)
    out << rc.kmterm << "\n";
    out << rc.kirregular << "\n";
    out << rc.kdrift << "\n";
    out << rc.kbodymotion << "\n";
    out << rc.kforwardspeed << "\n";
    out << rc.kcatamaran << "\n";
    out << rc.kfsintegral << "\n";
    out << rc.kload << "\n";
    out << rc.kdamp << "\n";
    out << rc.kpoi << "\n";
    out << rc.krandomsea << "\n";
    out << rc.kexternalrestoring << "\n";
    out << rc.kexternaldamping << "\n";
    out << rc.kfieldelevation << "\n";
    out << rc.kwalleffect << "\n";
    out << rc.knurbssmooth << "\n";
    out << "/endruncontrols\n";

    return block;
}

QString RunConfigDialog::generateBodiesBlock() const
{
    auto bodies = getBodies();
    QString block;
    QTextStream out(&block);

    out << "&multiplebodys\n";
    out << bodies.size() << "\n";

    // bodyx list
    for (const auto& b : bodies) out << b.x << "\n";
    // bodyy list
    for (const auto& b : bodies) out << b.y << "\n";
    // bodyangle list
    for (const auto& b : bodies) out << b.angle << "\n";
    // bodyname list
    for (const auto& b : bodies) out << b.name << "\n";

    out << "/endmultiplebodys\n";

    return block;
}

QString RunConfigDialog::generateRestrainedModesBlock() const
{
    auto modes = getRestrainedModes();
    QString block;
    QTextStream out(&block);

    out << "&restrainmodes\n";
    for (int i = 0; i < 6; ++i)
        out << (modes[i] ? 1 : 0) << "\n";
    out << "/endrestainmodes\n";

    return block;
}
