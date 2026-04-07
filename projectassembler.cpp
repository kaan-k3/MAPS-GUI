
#include "projectassembler.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <algorithm>

ProjectAssembler::ProjectAssembler(QObject* parent)
    : QObject(parent)
{
}


void ProjectAssembler::setProjectName(const QString& name) { m_projectName = name; }
void ProjectAssembler::setRunControls(const RunConfigDialog::RunControls& rc) { m_runControls = rc; }
void ProjectAssembler::setBodies(const std::vector<RunConfigDialog::BodyDef>& bodies) { m_bodies = bodies; }
void ProjectAssembler::setRestrainedModes(const std::vector<bool>& modes) { m_restrainedModes = modes; }
void ProjectAssembler::setPrincipalData(const PrincipalDataDialog::PrincipalData& pd) { m_principalData = pd; }
void ProjectAssembler::setWaveData(const WaveSettingsDialog::WaveData& wd) { m_waveData = wd; }
void ProjectAssembler::setOptionalFiles(const RunConfigDialog::OptionalFiles& files) { m_optFiles = files; }
void ProjectAssembler::setWallData(const RunConfigDialog::WallData& wd) { m_wallData = wd; }

QString ProjectAssembler::designationBlock() const
{
    QString block;
    QTextStream out(&block);
    out << "&designation\n";
    out << "projectname='" << m_projectName << "'\n";
    out << "date='" << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "'\n";
    out << "username='GUI_MAPS'\n";
    out << "/endnames\n";
    return block;
}

QString ProjectAssembler::runControlsBlock() const
{
    const auto& rc = m_runControls;
    QString block;
    QTextStream out(&block);

    out << "&runcontrols\n";

    // Hardcoded flags
    out << "kgeometry=1\n";
    out << "isolve=1\n";
    out << "koutput=1\n";
    out << "knurbformat=2\n";
    out << "kirf=0\n";
    out << "kiterative=0\n";
    out << "kdriftoutput=0\n";
    out << "kinterpolate=1\n";

    // GUI-configured flags
    out << "kfreq=" << m_waveData.freqMode << "\n";
    out << "kmterm=" << rc.kmterm << "\n";
    out << "kirregular=" << rc.kirregular << "\n";
    out << "ktrim=" << rc.ktrim << "\n";
    out << "trimprecision=" << rc.trimPrec << "\n";
    out << "kdrift=" << rc.kdrift << "\n";
    out << "kbodymotion=" << rc.kbodymotion << "\n";
    out << "kforwardspeed=" << rc.kforwardspeed << "\n";
    out << "kcatamaran=" << rc.kcatamaran << "\n";
    out << "kfsintegral=" << rc.kfsintegral << "\n";
    out << "kload=" << rc.kload << "\n";
    out << "kdamp=" << rc.kdamp << "\n";
    out << "kpoi=" << (m_optFiles.poiFile.isEmpty() ? 0 : 1) << "\n";
    out << "krandomsea=" << rc.krandomsea << "\n";
    out << "kexternalrestoring=" << (m_optFiles.exRestoreFile.isEmpty() ? 0 : 1) << "\n";
    out << "kexternaldamping=" << (m_optFiles.exDampFile.isEmpty() ? 0 : 1) << "\n";
    out << "kfieldelevation=" << (m_optFiles.fieldFile.isEmpty() ? 0 : 1) << "\n";
    out << "kwalleffect=" << rc.kwalleffect << "\n";
    out << "knurbssmooth=" << rc.knurbssmooth << "\n";

    out << "/endruncontrols\n";

    return block;
}

QString ProjectAssembler::waterDepthBlock() const
{
    QString block;
    QTextStream out(&block);
    out << "&waterdepth\n";
    out << "kdepth=" << m_waveData.depthMode << "\n";
    out << "wdepth=" << m_waveData.waterDepth << "\n";
    out << "/endwaterdepth\n";
    return block;
}

QString ProjectAssembler::bodiesBlock() const
{
    QString block;
    QTextStream out(&block);

    out << "&multiplebodys\n";
    out << "numbody=" << m_bodies.size() << "\n";

    // Body names
    for (int i = 0; i < (int)m_bodies.size(); ++i)
    {
        if (i == 0)
            out << "bodyname='" << m_bodies[i].name << "'";
        else
            out << ",'" << m_bodies[i].name << "'";
    }
    out << "\n";

    // X positions
    out << "bodyx=";
    for (int i = 0; i < (int)m_bodies.size(); ++i)
    {
        if (i > 0) out << ",";
        out << m_bodies[i].x;
    }
    out << "\n";

    // Y positions
    out << "bodyy=";
    for (int i = 0; i < (int)m_bodies.size(); ++i)
    {
        if (i > 0) out << ",";
        out << m_bodies[i].y;
    }
    out << "\n";

    // Heading angles
    out << "bodyangle=";
    for (int i = 0; i < (int)m_bodies.size(); ++i)
    {
        if (i > 0) out << ",";
        out << m_bodies[i].angle;
    }
    out << "\n";

    out << "/endmultiplebodys\n";
    return block;
}

QString ProjectAssembler::restrainedModesBlock() const
{
    QString block;
    QTextStream out(&block);
    out << "&restrainmodes\n";
    out << "resmodes=";
    for (int i = 0; i < 6; ++i)
    {
        if (i > 0) out << ",";
        if (i < (int)m_restrainedModes.size())
            out << (m_restrainedModes[i] ? 1 : 0);
        else
            out << "0";
    }
    out << "\n";
    out << "/endrestrainmodes\n";
    return block;
}

QString ProjectAssembler::principalDataBlock() const
{
    const auto& d = m_principalData;
    QString block;
    QTextStream out(&block);

    out << "&principaldata\n";
    out << "sl=" << d.length << "\n";
    out << "sb=" << d.beam << "\n";
    out << "st=" << d.draft << "\n";
    out << "cb=" << d.blockCoeff << "\n";
    out << "vol=" << d.displacement << "\n";
    out << "xg=" << d.xg << "\n";
    out << "yg=" << d.yg << "\n";
    out << "zg=" << d.zg << "\n";
    out << "rxx=" << d.rxx << "\n";
    out << "ryy=" << d.ryy << "\n";
    out << "rzz=" << d.rzz << "\n";
    out << "rxy=" << d.rxy << "\n";
    out << "rxz=" << d.rxz << "\n";
    out << "ryx=" << d.ryx << "\n";
    out << "ryz=" << d.ryz << "\n";
    out << "rzx=" << d.rzx << "\n";
    out << "rzy=" << d.rzy << "\n";
    out << "dampv44=" << d.dampv44 << "\n";
    out << "gmt=" << d.gmt << "\n";
    out << "/endprincipaldata\n";

    return block;
}

QString ProjectAssembler::speedsBlock() const
{
    QString block;
    QTextStream out(&block);
    out << "&bodyspeeds\n";
    out << "nspeed=" << m_waveData.speeds.size() << "\n";
    out << "speeds=";
    for (int i = 0; i < (int)m_waveData.speeds.size(); ++i)
    {
        if (i > 0) out << ",";
        out << m_waveData.speeds[i];
    }
    out << "\n";
    out << "/endspeeds\n";
    return block;
}

QString ProjectAssembler::wavesBlock() const
{
    QString block;
    QTextStream out(&block);

    out << "&incidentwaves\n";
    out << "waveamp=" << m_waveData.waveAmplitude << "\n";
    out << "nhead=" << m_waveData.headings.size() << "\n";

    out << "headings=";
    for (int i = 0; i < (int)m_waveData.headings.size(); ++i)
    {
        if (i > 0) out << ",";
        out << m_waveData.headings[i];
    }
    out << "\n";

    if (m_waveData.freqMode == 1)
    {
        // Mode 1: explicit list
        out << "nfreq=" << m_waveData.frequencies.size() << "\n";
        out << "freqs=";
        for (int i = 0; i < (int)m_waveData.frequencies.size(); ++i)
        {
            if (i > 0) out << ",";
            out << m_waveData.frequencies[i];
        }
        out << "\n";
    }
    else
    {
        // Mode 2: start + increment
        out << "nfreq=" << m_waveData.nFreq << "\n";
        out << "freqs=" << m_waveData.startFreq << "," << m_waveData.freqStep << "\n";
    }

    out << "/endincidents\n";
    return block;
}

QString ProjectAssembler::spectrumBlock() const
{
    if (!m_waveData.useSpectrum) return "";

    QString block;
    QTextStream out(&block);

    out << "&wavespectrum\n";
    out << "spectrumtype=" << m_waveData.spectrumType << "\n";
    out << "numseastate=1\n";
    out << "/endwavespectrum\n";

    if (m_waveData.spectrumType == 6)
    {
        // Ochi-Hubble six-parameter spectrum
        out << "\n";
        out << "&sixparawavespectrum\n";
        out << "Hsig1=" << m_waveData.hsig1 << "\n";
        out << "Hsig2=" << m_waveData.hsig2 << "\n";
        out << "Peakfreq1=" << m_waveData.peakFreq1 << "\n";
        out << "Peakfreq2=" << m_waveData.peakFreq2 << "\n";
        out << "Lambda1=" << m_waveData.lambda1 << "\n";
        out << "Lambda2=" << m_waveData.lambda2 << "\n";
        out << "/endsixparawavespectrum\n";
    }
    else
    {
        // Two-parameter spectrum — types 1-5
        out << "\n";
        out << "&twoparawavespectrum\n";
        out << "Hsig=" << m_waveData.sigWaveHeight << "\n";
        out << "Peakfreq=" << m_waveData.peakFreq << "\n";
        out << "gammajonswap=" << m_waveData.gammaJonswap << "\n";
        out << "/endtwoparawavespectrum\n";
    }

    return block;
}

QString ProjectAssembler::wallDataBlock() const
{
    if (m_runControls.kwalleffect == 0) return "";

    QString block;
    QTextStream out(&block);

    out << "&walldata\n";
    out << "numwall=" << m_wallData.numwall << "\n";
    out << "numimage=" << m_wallData.numimage << "\n";

    // Distance to wall 1 for each body
    out << "distancewall1=";
    if (m_wallData.distancewall1.empty())
    {
        for (int i = 0; i < (int)m_bodies.size(); ++i)
        {
            if (i > 0) out << ",";
            out << "0";
        }
    }
    else
    {
        for (int i = 0; i < (int)m_bodies.size(); ++i)
        {
            if (i > 0) out << ",";
            int idx = std::min(i, (int)m_wallData.distancewall1.size() - 1);
            out << m_wallData.distancewall1[idx];
        }
    }
    out << "\n";

    // Distance to wall 2 (only if numwall=2)
    if (m_wallData.numwall == 2)
    {
        out << "distancewall2=";
        if (m_wallData.distancewall2.empty())
        {
            for (int i = 0; i < (int)m_bodies.size(); ++i)
            {
                if (i > 0) out << ",";
                out << "0";
            }
        }
        else
        {
            for (int i = 0; i < (int)m_bodies.size(); ++i)
            {
                if (i > 0) out << ",";
                int idx = std::min(i, (int)m_wallData.distancewall2.size() - 1);
                out << m_wallData.distancewall2[idx];
            }
        }
        out << "\n";
    }

    out << "/endwalldata\n";
    return block;
}


QString ProjectAssembler::generateProjectFile() const
{
    QString content;

    content += designationBlock();
    content += "\n";
    content += runControlsBlock();
    content += "\n";
    content += waterDepthBlock();
    content += "\n";
    content += bodiesBlock();
    content += "\n";
    content += restrainedModesBlock();
    content += "\n";
    content += principalDataBlock();
    content += "\n";
    content += speedsBlock();
    content += "\n";
    content += wavesBlock();

    QString spec = spectrumBlock();
    if (!spec.isEmpty())
    {
        content += "\n";
        content += spec;
    }

    QString wall = wallDataBlock();
    if (!wall.isEmpty())
    {
        content += "\n";
        content += wall;
    }

    return content;
}

bool ProjectAssembler::writeProjectFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << generateProjectFile();
    file.close();

    qDebug() << "Project file written to:" << path;
    return true;
}


void ProjectAssembler::runMAPS(const QString& mapsExecutable, const QString& projectPath)
{
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        emit mapsError("MAPS is already running.");
        return;
    }

    if (m_process)
    {
        delete m_process;
        m_process = nullptr;
    }

    // Project path without extension for MAPS command line
    QFileInfo fi(projectPath);
    QString projectName = fi.absolutePath() + "/" + fi.baseName();

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(fi.absolutePath());

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ProjectAssembler::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &ProjectAssembler::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProjectAssembler::onProcessFinished);

    qDebug() << "Starting MAPS:" << mapsExecutable << fi.baseName();
    m_process->start(mapsExecutable, {fi.baseName()});

    if (!m_process->waitForStarted(5000))
    {
        emit mapsError("Failed to start MAPS executable: " + mapsExecutable +
                       "\nError: " + m_process->errorString());
    }
}

void ProjectAssembler::stopMAPS()
{
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

bool ProjectAssembler::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void ProjectAssembler::onProcessOutput()
{
    while (m_process->canReadLine())
    {
        QString line = QString::fromLocal8Bit(m_process->readLine()).trimmed();
        if (!line.isEmpty())
            emit mapsOutput(line);
    }
}

void ProjectAssembler::onProcessError()
{
    QString err = QString::fromLocal8Bit(m_process->readAllStandardError()).trimmed();
    if (!err.isEmpty())
        emit mapsOutput("[STDERR] " + err);
}

void ProjectAssembler::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    // Read any remaining output
    QString remaining = QString::fromLocal8Bit(m_process->readAllStandardOutput()).trimmed();
    if (!remaining.isEmpty())
        emit mapsOutput(remaining);

    QString summary;
    if (status == QProcess::CrashExit)
        summary = "MAPS crashed.";
    else if (exitCode != 0)
        summary = QString("MAPS exited with code %1.").arg(exitCode);
    else
        summary = "MAPS completed successfully.";

    emit mapsFinished(exitCode, summary);
}
