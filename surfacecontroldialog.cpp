#include "surfacecontroldialog.h"
#include "theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <cmath>

SurfaceControlDialog::SurfaceControlDialog(int patchCount,
                                           const std::vector<bool>& flippedPatches,
                                           const std::vector<std::vector<QVector3D>>& patchPoints,
                                           QWidget* parent)
    : QDialog(parent), m_patchCount(patchCount)
{
    setWindowTitle("Surface Control (.srfctr)");
    setMinimumSize(750, 500);
    resize(800, 600);

    detectWaterline(patchPoints);
    applyDarkStyle();
    setupUI();
    populateTable(flippedPatches);
}

void SurfaceControlDialog::detectWaterline(const std::vector<std::vector<QVector3D>>& patchPoints)
{
    m_autoWaterline.resize(m_patchCount, false);

    for (int p = 0; p < m_patchCount && p < (int)patchPoints.size(); ++p)
    {
        bool hasAbove = false;
        bool hasBelow = false;

        for (const auto& pt : patchPoints[p])
        {
            if (!std::isfinite(pt.z())) continue;
            if (pt.z() > 0.001f) hasAbove = true;
            if (pt.z() < -0.001f) hasBelow = true;
            if (hasAbove && hasBelow) break;
        }

        m_autoWaterline[p] = (hasAbove && hasBelow);
    }
}

void SurfaceControlDialog::applyDarkStyle()
{
    setStyleSheet(darkDialogStyle());
}

void SurfaceControlDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    QGroupBox* globalGroup = new QGroupBox("Global Settings");
    QGridLayout* gg = new QGridLayout(globalGroup);
    gg->setSpacing(8);

    gg->addWidget(new QLabel("Surface Symmetry:"), 0, 0);
    m_symmetryCombo = new QComboBox();
    m_symmetryCombo->addItem("Full body (0)", 0);
    m_symmetryCombo->addItem("Half body — mirror (1)", 1);
    gg->addWidget(m_symmetryCombo, 0, 1);

    gg->addWidget(new QLabel("Scale X:"), 1, 0);
    m_xRatio = new QDoubleSpinBox(); m_xRatio->setRange(-1e6, 1e6); m_xRatio->setDecimals(6); m_xRatio->setValue(1.0);
    gg->addWidget(m_xRatio, 1, 1);
    gg->addWidget(new QLabel("Scale Y:"), 1, 2);
    m_yRatio = new QDoubleSpinBox(); m_yRatio->setRange(-1e6, 1e6); m_yRatio->setDecimals(6); m_yRatio->setValue(1.0);
    gg->addWidget(m_yRatio, 1, 3);
    gg->addWidget(new QLabel("Scale Z:"), 1, 4);
    m_zRatio = new QDoubleSpinBox(); m_zRatio->setRange(-1e6, 1e6); m_zRatio->setDecimals(6); m_zRatio->setValue(1.0);
    gg->addWidget(m_zRatio, 1, 5);

    gg->addWidget(new QLabel("Shift X (m):"), 2, 0);
    m_xShift = new QDoubleSpinBox(); m_xShift->setRange(-1e6, 1e6); m_xShift->setDecimals(4); m_xShift->setValue(0.0);
    gg->addWidget(m_xShift, 2, 1);
    gg->addWidget(new QLabel("Shift Y (m):"), 2, 2);
    m_yShift = new QDoubleSpinBox(); m_yShift->setRange(-1e6, 1e6); m_yShift->setDecimals(4); m_yShift->setValue(0.0);
    gg->addWidget(m_yShift, 2, 3);
    gg->addWidget(new QLabel("Shift Z (m):"), 2, 4);
    m_zShift = new QDoubleSpinBox(); m_zShift->setRange(-1e6, 1e6); m_zShift->setDecimals(4); m_zShift->setValue(0.0);
    gg->addWidget(m_zShift, 2, 5);

    mainLayout->addWidget(globalGroup);

    QGroupBox* bulkGroup = new QGroupBox("Bulk Set");
    QHBoxLayout* bl = new QHBoxLayout(bulkGroup);
    bl->setSpacing(8);

    bl->addWidget(new QLabel("Gauss U:"));
    m_bulkGaussU = new QSpinBox(); m_bulkGaussU->setRange(1, 100); m_bulkGaussU->setValue(8);
    bl->addWidget(m_bulkGaussU);
    bl->addWidget(new QLabel("Gauss V:"));
    m_bulkGaussV = new QSpinBox(); m_bulkGaussV->setRange(1, 100); m_bulkGaussV->setValue(8);
    bl->addWidget(m_bulkGaussV);

    m_applyAllBtn = new QPushButton("Apply to All");
    m_applyAllBtn->setObjectName("applyBtn");
    connect(m_applyAllBtn, &QPushButton::clicked, this, &SurfaceControlDialog::onApplyBulkGauss);
    bl->addWidget(m_applyAllBtn);

    bl->addStretch();

    m_autoWlBtn = new QPushButton("Auto-Detect Waterline");
    m_autoWlBtn->setObjectName("autoWlBtn");
    connect(m_autoWlBtn, &QPushButton::clicked, this, &SurfaceControlDialog::onAutoDetectWaterline);
    bl->addWidget(m_autoWlBtn);

    QCheckBox* allWl = new QCheckBox("All intersect waterline");
    connect(allWl, &QCheckBox::toggled, this, &SurfaceControlDialog::onSetAllWaterline);
    bl->addWidget(allWl);

    mainLayout->addWidget(bulkGroup);

    m_table = new QTableWidget(m_patchCount, 7);
    m_table->setHorizontalHeaderLabels({
        "Gauss U", "Gauss V", "Half V", "Flip Normal", "Waterline",
        "Lid U", "Lid V"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setDefaultSectionSize(28);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList rowHeaders;
    for (int i = 0; i < m_patchCount; ++i)
        rowHeaders << QString("Surface %1").arg(i);
    m_table->setVerticalHeaderLabels(rowHeaders);

    mainLayout->addWidget(m_table, 1);

    QLabel* lidNote = new QLabel(
        "Lid U / Lid V: Gauss points on the internal waterplane lid for irregular frequency removal.\n"
        "Only used when 'Remove irregular frequencies' is enabled in Run Configuration.");
    lidNote->setStyleSheet("color: #888888; font-size: 11px;");
    lidNote->setWordWrap(true);
    mainLayout->addWidget(lidNote);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setObjectName("cancelBtn");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);
    m_exportBtn = new QPushButton("Export .srfctr...");
    connect(m_exportBtn, &QPushButton::clicked, this, &SurfaceControlDialog::onExport);
    btnLayout->addWidget(m_exportBtn);
    mainLayout->addLayout(btnLayout);
}

void SurfaceControlDialog::populateTable(const std::vector<bool>& flippedPatches)
{
    for (int row = 0; row < m_patchCount; ++row)
    {
        QSpinBox* gu = new QSpinBox(); gu->setRange(1, 100); gu->setValue(8); gu->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(row, 0, gu);

        QSpinBox* gv = new QSpinBox(); gv->setRange(1, 100); gv->setValue(8); gv->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(row, 1, gv);

        QComboBox* hv = new QComboBox();
        hv->addItem("Full (0)", 0); hv->addItem("Half (1)", 1);
        m_table->setCellWidget(row, 2, hv);

        QComboBox* kn = new QComboBox();
        kn->addItem("Keep (0)", 0); kn->addItem("Flip (1)", 1);
        if (row < (int)flippedPatches.size() && flippedPatches[row])
            kn->setCurrentIndex(1);
        m_table->setCellWidget(row, 3, kn);

        QComboBox* wl = new QComboBox();
        wl->addItem("No (0)", 0); wl->addItem("Yes (1)", 1);

        if (row < (int)m_autoWaterline.size() && m_autoWaterline[row])
            wl->setCurrentIndex(1);
        m_table->setCellWidget(row, 4, wl);

        QSpinBox* lidU = new QSpinBox(); lidU->setRange(1, 100); lidU->setValue(10); lidU->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(row, 5, lidU);

        QSpinBox* lidV = new QSpinBox(); lidV->setRange(1, 100); lidV->setValue(10); lidV->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(row, 6, lidV);
    }
}



int SurfaceControlDialog::getSurfaceSymmetry() const { return m_symmetryCombo->currentData().toInt(); }
double SurfaceControlDialog::getXRatio() const { return m_xRatio->value(); }
double SurfaceControlDialog::getYRatio() const { return m_yRatio->value(); }
double SurfaceControlDialog::getZRatio() const { return m_zRatio->value(); }
double SurfaceControlDialog::getXShift() const { return m_xShift->value(); }
double SurfaceControlDialog::getYShift() const { return m_yShift->value(); }
double SurfaceControlDialog::getZShift() const { return m_zShift->value(); }

std::vector<SurfaceControlDialog::PatchControl> SurfaceControlDialog::getPatchControls() const
{
    std::vector<PatchControl> ctrls(m_patchCount);
    for (int r = 0; r < m_patchCount; ++r)
    {
        auto* gu = qobject_cast<QSpinBox*>(m_table->cellWidget(r, 0));
        auto* gv = qobject_cast<QSpinBox*>(m_table->cellWidget(r, 1));
        auto* hv = qobject_cast<QComboBox*>(m_table->cellWidget(r, 2));
        auto* kn = qobject_cast<QComboBox*>(m_table->cellWidget(r, 3));
        auto* wl = qobject_cast<QComboBox*>(m_table->cellWidget(r, 4));
        auto* lu = qobject_cast<QSpinBox*>(m_table->cellWidget(r, 5));
        auto* lv = qobject_cast<QSpinBox*>(m_table->cellWidget(r, 6));
        if (gu) ctrls[r].ngauss_u = gu->value();
        if (gv) ctrls[r].ngauss_v = gv->value();
        if (hv) ctrls[r].khalf    = hv->currentData().toInt();
        if (kn) ctrls[r].knormal  = kn->currentData().toInt();
        if (wl) ctrls[r].wlindex  = wl->currentData().toInt();
        if (lu) ctrls[r].ngaussfsu = lu->value();
        if (lv) ctrls[r].ngaussfsv = lv->value();
    }
    return ctrls;
}

bool SurfaceControlDialog::exportToFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    auto ctrls = getPatchControls();

    // nurbcontrol, kucontrol, kvcontrol, ksurfacesymmetry
    out << "1, 1, 0, " << getSurfaceSymmetry() << "\n";
    // scale ratios
    out << getXRatio() << ", " << getYRatio() << ", " << getZRatio() << "\n";
    // shifts
    out << getXShift() << ", " << getYShift() << ", " << getZShift() << "\n";
    // npatchbody, npatch
    out << m_patchCount << ", " << m_patchCount << "\n";

    // ngauss_u, ngauss_v, khalf, knormal, wlindex
    for (int i = 0; i < m_patchCount; ++i)
    {
        const auto& c = ctrls[i];
        out << c.ngauss_u << ", " << c.ngauss_v << ", "
            << c.khalf << ", " << c.knormal << ", " << c.wlindex << "\n";
    }

    out << "\n";
    for (int i = 0; i < m_patchCount; ++i)
    {
        const auto& c = ctrls[i];
        out << c.ngaussfsu << ", " << c.ngaussfsv << "\n";
    }

    file.close();
    return true;
}



void SurfaceControlDialog::onExport()
{
    QString path = QFileDialog::getSaveFileName(this, "Export Surface Control File", "",
                                                "Surface Control Files (*.srfctr);;All Files (*.*)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".srfctr", Qt::CaseInsensitive)) path += ".srfctr";

    if (exportToFile(path))
    {
        QMessageBox::information(this, "Export Successful",
                                 QString("Surface control file saved to:\n%1").arg(path));
        accept();
    }
    else
        QMessageBox::warning(this, "Export Failed", "Could not write to the specified file.");
}

void SurfaceControlDialog::onApplyBulkGauss()
{
    int gu = m_bulkGaussU->value();
    int gv = m_bulkGaussV->value();
    for (int r = 0; r < m_patchCount; ++r)
    {
        auto* su = qobject_cast<QSpinBox*>(m_table->cellWidget(r, 0));
        auto* sv = qobject_cast<QSpinBox*>(m_table->cellWidget(r, 1));
        if (su) su->setValue(gu);
        if (sv) sv->setValue(gv);
    }
}

void SurfaceControlDialog::onSetAllWaterline(bool checked)
{
    for (int r = 0; r < m_patchCount; ++r)
    {
        auto* wl = qobject_cast<QComboBox*>(m_table->cellWidget(r, 4));
        if (wl) wl->setCurrentIndex(checked ? 1 : 0);
    }
}

void SurfaceControlDialog::onAutoDetectWaterline()
{
    for (int r = 0; r < m_patchCount; ++r)
    {
        auto* wl = qobject_cast<QComboBox*>(m_table->cellWidget(r, 4));
        if (wl && r < (int)m_autoWaterline.size())
            wl->setCurrentIndex(m_autoWaterline[r] ? 1 : 0);
    }
}
