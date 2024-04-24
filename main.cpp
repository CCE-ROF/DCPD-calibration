#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QApplication::tr("FectorForce Calibration"));
    a.setOrganizationName("VectorForce Technologies GmbH & Co. KG");
    a.setApplicationVersion("1.0");

    MainWindow w;
    w.show();
    return a.exec();
}
