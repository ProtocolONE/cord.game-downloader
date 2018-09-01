#pragma once

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileInfoList>

class FileUtils
{
public:
  static bool removeDir(const QString &dirName);

  static QString createDirectoryIfNotExist(const QString &targetFilePath);
  static void recurseCopyDiretory(const QString& source, const QString& target);
  static void prepairWorkSpace(const QString& testCaseName, QString& testName);
  static QString workspaceRoot(const QString& testCaseName, QString& testName);
};

#define PREPAIR_WORK_SPACE(x,y) FileUtils::prepairWorkSpace(QString(#x), QString(#y));
#define WORKSPACE_ROOT(x,y) FileUtils::workspaceRoot(QString(#x), QString(#y));
