#include "mainwindow.h"
#include <TopoDS_Shape.hxx>


#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Initialize the QT Application

    MainWindow w; // Create and show the main window
    w.show();

    return a.exec(); // Enter Event Loop for QT
}
