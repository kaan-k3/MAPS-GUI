#include "mainwindow.h"

#include <QApplication>
#include <QMetaType>
#include <QVector3D>
#include <vector>
#include <utility>

using PairIntInt = std::pair<int,int>;

Q_DECLARE_METATYPE(std::vector<QVector3D>)
Q_DECLARE_METATYPE(std::vector<std::vector<QVector3D>>)
Q_DECLARE_METATYPE(std::vector<PairIntInt>)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<std::vector<QVector3D>>();
    qRegisterMetaType<std::vector<std::vector<QVector3D>>>();
    qRegisterMetaType<std::vector<PairIntInt>>();

    MainWindow w;
    w.show();
    return a.exec();
}
