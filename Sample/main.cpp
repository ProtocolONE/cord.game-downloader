#include "Program.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <Windows.h>

#include <BugTrap\BugTrap.h>


void initBugTrap()
{
  BT_SetAppName(L"DownloadSampler");
  BT_SetSupportEMail(L"support@gamenet.ru");
  BT_SetFlags(BTF_DETAILEDMODE | BTF_ATTACHREPORT | BTF_SCREENCAPTURE);
  BT_SetSupportServer(L"fs1.gamenet.ru", 9999);

  BT_InstallSehFilter();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    initBugTrap();

    system("chcp 866");
    Program p;
    return a.exec();
}
