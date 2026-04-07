#include "principaldatadialog.h"
#include "theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>

PrincipalDataDialog::PrincipalDataDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Principal Data");
    setMinimumSize(550, 580);
    resize(600, 620);

    applyDarkStyle();
    setupUI();
}

void PrincipalDataDialog::applyDarkStyle()
{
    setStyleSheet(darkDialogStyle());
}

QDoubleSpinBox* PrincipalDataDialog::makeSpinBox(double min, double max, int decimals, double value)
{
    QDoubleSpinBox* sb = new QDoubleSpinBox();
    sb->setRange(min, max);
    sb->setDecimals(decimals);
    sb->setValue(value);
    return sb;
}

void PrincipalDataDialog::setupUI()
{
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setSpacing(10);
    main->setContentsMargins(12, 12, 12, 12);

    QGroupBox* dimGroup = new QGroupBox("Hull Dimensions");
    QGridLayout* dg = new QGridLayout(dimGroup);
    dg->setSpacing(8);

    dg->addWidget(new QLabel("Length (m):"), 0, 0);
    m_length = makeSpinBox(0, 1e6, 3, 0); dg->addWidget(m_length, 0, 1);

    dg->addWidget(new QLabel("Beam (m):"), 0, 2);
    m_beam = makeSpinBox(0, 1e6, 3, 0); dg->addWidget(m_beam, 0, 3);

    dg->addWidget(new QLabel("Draft (m):"), 1, 0);
    m_draft = makeSpinBox(0, 1e6, 3, 0); dg->addWidget(m_draft, 1, 1);

    dg->addWidget(new QLabel("Block Coeff:"), 1, 2);
    m_blockCoeff = makeSpinBox(0, 1.0, 4, 0); dg->addWidget(m_blockCoeff, 1, 3);

    dg->addWidget(new QLabel("Displacement (m³):"), 2, 0);
    m_displacement = makeSpinBox(0, 1e9, 2, 0); dg->addWidget(m_displacement, 2, 1);

    main->addWidget(dimGroup);

    QGroupBox* cogGroup = new QGroupBox("Center of Gravity");
    QGridLayout* cg = new QGridLayout(cogGroup);
    cg->setSpacing(8);

    cg->addWidget(new QLabel("Xg (m):"), 0, 0);
    m_xg = makeSpinBox(-1e6, 1e6, 4, 0); cg->addWidget(m_xg, 0, 1);

    cg->addWidget(new QLabel("Yg (m):"), 0, 2);
    m_yg = makeSpinBox(-1e6, 1e6, 4, 0); cg->addWidget(m_yg, 0, 3);

    cg->addWidget(new QLabel("Zg (m):"), 0, 4);
    m_zg = makeSpinBox(-1e6, 1e6, 4, 0); cg->addWidget(m_zg, 0, 5);

    main->addWidget(cogGroup);

    QGroupBox* rogGroup = new QGroupBox("Radii of Gyration");
    QGridLayout* rg = new QGridLayout(rogGroup);
    rg->setSpacing(8);

    rg->addWidget(new QLabel("Rxx — Roll (m):"), 0, 0);
    m_rxx = makeSpinBox(0, 1e6, 4, 0); rg->addWidget(m_rxx, 0, 1);

    rg->addWidget(new QLabel("Ryy — Pitch (m):"), 0, 2);
    m_ryy = makeSpinBox(0, 1e6, 4, 0); rg->addWidget(m_ryy, 0, 3);

    rg->addWidget(new QLabel("Rzz — Yaw (m):"), 0, 4);
    m_rzz = makeSpinBox(0, 1e6, 4, 0); rg->addWidget(m_rzz, 0, 5);

    // Estimate button
    m_estimateBtn = new QPushButton("Estimate from L, B");
    m_estimateBtn->setObjectName("estimateBtn");
    connect(m_estimateBtn, &QPushButton::clicked, this, &PrincipalDataDialog::onEstimateRadii);
    rg->addWidget(m_estimateBtn, 1, 0, 1, 2);

    // Cross terms (collapsed row)
    rg->addWidget(new QLabel("Cross terms:"), 2, 0);

    QHBoxLayout* crossRow1 = new QHBoxLayout();
    crossRow1->addWidget(new QLabel("Rxy:")); m_rxy = makeSpinBox(-1e6, 1e6, 4, 0); crossRow1->addWidget(m_rxy);
    crossRow1->addWidget(new QLabel("Rxz:")); m_rxz = makeSpinBox(-1e6, 1e6, 4, 0); crossRow1->addWidget(m_rxz);
    crossRow1->addWidget(new QLabel("Ryx:")); m_ryx = makeSpinBox(-1e6, 1e6, 4, 0); crossRow1->addWidget(m_ryx);
    rg->addLayout(crossRow1, 2, 1, 1, 5);

    QHBoxLayout* crossRow2 = new QHBoxLayout();
    crossRow2->addWidget(new QLabel("Ryz:")); m_ryz = makeSpinBox(-1e6, 1e6, 4, 0); crossRow2->addWidget(m_ryz);
    crossRow2->addWidget(new QLabel("Rzx:")); m_rzx = makeSpinBox(-1e6, 1e6, 4, 0); crossRow2->addWidget(m_rzx);
    crossRow2->addWidget(new QLabel("Rzy:")); m_rzy = makeSpinBox(-1e6, 1e6, 4, 0); crossRow2->addWidget(m_rzy);
    rg->addLayout(crossRow2, 3, 1, 1, 5);

    main->addWidget(rogGroup);

    // damping stuff
    QGroupBox* dampGroup = new QGroupBox("Roll Damping & Stability");
    QGridLayout* dmpg = new QGridLayout(dampGroup);
    dmpg->setSpacing(8);

    dmpg->addWidget(new QLabel("Viscous Roll Damping (% critical):"), 0, 0);
    m_dampv44 = makeSpinBox(0, 1.0, 4, 0.05); dmpg->addWidget(m_dampv44, 0, 1);

    dmpg->addWidget(new QLabel("Transverse GM (m):"), 0, 2);
    m_gmt = makeSpinBox(-1e6, 1e6, 4, 0); dmpg->addWidget(m_gmt, 0, 3);

    main->addWidget(dampGroup);


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

//estimation

void PrincipalDataDialog::onEstimateRadii()
{
    double L = m_length->value();
    double B = m_beam->value();

    if (L <= 0 || B <= 0)
    {
        QMessageBox::information(this, "Estimate",
                                 "Enter length and beam first.");
        return;
    }

    // MAPS defaults: rxx = 0.35*B, ryy = rzz = 0.25*L
    m_rxx->setValue(0.35 * B);
    m_ryy->setValue(0.25 * L);
    m_rzz->setValue(0.25 * L);
}



PrincipalDataDialog::PrincipalData PrincipalDataDialog::getData() const
{
    PrincipalData d;
    d.length = m_length->value();
    d.beam = m_beam->value();
    d.draft = m_draft->value();
    d.blockCoeff = m_blockCoeff->value();
    d.displacement = m_displacement->value();
    d.xg = m_xg->value(); d.yg = m_yg->value(); d.zg = m_zg->value();
    d.rxx = m_rxx->value(); d.ryy = m_ryy->value(); d.rzz = m_rzz->value();
    d.rxy = m_rxy->value(); d.rxz = m_rxz->value();
    d.ryx = m_ryx->value(); d.ryz = m_ryz->value();
    d.rzx = m_rzx->value(); d.rzy = m_rzy->value();
    d.dampv44 = m_dampv44->value();
    d.gmt = m_gmt->value();
    return d;
}

void PrincipalDataDialog::setData(const PrincipalData& d)
{
    m_length->setValue(d.length); m_beam->setValue(d.beam); m_draft->setValue(d.draft);
    m_blockCoeff->setValue(d.blockCoeff); m_displacement->setValue(d.displacement);
    m_xg->setValue(d.xg); m_yg->setValue(d.yg); m_zg->setValue(d.zg);
    m_rxx->setValue(d.rxx); m_ryy->setValue(d.ryy); m_rzz->setValue(d.rzz);
    m_rxy->setValue(d.rxy); m_rxz->setValue(d.rxz);
    m_ryx->setValue(d.ryx); m_ryz->setValue(d.ryz);
    m_rzx->setValue(d.rzx); m_rzy->setValue(d.rzy);
    m_dampv44->setValue(d.dampv44); m_gmt->setValue(d.gmt);
}

QString PrincipalDataDialog::generateBlock() const
{
    auto d = getData();
    QString block;
    QTextStream out(&block);

    out << "&principaldata\n";
    out << d.length << "\n";      // sl
    out << d.beam << "\n";        // sb
    out << d.draft << "\n";       // st
    out << d.blockCoeff << "\n";  // cb
    out << d.displacement << "\n";// vol
    out << d.xg << "\n";          // xg
    out << d.yg << "\n";          // yg
    out << d.zg << "\n";          // zg
    out << d.rxx << "\n";         // rxx
    out << d.ryy << "\n";         // ryy
    out << d.rzz << "\n";         // rzz
    out << d.rxy << "\n";
    out << d.rxz << "\n";
    out << d.ryx << "\n";
    out << d.ryz << "\n";
    out << d.rzx << "\n";
    out << d.rzy << "\n";
    out << d.dampv44 << "\n";
    out << d.gmt << "\n";
    out << "/endprincipaldata\n";

    return block;
}
