#pragma once
#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <vector>

class WaveSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaveSettingsDialog(QWidget* parent = nullptr);

    struct WaveData {
        double waveAmplitude = 1.0;  // meters

        // Frequency input
        int freqMode = 1;           // 1 = list, 2 = start+increment
        std::vector<double> frequencies;  // rad/s (mode 1)
        double startFreq = 0.1;     // rad/s (mode 2)
        double freqStep = 0.1;      // rad/s (mode 2)
        int nFreq = 10;

        // Headings
        std::vector<double> headings;  // degrees
        int nHead = 1;

        // Water depth
        int depthMode = 1;         // 1 = deep, 2 = finite
        double waterDepth = 1000.0;  // meters

        // Speeds
        std::vector<double> speeds;  // knots

        // Wave spectrum (optional)
        bool useSpectrum = false;
        int spectrumType = 1;       // 1=PM, 2=JONSWAP, 3=ISSC, 4=ITTC, 5=Bretschneider, 6=Ochi-Hubble
        double sigWaveHeight = 0.0;
        double peakFreq = 0.0;
        double gammaJonswap = 3.3;

        // Ochi-Hubble six-parameter spectrum (spectrumType=6)
        double hsig1 = 0.0;
        double hsig2 = 0.0;
        double peakFreq1 = 0.0;
        double peakFreq2 = 0.0;
        double lambda1 = 0.0;
        double lambda2 = 0.0;
    };

    WaveData getData() const;
    void setData(const WaveData& d);

private:
    // Amplitude
    QDoubleSpinBox* m_amplitude = nullptr;

    // Frequency
    QComboBox* m_freqMode = nullptr;
    QSpinBox*  m_nFreq = nullptr;
    QTextEdit* m_freqList = nullptr;
    QDoubleSpinBox* m_startFreq = nullptr;
    QDoubleSpinBox* m_freqStep = nullptr;
    QLabel* m_freqListLabel = nullptr;
    QLabel* m_startFreqLabel = nullptr;
    QLabel* m_freqStepLabel = nullptr;

    // Headings
    QSpinBox*  m_nHead = nullptr;
    QTextEdit* m_headingList = nullptr;

    // Water depth
    QComboBox*      m_depthMode = nullptr;
    QDoubleSpinBox* m_waterDepth = nullptr;
    QLabel*         m_depthLabel = nullptr;

    // Speeds
    QTextEdit* m_speedList = nullptr;

    // Spectrum
    QCheckBox*      m_useSpectrum = nullptr;
    QGroupBox*      m_spectrumGroup = nullptr;
    QComboBox*      m_spectrumType = nullptr;
    QDoubleSpinBox* m_sigHeight = nullptr;
    QDoubleSpinBox* m_peakFreq = nullptr;
    QDoubleSpinBox* m_gamma = nullptr;

    // Ochi-Hubble six-parameter (shown when type=6)
    QGroupBox*      m_sixParamGroup = nullptr;
    QDoubleSpinBox* m_hsig1 = nullptr;
    QDoubleSpinBox* m_hsig2 = nullptr;
    QDoubleSpinBox* m_peakFreq1 = nullptr;
    QDoubleSpinBox* m_peakFreq2 = nullptr;
    QDoubleSpinBox* m_lambda1 = nullptr;
    QDoubleSpinBox* m_lambda2 = nullptr;

    QPushButton* m_okBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;

    void setupUI();
    void applyDarkStyle();

    std::vector<double> parseList(const QString& text) const;

private slots:
    void onFreqModeChanged(int index);
    void onDepthModeChanged(int index);
    void onSpectrumToggled(bool on);
    void onSpectrumTypeChanged(int index);
};

