
#include "resultsviewer.h"
#include "theme.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPixmap>
#include <QIcon>
#include <QScrollBar>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QToolTip>
#include <qlegendmarker.h>
#include <utility>
#include <cmath>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>


class ZoomableChartView : public QChartView
{
public:
    ZoomableChartView(QChart* chart, QWidget* parent = nullptr)
        : QChartView(chart, parent)
    {
        setRubberBand(QChartView::RectangleRubberBand);
        setMouseTracking(true);
    }

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        chart()->zoomReset();
        QChartView::mouseDoubleClickEvent(event);
    }

    // value on hover
    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (!chart() || chart()->series().isEmpty())
        {
            QChartView::mouseMoveEvent(event);
            return;
        }

        QPointF scenePos = mapToScene(event->pos());
        QPointF chartPos = chart()->mapToValue(scenePos);
        QRectF plotArea = chart()->plotArea();
        if (plotArea.contains(event->pos()))
        {
            QString tip = QString("freq = %1 rad/s\nvalue = %2")
            .arg(chartPos.x(), 0, 'f', 4)
                .arg(chartPos.y(), 0, 'g', 6);
            QToolTip::showText(event->globalPosition().toPoint(), tip, this);
        }
        else
        {
            QToolTip::hideText();
        }

        QChartView::mouseMoveEvent(event);
    }
};

QColor ResultsViewer::modeColor(int i) const
{
    static const QColor c[] = {
        {255,80,80},{80,200,80},{80,140,255},{255,180,40},{180,80,220},{40,220,220}
    };
    return c[i % 6];
}

QColor ResultsViewer::datasetColor(int i) const
{
    static const QColor c[] = {
        {255, 80,  80},   // red
        {80,  200, 80},   // green
        {80,  140, 255},  // blue
        {255, 200, 40},   // yellow
        {200, 100, 255},  // purple
        {40,  220, 220},  // cyan
        {255, 130, 40},   // orange
        {255, 100, 200},  // pink
        {140, 230, 100},  // lime
        {100, 200, 255},  // sky blue
    };
    return c[i % 10];
}

QString ResultsViewer::modeName(int i) const
{
    static const QString n[] = {"Surge","Sway","Heave","Roll","Pitch","Yaw"};
    return n[i % 6];
}

QString ResultsViewer::modeUnit(bool isPhase, int i) const
{
    if (isPhase) return "deg";
    return (i < 3) ? "m/m" : "deg/m";
}

QPen ResultsViewer::datasetPen(int datasetIdx, const QColor& color, qreal width) const
{
    QPen pen(color, width);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    switch (datasetIdx % 5)
    {
    case 0:
        pen.setStyle(Qt::SolidLine);
        break;
    case 1:
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern({1, 4});
        break;
    case 2:
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern({10, 6});
        break;
    case 3:
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern({8, 4, 2, 4});
        break;
    case 4:
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern({4, 5});
        break;
    }

    return pen;
}

QString ResultsViewer::blockLabel(const QString& body, double speed, double heading) const
{
    QString name = body.trimmed().isEmpty() ? "unknown" : body.trimmed();
    return QString("%1 (V=%2 kn, \u03B2=%3\u00B0)")
        .arg(name).arg(speed, 0, 'f', 1).arg(heading, 0, 'f', 0);
}

ResultsViewer::ResultsViewer(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("MAPS Results Viewer");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    applyDarkStyle();
    setupUI();
}

void ResultsViewer::applyDarkStyle()
{
    setStyleSheet(darkDialogStyle());
}

static QWidget* buildForceTab(
    QListWidget*& datasets,
    QComboBox*& viewMode,
    QComboBox*& dataType,
    QWidget*& chartContainer,
    QVBoxLayout*& chartLayout,
    std::vector<QCheckBox*>& modeChecks,
    const QString& loadLabel,
    const QStringList& modeLabels,
    const std::function<QColor(int)>& colorFn,
    const std::function<void()>& onLoad,
    const std::function<void()>& onRemove,
    const std::function<void()>& onChanged)
{
    QWidget* w = new QWidget();
    QHBoxLayout* hl = new QHBoxLayout(w);
    hl->setSpacing(8);

    QVBoxLayout* lp = new QVBoxLayout();
    lp->setSpacing(6);

    QLabel* dsLbl = new QLabel("Loaded Datasets:");
    dsLbl->setStyleSheet("font-weight: bold;");
    lp->addWidget(dsLbl);

    datasets = new QListWidget();
    datasets->setMaximumWidth(280);
    datasets->setMinimumWidth(220);
    QObject::connect(datasets, &QListWidget::itemChanged, onChanged);
    lp->addWidget(datasets, 1);

    QHBoxLayout* btns = new QHBoxLayout();
    QPushButton* lb = new QPushButton(loadLabel);
    lb->setObjectName("secondaryBtn");
    QObject::connect(lb, &QPushButton::clicked, onLoad);
    btns->addWidget(lb);
    QPushButton* rb = new QPushButton("Remove");
    rb->setObjectName("removeBtn");
    QObject::connect(rb, &QPushButton::clicked, onRemove);
    btns->addWidget(rb);
    lp->addLayout(btns);

    lp->addSpacing(8);
    lp->addWidget(new QLabel("View:"));
    viewMode = new QComboBox();
    viewMode->addItem("Combined (all modes)");
    viewMode->addItem("Split (individual plots)");
    QObject::connect(viewMode, QOverload<int>::of(&QComboBox::currentIndexChanged), onChanged);
    lp->addWidget(viewMode);

    lp->addWidget(new QLabel("Data:"));
    dataType = new QComboBox();
    dataType->addItem("Amplitude");
    dataType->addItem("Phase Angle");
    QObject::connect(dataType, QOverload<int>::of(&QComboBox::currentIndexChanged), onChanged);
    lp->addWidget(dataType);

    lp->addSpacing(8);

    QGroupBox* mb = new QGroupBox("Visible Modes");
    QVBoxLayout* ml = new QVBoxLayout(mb);
    for (int i = 0; i < 6; ++i)
    {
        QCheckBox* cb = new QCheckBox(modeLabels[i]);
        cb->setChecked(true);
        cb->setStyleSheet(QString("QCheckBox { color: %1; }").arg(colorFn(i).name()));
        QObject::connect(cb, &QCheckBox::toggled, onChanged);
        ml->addWidget(cb);
        modeChecks.push_back(cb);
    }
    lp->addWidget(mb);

    hl->addLayout(lp);

    QScrollArea* sc = new QScrollArea();
    sc->setWidgetResizable(true);
    chartContainer = new QWidget();
    chartLayout = new QVBoxLayout(chartContainer);
    chartLayout->setSpacing(4);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    sc->setWidget(chartContainer);
    hl->addWidget(sc, 1);

    return w;
}

void ResultsViewer::setupUI()
{
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setSpacing(6);
    main->setContentsMargins(8, 8, 8, 8);

    QLabel* title = new QLabel("MAPS0 Results Viewer");
    title->setStyleSheet("font-size: 15px; font-weight: bold; color: #ffffff; padding: 2px;");
    main->addWidget(title);

    m_tabs = new QTabWidget();
    m_tabs->addTab(createMotionTab(), "Motion RAOs");
    m_tabs->addTab(createAddedMassTab(), "Added Mass");
    m_tabs->addTab(createDampingTab(), "Damping");
    m_tabs->addTab(createWaveForceTab(), "Wave Forces");
    main->addWidget(m_tabs);
}

QWidget* ResultsViewer::createMotionTab()
{
    QStringList labels;
    for (int i = 0; i < 6; ++i) labels << modeName(i);
    return buildForceTab(
        m_motionDatasets, m_motionViewMode, m_motionDataType,
        m_motionChartContainer, m_motionChartLayout, m_motionModeChecks,
        "Load .motion...", labels,
        [this](int i){ return modeColor(i); },
        [this](){ onLoadMotion(); },
        [this](){ onRemoveMotion(); },
        [this](){ onMotionSettingsChanged(); });
}

QWidget* ResultsViewer::createDampingTab()
{
    QStringList labels;
    for (int i = 0; i < 6; ++i) labels << modeName(i);
    return buildForceTab(
        m_dampDatasets, m_dampViewMode, m_dampDataType,
        m_dampChartContainer, m_dampChartLayout, m_dampModeChecks,
        "Load .damp...", labels,
        [this](int i){ return modeColor(i); },
        [this](){ onLoadDamping(); },
        [this](){ onRemoveDamping(); },
        [this](){ onDampSettingsChanged(); });
}

QWidget* ResultsViewer::createWaveForceTab()
{
    QStringList labels = {"Fx — Surge","Fy — Sway","Fz — Heave","Mx — Roll","My — Pitch","Mz — Yaw"};
    return buildForceTab(
        m_waveDatasets, m_waveViewMode, m_waveDataType,
        m_waveChartContainer, m_waveChartLayout, m_waveModeChecks,
        "Load .waveexf...", labels,
        [this](int i){ return modeColor(i); },
        [this](){ onLoadWaveForce(); },
        [this](){ onRemoveWaveForce(); },
        [this](){ onWaveSettingsChanged(); });
}


QWidget* ResultsViewer::createAddedMassTab()
{
    QWidget* w = new QWidget();
    QHBoxLayout* hl = new QHBoxLayout(w);
    hl->setSpacing(8);

    QVBoxLayout* lp = new QVBoxLayout();
    lp->setSpacing(6);

    QLabel* dsLbl = new QLabel("Loaded Datasets:");
    dsLbl->setStyleSheet("font-weight: bold;");
    lp->addWidget(dsLbl);

    m_admsDatasets = new QListWidget();
    m_admsDatasets->setMaximumWidth(280);
    m_admsDatasets->setMinimumWidth(220);
    connect(m_admsDatasets, &QListWidget::itemChanged,
            this, &ResultsViewer::onAdmsSettingsChanged);
    lp->addWidget(m_admsDatasets, 1);

    QHBoxLayout* btns = new QHBoxLayout();
    QPushButton* lb = new QPushButton("Load .adms...");
    lb->setObjectName("secondaryBtn");
    connect(lb, &QPushButton::clicked, this, &ResultsViewer::onLoadAddedMass);
    btns->addWidget(lb);
    QPushButton* rb = new QPushButton("Remove");
    rb->setObjectName("removeBtn");
    connect(rb, &QPushButton::clicked, this, &ResultsViewer::onRemoveAddedMass);
    btns->addWidget(rb);
    lp->addLayout(btns);

    lp->addSpacing(8);
    lp->addWidget(new QLabel("View:"));
    m_admsViewMode = new QComboBox();
    m_admsViewMode->addItem("Diagonal terms (A11..A66)");
    m_admsViewMode->addItem("All 6x6 terms");
    connect(m_admsViewMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ResultsViewer::onAdmsSettingsChanged);
    lp->addWidget(m_admsViewMode);

    lp->addSpacing(8);

    QGroupBox* mb = new QGroupBox("Visible Terms");
    QVBoxLayout* ml = new QVBoxLayout(mb);
    for (int i = 0; i < 6; ++i)
    {
        QCheckBox* cb = new QCheckBox(QString("A(%1,%1) — %2").arg(i+1).arg(modeName(i)));
        cb->setChecked(true);
        connect(cb, &QCheckBox::toggled, this, &ResultsViewer::onAdmsSettingsChanged);
        ml->addWidget(cb);
        m_admsModeChecks.push_back(cb);
    }
    lp->addWidget(mb);

    hl->addLayout(lp);

    QScrollArea* sc = new QScrollArea();
    sc->setWidgetResizable(true);
    m_admsChartContainer = new QWidget();
    m_admsChartLayout = new QVBoxLayout(m_admsChartContainer);
    m_admsChartLayout->setSpacing(4);
    m_admsChartLayout->setContentsMargins(0, 0, 0, 0);
    sc->setWidget(m_admsChartContainer);
    hl->addWidget(sc, 1);

    return w;
}

std::vector<double> ResultsViewer::parseDataLine(const QString& line) const
{
    std::vector<double> vals;
    const QStringList parts = line.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const auto& p : std::as_const(parts))
    {
        bool ok;
        double v = p.toDouble(&ok);
        if (ok) vals.push_back(v);
    }
    return vals;
}


std::vector<ResultsViewer::MotionBlock> ResultsViewer::parseMotionBlocks(const QString& content) const
{
    std::vector<MotionBlock> blocks;
    MotionBlock current;
    bool inBlock = true;  // start collecting immediately (some files lack Body name header)

    const QStringList lines = content.split('\n');
    for (const auto& line : std::as_const(lines))
    {
        QString trimmed = line.trimmed();

        if (trimmed.contains("Body name:") || trimmed.contains("body name"))
        {
            if (inBlock && !current.freq.empty())
                blocks.push_back(current);

            current = MotionBlock();
            int colonPos = trimmed.lastIndexOf(':');
            if (colonPos >= 0)
                current.bodyName = trimmed.mid(colonPos + 1).trimmed();
            inBlock = true;
            continue;
        }

        if (trimmed.contains("Wave heading"))
        {
            // New heading = new block (even without Body name)
            if (inBlock && !current.freq.empty())
            {
                blocks.push_back(current);
                QString prevName = current.bodyName;
                double prevSpeed = current.speed;
                current = MotionBlock();
                current.bodyName = prevName;
                current.speed = prevSpeed;
            }
            current.heading = trimmed.mid(trimmed.lastIndexOf(':') + 1).trimmed().toDouble();
            continue;
        }

        if (trimmed.contains("Speed") || trimmed.contains("speed"))
        {
            int lastColon = trimmed.lastIndexOf(':');
            current.speed = trimmed.mid(lastColon + 1).trimmed().toDouble();
            continue;
        }

        if (trimmed.isEmpty() || trimmed.startsWith('#') || trimmed.startsWith("due"))
            continue;


        auto vals = parseDataLine(trimmed);
        if (vals.size() >= 15)
        {
            current.freq.push_back(vals[1]);
            current.period.push_back(vals[2]);
            for (int m = 0; m < 6; ++m)
                current.rao[m].push_back(vals[3 + m]);
            for (int m = 0; m < 6; ++m)
                current.phase[m].push_back(vals[9 + m]);
        }
    }

    if (inBlock && !current.freq.empty())
        blocks.push_back(current);

    return blocks;
}


std::vector<ResultsViewer::MatrixBlock> ResultsViewer::parseMatrixBlocks(
    const QString& content, int numDataCols) const
{
    std::vector<MatrixBlock> blocks;
    MatrixBlock current;
    bool inBlock = true;

    const QStringList lines = content.split('\n');
    for (const auto& line : std::as_const(lines))
    {
        QString trimmed = line.trimmed();

        if (trimmed.contains("Body name:") || trimmed.contains("body name"))
        {
            if (inBlock && !current.freq.empty())
                blocks.push_back(current);

            current = MatrixBlock();
            int colonPos = trimmed.lastIndexOf(':');
            if (colonPos >= 0)
                current.bodyName = trimmed.mid(colonPos + 1).trimmed();
            inBlock = true;
            continue;
        }

        if (trimmed.contains("Wave heading"))
        {
            if (inBlock && !current.freq.empty())
            {
                blocks.push_back(current);
                QString prevName = current.bodyName;
                double prevSpeed = current.speed;
                current = MatrixBlock();
                current.bodyName = prevName;
                current.speed = prevSpeed;
            }
            current.heading = trimmed.mid(trimmed.lastIndexOf(':') + 1).trimmed().toDouble();
            continue;
        }

        if (trimmed.contains("Speed") || trimmed.contains("speed"))
        {
            int lastColon = trimmed.lastIndexOf(':');
            current.speed = trimmed.mid(lastColon + 1).trimmed().toDouble();
            continue;
        }

        if (trimmed.isEmpty() || trimmed.startsWith('#') || trimmed.startsWith("due"))
            continue;

        auto vals = parseDataLine(trimmed);
        if ((int)vals.size() >= 3 + numDataCols)
        {
            current.freq.push_back(vals[1]);
            current.period.push_back(vals[2]);
            std::vector<double> data(numDataCols);
            for (int k = 0; k < numDataCols; ++k)
                data[k] = vals[3 + k];
            current.matrix6x6.push_back(data);
        }
    }

    if (inBlock && !current.freq.empty())
        blocks.push_back(current);

    return blocks;
}

bool ResultsViewer::loadMotionFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QString content = file.readAll();
    file.close();
    auto blocks = parseMotionBlocks(content);
    if (blocks.empty()) return false;

    QString fallback = QFileInfo(path).baseName();
    for (auto& b : blocks)
        if (b.bodyName.trimmed().isEmpty()) b.bodyName = fallback;

    MotionFile mf; mf.filePath = path; mf.blocks = std::move(blocks);
    m_motionFiles.push_back(std::move(mf));
    refreshMotionDatasets();
    rebuildMotionCharts();
    return true;
}

bool ResultsViewer::loadAddedMassFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QString content = file.readAll();
    file.close();
    auto blocks = parseMatrixBlocks(content, 36);
    if (blocks.empty()) return false;

    QString fallback = QFileInfo(path).baseName();
    for (auto& b : blocks)
        if (b.bodyName.trimmed().isEmpty()) b.bodyName = fallback;

    MatrixFile mf; mf.filePath = path; mf.blocks = std::move(blocks);
    m_addedMassFiles.push_back(std::move(mf));
    refreshAddedMassDatasets();
    rebuildAddedMassCharts();
    return true;
}

bool ResultsViewer::loadDampingFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QString content = file.readAll();
    file.close();
    auto blocks = parseMotionBlocks(content);
    if (blocks.empty()) return false;

    QString fallback = QFileInfo(path).baseName();
    for (auto& b : blocks)
        if (b.bodyName.trimmed().isEmpty()) b.bodyName = fallback;

    MotionFile mf; mf.filePath = path; mf.blocks = std::move(blocks);
    m_dampingFiles.push_back(std::move(mf));
    refreshDampingDatasets();
    rebuildDampingCharts();
    return true;
}

bool ResultsViewer::loadWaveForceFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QString content = file.readAll();
    file.close();
    auto blocks = parseMotionBlocks(content);
    if (blocks.empty()) return false;

    QString fallback = QFileInfo(path).baseName();
    for (auto& b : blocks)
        if (b.bodyName.trimmed().isEmpty()) b.bodyName = fallback;

    MotionFile mf; mf.filePath = path; mf.blocks = std::move(blocks);
    m_waveForceFiles.push_back(std::move(mf));
    refreshWaveForceDatasets();
    rebuildWaveForceCharts();
    return true;
}

static void fillDatasetList(QListWidget* list,
                            const std::vector<ResultsViewer::MotionFile>& files,
                            const std::function<QString(const QString&, double, double)>& labelFn,
                            const std::function<QColor(int)>& colorFn)
{
    list->blockSignals(true);
    list->clear();
    int flatIdx = 0;
    for (int fi = 0; fi < (int)files.size(); ++fi)
        for (int bi = 0; bi < (int)files[fi].blocks.size(); ++bi)
        {
            const auto& b = files[fi].blocks[bi];
            QListWidgetItem* item = new QListWidgetItem(labelFn(b.bodyName, b.speed, b.heading));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            item->setData(Qt::UserRole, fi);
            item->setData(Qt::UserRole + 1, bi);
            item->setData(Qt::UserRole + 2, flatIdx);


            QPixmap px(12, 12);
            px.fill(colorFn(flatIdx));
            item->setIcon(QIcon(px));

            list->addItem(item);
            flatIdx++;
        }
    list->blockSignals(false);
}

void ResultsViewer::refreshMotionDatasets()
{
    fillDatasetList(m_motionDatasets, m_motionFiles,
                    [this](const QString& b, double s, double h){ return blockLabel(b, s, h); },
                    [this](int i){ return datasetColor(i); });
}

void ResultsViewer::refreshAddedMassDatasets()
{
    m_admsDatasets->blockSignals(true);
    m_admsDatasets->clear();
    int flatIdx = 0;
    for (int fi = 0; fi < (int)m_addedMassFiles.size(); ++fi)
        for (int bi = 0; bi < (int)m_addedMassFiles[fi].blocks.size(); ++bi)
        {
            const auto& b = m_addedMassFiles[fi].blocks[bi];
            QListWidgetItem* item = new QListWidgetItem(blockLabel(b.bodyName, b.speed, b.heading));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            item->setData(Qt::UserRole, fi);
            item->setData(Qt::UserRole + 1, bi);
            item->setData(Qt::UserRole + 2, flatIdx);

            QPixmap px(12, 12);
            px.fill(datasetColor(flatIdx));
            item->setIcon(QIcon(px));

            m_admsDatasets->addItem(item);
            flatIdx++;
        }
    m_admsDatasets->blockSignals(false);
}

void ResultsViewer::refreshDampingDatasets()
{
    fillDatasetList(m_dampDatasets, m_dampingFiles,
                    [this](const QString& b, double s, double h){ return blockLabel(b, s, h); },
                    [this](int i){ return datasetColor(i); });
}

void ResultsViewer::refreshWaveForceDatasets()
{
    fillDatasetList(m_waveDatasets, m_waveForceFiles,
                    [this](const QString& b, double s, double h){ return blockLabel(b, s, h); },
                    [this](int i){ return datasetColor(i); });
}

void ResultsViewer::styleChart(QChart* chart)
{
    chart->setBackgroundBrush(QBrush(QColor(30, 30, 30)));
    chart->setPlotAreaBackgroundBrush(QBrush(QColor(25, 25, 28)));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setTitleBrush(QBrush(QColor(220, 220, 220)));
    chart->setTitleFont(QFont("Segoe UI", 11, QFont::Bold));
    chart->legend()->setLabelColor(QColor(200, 200, 200));
    chart->legend()->setFont(QFont("Segoe UI", 9));
    chart->legend()->setBrush(QBrush(QColor(40, 40, 44)));
    chart->legend()->setPen(QPen(QColor(62, 62, 66)));
    chart->setMargins(QMargins(8, 8, 8, 8));
}

void ResultsViewer::styleAxis(QValueAxis* axis)
{
    axis->setLabelsColor(QColor(180, 180, 180));
    axis->setTitleBrush(QBrush(QColor(180, 180, 180)));
    axis->setGridLineColor(QColor(50, 50, 55));
    axis->setMinorGridLineColor(QColor(40, 40, 44));
    axis->setLinePenColor(QColor(80, 80, 85));
    axis->setLabelsFont(QFont("Segoe UI", 9));
    axis->setTitleFont(QFont("Segoe UI", 10));
    axis->setLabelFormat("%.3g");
}

void ResultsViewer::clearLayout(QVBoxLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->widget()) delete item->widget();
        delete item;
    }
}

void ResultsViewer::rebuildGenericCharts(
    const std::vector<MotionFile>& files,
    QListWidget* datasets,
    QComboBox* viewMode,
    QComboBox* dataType,
    QVBoxLayout* chartLayout,
    const std::vector<QCheckBox*>& modeChecks,
    const QString& title,
    const QStringList& modeLabels,
    const QStringList& unitLabels)
{
    clearLayout(chartLayout);

    bool showPhase = (dataType->currentIndex() == 1);
    bool splitView = (viewMode->currentIndex() == 1);

    struct Ref { int fi; int bi; int globalIdx; };
    std::vector<Ref> checked;
    for (int i = 0; i < datasets->count(); ++i)
    {
        auto* item = datasets->item(i);
        if (item->checkState() == Qt::Checked)
            checked.push_back({item->data(Qt::UserRole).toInt(),
                               item->data(Qt::UserRole + 1).toInt(),
                               item->data(Qt::UserRole + 2).toInt()});
    }
    if (checked.empty()) return;

    auto buildChart = [&](int mStart, int mEnd, const QString& chartTitle)
    {
        QChart* chart = new QChart();
        chart->setTitle(chartTitle);

        double yMin = 1e30, yMax = -1e30, xMin = 1e30, xMax = -1e30;

        for (int di = 0; di < (int)checked.size(); ++di)
        {
            int gIdx = checked[di].globalIdx;
            const auto& blk = files[checked[di].fi].blocks[checked[di].bi];
            QString prefix = (checked.size() > 1)
                                 ? blockLabel(blk.bodyName, blk.speed, blk.heading) + " \u2014 " : "";

            for (int m = mStart; m < mEnd; ++m)
            {
                if (!modeChecks[m]->isChecked()) continue;
                const auto& data = showPhase ? blk.phase[m] : blk.rao[m];

                QLineSeries* series = new QLineSeries();
                series->setName(prefix + modeLabels[m]);


                QColor lineColor = (checked.size() > 1) ? datasetColor(gIdx) : modeColor(m);
                series->setPen(datasetPen(gIdx, lineColor));

                for (int i = 0; i < (int)blk.freq.size() && i < (int)data.size(); ++i)
                {
                    series->append(blk.freq[i], data[i]);
                    yMin = std::min(yMin, data[i]);
                    yMax = std::max(yMax, data[i]);
                    xMin = std::min(xMin, blk.freq[i]);
                    xMax = std::max(xMax, blk.freq[i]);
                }
                chart->addSeries(series);
            }
        }

        QValueAxis* xAxis = new QValueAxis();
        xAxis->setTitleText("Frequency (rad/s)");
        if (xMin < xMax) xAxis->setRange(xMin, xMax);
        styleAxis(xAxis);
        chart->addAxis(xAxis, Qt::AlignBottom);

        QValueAxis* yAxis = new QValueAxis();
        yAxis->setTitleText(showPhase ? "Phase (deg)" : title);
        if (yMin < yMax) {
            double margin = (yMax - yMin) * 0.1;
            if (margin < 0.001) margin = 0.1;
            yAxis->setRange(yMin - margin, yMax + margin);
        }
        styleAxis(yAxis);
        chart->addAxis(yAxis, Qt::AlignLeft);

        for (auto* s : chart->series()) {
            s->attachAxis(xAxis); s->attachAxis(yAxis);
        }

        styleChart(chart);


        chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
        for (auto* marker : chart->legend()->markers())
        {
            if (auto* lm = qobject_cast<QLineSeries*>(marker->series()))
            {
                QPen legendPen = lm->pen();
                legendPen.setWidthF(3.0);
                marker->setPen(legendPen);
                marker->setBrush(Qt::NoBrush);
            }
        }

        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);

        ZoomableChartView* view = new ZoomableChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);
        view->setMinimumHeight(splitView ? 350 : 550);
        chartLayout->addWidget(view);
    };

    if (splitView)
    {
        for (int m = 0; m < NUM_MODES; ++m)
        {
            if (!modeChecks[m]->isChecked()) continue;
            QString u = (m < unitLabels.size()) ? unitLabels[m] : "";
            buildChart(m, m + 1,
                       QString("%1 %2 (%3)")
                           .arg(modeLabels[m])
                           .arg(showPhase ? "Phase" : "Amplitude")
                           .arg(showPhase ? "deg" : u));
        }
    }
    else
    {
        buildChart(0, 6, showPhase ? (title + " Phases") : title);
    }
}

void ResultsViewer::rebuildMotionCharts()
{
    QStringList labels, units;
    for (int i = 0; i < 6; ++i) { labels << modeName(i); units << modeUnit(false, i); }
    rebuildGenericCharts(m_motionFiles, m_motionDatasets, m_motionViewMode,
                         m_motionDataType, m_motionChartLayout, m_motionModeChecks,
                         "Motion RAO", labels, units);
}

void ResultsViewer::rebuildDampingCharts()
{
    QStringList labels, units;
    for (int i = 0; i < 6; ++i) { labels << modeName(i); units << (i < 3 ? "KN/m" : "KNm/m"); }
    rebuildGenericCharts(m_dampingFiles, m_dampDatasets, m_dampViewMode,
                         m_dampDataType, m_dampChartLayout, m_dampModeChecks,
                         "Damping Force", labels, units);
}

void ResultsViewer::rebuildWaveForceCharts()
{
    QStringList labels = {"Fx","Fy","Fz","Mx","My","Mz"};
    QStringList units = {"KN/m","KN/m","KN/m","KNm/m","KNm/m","KNm/m"};
    rebuildGenericCharts(m_waveForceFiles, m_waveDatasets, m_waveViewMode,
                         m_waveDataType, m_waveChartLayout, m_waveModeChecks,
                         "Wave Exciting Force", labels, units);
}

void ResultsViewer::rebuildAddedMassCharts()
{
    clearLayout(m_admsChartLayout);

    bool showAll = (m_admsViewMode->currentIndex() == 1);

    struct Ref { int fi; int bi; int globalIdx; };
    std::vector<Ref> checked;
    for (int i = 0; i < m_admsDatasets->count(); ++i)
    {
        auto* item = m_admsDatasets->item(i);
        if (item->checkState() == Qt::Checked)
            checked.push_back({item->data(Qt::UserRole).toInt(),
                               item->data(Qt::UserRole + 1).toInt(),
                               item->data(Qt::UserRole + 2).toInt()});
    }
    if (checked.empty()) return;

    std::vector<std::pair<int,int>> terms;
    if (showAll)
        for (int r = 0; r < 6; ++r) for (int c = 0; c < 6; ++c) terms.push_back({r, c});
    else
        for (int i = 0; i < 6; ++i)
            if (m_admsModeChecks[i]->isChecked()) terms.push_back({i, i});

    for (const auto& [row, col] : terms)
    {
        QChart* chart = new QChart();
        chart->setTitle(QString("A(%1,%2) \u2014 %3/%4")
                            .arg(row+1).arg(col+1).arg(modeName(row)).arg(modeName(col)));

        double yMin = 1e30, yMax = -1e30, xMin = 1e30, xMax = -1e30;

        for (int di = 0; di < (int)checked.size(); ++di)
        {
            int gIdx = checked[di].globalIdx;
            const auto& blk = m_addedMassFiles[checked[di].fi].blocks[checked[di].bi];
            QLineSeries* series = new QLineSeries();
            series->setName(blockLabel(blk.bodyName, blk.speed, blk.heading));
            series->setPen(datasetPen(gIdx, datasetColor(gIdx)));

            int idx = row * 6 + col;
            for (int i = 0; i < (int)blk.freq.size(); ++i)
            {
                if (idx < (int)blk.matrix6x6[i].size())
                {
                    double val = blk.matrix6x6[i][idx] / 1000.0;  // kg to tonnes
                    series->append(blk.freq[i], val);
                    yMin = std::min(yMin, val); yMax = std::max(yMax, val);
                    xMin = std::min(xMin, blk.freq[i]); xMax = std::max(xMax, blk.freq[i]);
                }
            }
            chart->addSeries(series);
        }

        QValueAxis* xAxis = new QValueAxis();
        xAxis->setTitleText("Frequency (rad/s)");
        if (xMin < xMax) xAxis->setRange(xMin, xMax);
        styleAxis(xAxis);
        chart->addAxis(xAxis, Qt::AlignBottom);

        QValueAxis* yAxis = new QValueAxis();
        QString unit = (col < 3) ? (row < 3 ? "t" : "t·m") : (row < 3 ? "t·m" : "t·m²");
        yAxis->setTitleText(QString("Added Mass (%1)").arg(unit));
        if (yMin < yMax) {
            double margin = (yMax - yMin) * 0.1;
            if (margin < 1e-6) margin = std::abs(yMax) * 0.1;
            yAxis->setRange(yMin - margin, yMax + margin);
        }
        styleAxis(yAxis);
        chart->addAxis(yAxis, Qt::AlignLeft);

        for (auto* s : chart->series()) { s->attachAxis(xAxis); s->attachAxis(yAxis); }
        styleChart(chart);

        chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
        for (auto* marker : chart->legend()->markers())
        {
            if (auto* lm = qobject_cast<QLineSeries*>(marker->series()))
            {
                QPen legendPen = lm->pen();
                legendPen.setWidthF(3.0);
                marker->setPen(legendPen);
                marker->setBrush(Qt::NoBrush);
            }
        }

        chart->legend()->setVisible(checked.size() > 1);

        ZoomableChartView* view = new ZoomableChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);
        view->setMinimumHeight(showAll ? 280 : 380);
        m_admsChartLayout->addWidget(view);
    }
}

void ResultsViewer::onLoadMotion()
{
    QString p = QFileDialog::getOpenFileName(this, "Open Motion File", "",
                                             "Motion Files (*.motion);;All Files (*.*)");
    if (!p.isEmpty()) loadMotionFile(p);
}
void ResultsViewer::onLoadAddedMass()
{
    QString p = QFileDialog::getOpenFileName(this, "Open Added Mass File", "",
                                             "Added Mass Files (*.adms);;All Files (*.*)");
    if (!p.isEmpty()) loadAddedMassFile(p);
}
void ResultsViewer::onLoadDamping()
{
    QString p = QFileDialog::getOpenFileName(this, "Open Damping File", "",
                                             "Damping Files (*.damp);;All Files (*.*)");
    if (!p.isEmpty()) loadDampingFile(p);
}
void ResultsViewer::onLoadWaveForce()
{
    QString p = QFileDialog::getOpenFileName(this, "Open Wave Force File", "",
                                             "Wave Force Files (*.waveexf);;All Files (*.*)");
    if (!p.isEmpty()) loadWaveForceFile(p);
}

void ResultsViewer::onRemoveMotion()
{
    int row = m_motionDatasets->currentRow();
    if (row < 0) return;
    int fi = m_motionDatasets->item(row)->data(Qt::UserRole).toInt();
    if (fi >= 0 && fi < (int)m_motionFiles.size())
        m_motionFiles.erase(m_motionFiles.begin() + fi);
    refreshMotionDatasets(); rebuildMotionCharts();
}
void ResultsViewer::onRemoveAddedMass()
{
    int row = m_admsDatasets->currentRow();
    if (row < 0) return;
    int fi = m_admsDatasets->item(row)->data(Qt::UserRole).toInt();
    if (fi >= 0 && fi < (int)m_addedMassFiles.size())
        m_addedMassFiles.erase(m_addedMassFiles.begin() + fi);
    refreshAddedMassDatasets(); rebuildAddedMassCharts();
}
void ResultsViewer::onRemoveDamping()
{
    int row = m_dampDatasets->currentRow();
    if (row < 0) return;
    int fi = m_dampDatasets->item(row)->data(Qt::UserRole).toInt();
    if (fi >= 0 && fi < (int)m_dampingFiles.size())
        m_dampingFiles.erase(m_dampingFiles.begin() + fi);
    refreshDampingDatasets(); rebuildDampingCharts();
}
void ResultsViewer::onRemoveWaveForce()
{
    int row = m_waveDatasets->currentRow();
    if (row < 0) return;
    int fi = m_waveDatasets->item(row)->data(Qt::UserRole).toInt();
    if (fi >= 0 && fi < (int)m_waveForceFiles.size())
        m_waveForceFiles.erase(m_waveForceFiles.begin() + fi);
    refreshWaveForceDatasets(); rebuildWaveForceCharts();
}

void ResultsViewer::onMotionSettingsChanged() { rebuildMotionCharts(); }
void ResultsViewer::onAdmsSettingsChanged() { rebuildAddedMassCharts(); }
void ResultsViewer::onDampSettingsChanged() { rebuildDampingCharts(); }
void ResultsViewer::onWaveSettingsChanged() { rebuildWaveForceCharts(); }
