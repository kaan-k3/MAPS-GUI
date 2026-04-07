#pragma once

#include <QString>
#include <QObject>
#include <QProcess>

#include "runconfigdialog.h"
#include "principaldatadialog.h"
#include "wavesettingsdialog.h"

class ProjectAssembler : public QObject
{
    Q_OBJECT

public:
    explicit ProjectAssembler(QObject* parent = nullptr);

    // Set all the data from the various dialogs
    void setProjectName(const QString& name);
    void setRunControls(const RunConfigDialog::RunControls& rc);
    void setBodies(const std::vector<RunConfigDialog::BodyDef>& bodies);
    void setRestrainedModes(const std::vector<bool>& modes);
    void setPrincipalData(const PrincipalDataDialog::PrincipalData& pd);
    void setWaveData(const WaveSettingsDialog::WaveData& wd);
    void setOptionalFiles(const RunConfigDialog::OptionalFiles& files);
    void setWallData(const RunConfigDialog::WallData& wd);


    QString generateProjectFile() const;

    bool writeProjectFile(const QString& path) const;

    void runMAPS(const QString& mapsExecutable, const QString& projectPath);
    void stopMAPS();
    bool isRunning() const;

signals:
    void mapsOutput(const QString& line);
    void mapsFinished(int exitCode, const QString& summary);
    void mapsError(const QString& error);

private:
    QString m_projectName = "project";

    RunConfigDialog::RunControls m_runControls;
    std::vector<RunConfigDialog::BodyDef> m_bodies;
    std::vector<bool> m_restrainedModes = {false, false, false, false, false, false};
    PrincipalDataDialog::PrincipalData m_principalData;
    WaveSettingsDialog::WaveData m_waveData;
    RunConfigDialog::OptionalFiles m_optFiles;
    RunConfigDialog::WallData m_wallData;

    QProcess* m_process = nullptr;

    QString designationBlock() const;
    QString runControlsBlock() const;
    QString waterDepthBlock() const;
    QString bodiesBlock() const;
    QString restrainedModesBlock() const;
    QString principalDataBlock() const;
    QString speedsBlock() const;
    QString wavesBlock() const;
    QString spectrumBlock() const;
    QString wallDataBlock() const;

private slots:
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
};

