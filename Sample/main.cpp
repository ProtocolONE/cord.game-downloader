#include "Program.h"
#include <QtCore/QCoreApplication>
#include <QtGui/QApplication>
#include <Windows.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    system("chcp 866");
    Program p;
    return a.exec();
}
