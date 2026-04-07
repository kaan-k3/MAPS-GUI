#include "wavesettingsdialog.h"
#include "theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextStream>

WaveSettingsDialog::WaveSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Wave & Environment Settings");
    setMinimumSize(650, 700);
    resize(700, 780);

    applyDarkStyle();
    setupUI();
}

void WaveSettingsDialog::applyDarkStyle()
{
    setStyleSheet(darkDialogStyle());
}

void WaveSettingsDialog::setupUI()
{
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setSpacing(10);
    main->setContentsMargins(12, 12, 12, 12);

    QGroupBox* depthGroup = new QGroupBox("Water Depth");
    QHBoxLayout* dl = new QHBoxLayout(depthGroup);
    dl->setSpacing(8);

    dl->addWidget(new QLabel("Depth:"));
    m_depthMode = new QComboBox();
    m_depthMode->addItem("Deep water", 1);
    m_depthMode->addItem("Finite depth", 2);
    connect(m_depthMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WaveSettingsDialog::onDepthModeChanged);
    dl->addWidget(m_depthMode);

    m_depthLabel = new QLabel("Depth (m):");
    m_depthLabel->setVisible(false);
    dl->addWidget(m_depthLabel);
    m_waterDepth = new QDoubleSpinBox();
    m_waterDepth->setRange(0.1, 1e6); m_waterDepth->setDecimals(2); m_waterDepth->setValue(1000.0);
    m_waterDepth->setVisible(false);
    dl->addWidget(m_waterDepth);
    dl->addStretch();

    main->addWidget(depthGroup);

    QGroupBox* speedGroup = new QGroupBox("Body Speeds");
    QVBoxLayout* sl = new QVBoxLayout(speedGroup);
    sl->addWidget(new QLabel("Speeds in knots (comma-separated):"));
    m_speedList = new QTextEdit();
    m_speedList->setMinimumHeight(32);
    m_speedList->setMaximumHeight(44);
    m_speedList->setPlaceholderText("e.g. 0, 5, 10, 15");
    m_speedList->setText("0");
    sl->addWidget(m_speedList);

    main->addWidget(speedGroup);


    QGroupBox* waveGroup = new QGroupBox("Incident Waves");
    QGridLayout* wg = new QGridLayout(waveGroup);
    wg->setSpacing(10);
    wg->setContentsMargins(10, 16, 10, 10);


    wg->addWidget(new QLabel("Wave Amplitude (m):"), 0, 0);
    m_amplitude = new QDoubleSpinBox();
    m_amplitude->setRange(0.001, 1000); m_amplitude->setDecimals(3); m_amplitude->setValue(1.0);
    wg->addWidget(m_amplitude, 0, 1);


    wg->addWidget(new QLabel("Frequency Input:"), 1, 0);
    m_freqMode = new QComboBox();
    m_freqMode->addItem("List of frequencies", 1);
    m_freqMode->addItem("Start + increment", 2);
    connect(m_freqMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WaveSettingsDialog::onFreqModeChanged);
    wg->addWidget(m_freqMode, 1, 1);

    wg->addWidget(new QLabel("Num. Frequencies:"), 1, 2);
    m_nFreq = new QSpinBox(); m_nFreq->setRange(1, 100); m_nFreq->setValue(10);
    wg->addWidget(m_nFreq, 1, 3);


    m_freqListLabel = new QLabel("Frequencies (rad/s, comma-separated):");
    wg->addWidget(m_freqListLabel, 2, 0, 1, 2);
    m_freqList = new QTextEdit();
    m_freqList->setMinimumHeight(40);
    m_freqList->setMaximumHeight(60);
    m_freqList->setPlaceholderText("e.g. 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2");
    wg->addWidget(m_freqList, 3, 0, 1, 4);


    m_startFreqLabel = new QLabel("Start Freq (rad/s):");
    m_startFreqLabel->setVisible(false);
    wg->addWidget(m_startFreqLabel, 4, 0);
    m_startFreq = new QDoubleSpinBox();
    m_startFreq->setRange(0.001, 100); m_startFreq->setDecimals(4); m_startFreq->setValue(0.1);
    m_startFreq->setVisible(false);
    wg->addWidget(m_startFreq, 4, 1);

    m_freqStepLabel = new QLabel("Freq Step (rad/s):");
    m_freqStepLabel->setVisible(false);
    wg->addWidget(m_freqStepLabel, 4, 2);
    m_freqStep = new QDoubleSpinBox();
    m_freqStep->setRange(0.001, 100); m_freqStep->setDecimals(4); m_freqStep->setValue(0.1);
    m_freqStep->setVisible(false);
    wg->addWidget(m_freqStep, 4, 3);


    wg->addWidget(new QLabel("Num. Headings:"), 5, 0);
    m_nHead = new QSpinBox(); m_nHead->setRange(1, 30); m_nHead->setValue(1);
    wg->addWidget(m_nHead, 5, 1);

    wg->addWidget(new QLabel("Headings (deg, comma-separated):"), 6, 0, 1, 2);
    m_headingList = new QTextEdit();
    m_headingList->setMinimumHeight(36);
    m_headingList->setMaximumHeight(50);
    m_headingList->setPlaceholderText("e.g. 180 (180 = head seas)");
    m_headingList->setText("180");
    wg->addWidget(m_headingList, 7, 0, 1, 4);

    main->addWidget(waveGroup);


    m_useSpectrum = new QCheckBox("Enable Wave Spectrum (random sea analysis)");
    connect(m_useSpectrum, &QCheckBox::toggled, this, &WaveSettingsDialog::onSpectrumToggled);
    main->addWidget(m_useSpectrum);

    m_spectrumGroup = new QGroupBox("Wave Spectrum");
    m_spectrumGroup->setEnabled(false);
    QGridLayout* sg = new QGridLayout(m_spectrumGroup);
    sg->setSpacing(8);

    sg->addWidget(new QLabel("Spectrum Type:"), 0, 0);
    m_spectrumType = new QComboBox();
    m_spectrumType->addItem("Pierson-Moskowitz (1)", 1);
    m_spectrumType->addItem("JONSWAP (2)", 2);
    m_spectrumType->addItem("ISSC (3)", 3);
    m_spectrumType->addItem("ITTC (4)", 4);
    m_spectrumType->addItem("Bretschneider (5)", 5);
    m_spectrumType->addItem("Ochi-Hubble 6-param (6)", 6);
    connect(m_spectrumType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WaveSettingsDialog::onSpectrumTypeChanged);
    sg->addWidget(m_spectrumType, 0, 1);

    sg->addWidget(new QLabel("Sig. Wave Height Hs (m):"), 1, 0);
    m_sigHeight = new QDoubleSpinBox();
    m_sigHeight->setRange(0, 100); m_sigHeight->setDecimals(3); m_sigHeight->setValue(0);
    sg->addWidget(m_sigHeight, 1, 1);

    sg->addWidget(new QLabel("Peak Freq (rad/s):"), 1, 2);
    m_peakFreq = new QDoubleSpinBox();
    m_peakFreq->setRange(0, 100); m_peakFreq->setDecimals(4); m_peakFreq->setValue(0);
    sg->addWidget(m_peakFreq, 1, 3);

    sg->addWidget(new QLabel("Gamma (JONSWAP):"), 2, 0);
    m_gamma = new QDoubleSpinBox();
    m_gamma->setRange(1, 10); m_gamma->setDecimals(2); m_gamma->setValue(3.30);
    sg->addWidget(m_gamma, 2, 1);

    main->addWidget(m_spectrumGroup);


    m_sixParamGroup = new QGroupBox("Ochi-Hubble Six Parameters");
    m_sixParamGroup->setVisible(false);
    QGridLayout* sp = new QGridLayout(m_sixParamGroup);
    sp->setSpacing(8);

    sp->addWidget(new QLabel("Hsig1 (m):"), 0, 0);
    m_hsig1 = new QDoubleSpinBox();
    m_hsig1->setRange(0, 100); m_hsig1->setDecimals(3); m_hsig1->setValue(0);
    sp->addWidget(m_hsig1, 0, 1);

    sp->addWidget(new QLabel("Hsig2 (m):"), 0, 2);
    m_hsig2 = new QDoubleSpinBox();
    m_hsig2->setRange(0, 100); m_hsig2->setDecimals(3); m_hsig2->setValue(0);
    sp->addWidget(m_hsig2, 0, 3);

    sp->addWidget(new QLabel("PeakFreq1 (rad/s):"), 1, 0);
    m_peakFreq1 = new QDoubleSpinBox();
    m_peakFreq1->setRange(0, 100); m_peakFreq1->setDecimals(4); m_peakFreq1->setValue(0);
    sp->addWidget(m_peakFreq1, 1, 1);

    sp->addWidget(new QLabel("PeakFreq2 (rad/s):"), 1, 2);
    m_peakFreq2 = new QDoubleSpinBox();
    m_peakFreq2->setRange(0, 100); m_peakFreq2->setDecimals(4); m_peakFreq2->setValue(0);
    sp->addWidget(m_peakFreq2, 1, 3);

    sp->addWidget(new QLabel("Lambda1:"), 2, 0);
    m_lambda1 = new QDoubleSpinBox();
    m_lambda1->setRange(0, 100); m_lambda1->setDecimals(4); m_lambda1->setValue(0);
    sp->addWidget(m_lambda1, 2, 1);

    sp->addWidget(new QLabel("Lambda2:"), 2, 2);
    m_lambda2 = new QDoubleSpinBox();
    m_lambda2->setRange(0, 100); m_lambda2->setDecimals(4); m_lambda2->setValue(0);
    sp->addWidget(m_lambda2, 2, 3);

    main->addWidget(m_sixParamGroup);


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



void WaveSettingsDialog::onFreqModeChanged(int index)
{
    bool isList = (index == 0);  // mode 1 = list
    m_freqListLabel->setVisible(isList);
    m_freqList->setVisible(isList);
    m_startFreqLabel->setVisible(!isList);
    m_startFreq->setVisible(!isList);
    m_freqStepLabel->setVisible(!isList);
    m_freqStep->setVisible(!isList);
}

void WaveSettingsDialog::onDepthModeChanged(int index)
{
    bool isFinite = (index == 1);
    m_depthLabel->setVisible(isFinite);
    m_waterDepth->setVisible(isFinite);
}

void WaveSettingsDialog::onSpectrumToggled(bool on)
{
    m_spectrumGroup->setEnabled(on);
    // Show six-param group only if spectrum is on AND type is Ochi-Hubble
    bool isSixParam = on && (m_spectrumType->currentData().toInt() == 6);
    m_sixParamGroup->setVisible(isSixParam);
    m_sixParamGroup->setEnabled(isSixParam);
}

void WaveSettingsDialog::onSpectrumTypeChanged(int /*index*/)
{
    int type = m_spectrumType->currentData().toInt();
    bool isSixParam = (type == 6);
    bool isJonswap = (type == 2);

    // Two-param fields (Hs, peak freq, gamma) are used for types 1-5
    m_sigHeight->setEnabled(!isSixParam);
    m_peakFreq->setEnabled(!isSixParam);
    m_gamma->setEnabled(isJonswap);

    // Six-param group only for Ochi-Hubble
    m_sixParamGroup->setVisible(isSixParam && m_useSpectrum->isChecked());
    m_sixParamGroup->setEnabled(isSixParam);
}



std::vector<double> WaveSettingsDialog::parseList(const QString& text) const
{
    std::vector<double> vals;
    QStringList parts = text.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
    for (const auto& p : parts)
    {
        bool ok;
        double v = p.toDouble(&ok);
        if (ok) vals.push_back(v);
    }
    return vals;
}



WaveSettingsDialog::WaveData WaveSettingsDialog::getData() const
{
    WaveData d;
    d.waveAmplitude = m_amplitude->value();

    d.freqMode = m_freqMode->currentData().toInt();
    d.nFreq = m_nFreq->value();
    d.frequencies = parseList(m_freqList->toPlainText());
    d.startFreq = m_startFreq->value();
    d.freqStep = m_freqStep->value();

    d.headings = parseList(m_headingList->toPlainText());
    d.nHead = m_nHead->value();

    d.depthMode = m_depthMode->currentData().toInt();
    d.waterDepth = m_waterDepth->value();

    d.speeds = parseList(m_speedList->toPlainText());

    d.useSpectrum = m_useSpectrum->isChecked();
    d.spectrumType = m_spectrumType->currentData().toInt();
    d.sigWaveHeight = m_sigHeight->value();
    d.peakFreq = m_peakFreq->value();
    d.gammaJonswap = m_gamma->value();

    // Ochi-Hubble six parameters
    d.hsig1 = m_hsig1->value();
    d.hsig2 = m_hsig2->value();
    d.peakFreq1 = m_peakFreq1->value();
    d.peakFreq2 = m_peakFreq2->value();
    d.lambda1 = m_lambda1->value();
    d.lambda2 = m_lambda2->value();

    return d;
}

void WaveSettingsDialog::setData(const WaveData& d)
{
    m_amplitude->setValue(d.waveAmplitude);

    m_freqMode->setCurrentIndex(d.freqMode == 2 ? 1 : 0);
    onFreqModeChanged(d.freqMode == 2 ? 1 : 0);
    m_nFreq->setValue(d.nFreq);
    m_startFreq->setValue(d.startFreq);
    m_freqStep->setValue(d.freqStep);

    // Build frequency list string
    QString freqStr;
    for (int i = 0; i < (int)d.frequencies.size(); ++i)
    {
        if (i > 0) freqStr += ", ";
        freqStr += QString::number(d.frequencies[i]);
    }
    m_freqList->setText(freqStr);

    m_nHead->setValue(d.nHead);
    QString headStr;
    for (int i = 0; i < (int)d.headings.size(); ++i)
    {
        if (i > 0) headStr += ", ";
        headStr += QString::number(d.headings[i]);
    }
    m_headingList->setText(headStr);

    m_depthMode->setCurrentIndex(d.depthMode == 2 ? 1 : 0);
    onDepthModeChanged(d.depthMode == 2 ? 1 : 0);
    m_waterDepth->setValue(d.waterDepth);

    QString speedStr;
    for (int i = 0; i < (int)d.speeds.size(); ++i)
    {
        if (i > 0) speedStr += ", ";
        speedStr += QString::number(d.speeds[i]);
    }
    m_speedList->setText(speedStr);

    m_useSpectrum->setChecked(d.useSpectrum);
    m_spectrumType->setCurrentIndex(d.spectrumType - 1);
    onSpectrumTypeChanged(d.spectrumType - 1);
    m_sigHeight->setValue(d.sigWaveHeight);
    m_peakFreq->setValue(d.peakFreq);
    m_gamma->setValue(d.gammaJonswap);

    // Ochi-Hubble six parameters
    m_hsig1->setValue(d.hsig1);
    m_hsig2->setValue(d.hsig2);
    m_peakFreq1->setValue(d.peakFreq1);
    m_peakFreq2->setValue(d.peakFreq2);
    m_lambda1->setValue(d.lambda1);
    m_lambda2->setValue(d.lambda2);
}

