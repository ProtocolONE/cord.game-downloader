#include "FileUtils.h"

#include <QtCore/QDirIterator>
#include <QtCore/QCoreApplication>

/*!
   Delete a directory along with all of its contents.
 
   \param dirName Path of directory to remove.
   \return true on success; false on error.
*/
bool FileUtils::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);
 
    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }
 
            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
 
    return result;
}

QString FileUtils::createDirectoryIfNotExist(const QString &targetFilePath)
{
  QFileInfo info(targetFilePath);
  QDir targetPath = info.dir();
  QString path = info.absolutePath();
  if (!targetPath.exists())
    targetPath.mkpath(path);

  return path;
}

void FileUtils::recurseCopyDiretory(const QString& source, const QString& target)
{
  int len = source.length();
  QDir dir(source);

  QDirIterator it(dir, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    QString file = it.next();
    if (!it.fileInfo().isDir()) 
    {
      QString name = file.right(file.length() - len);
      QString targetFile = QString("%1/%2").arg(target, name);
      FileUtils::createDirectoryIfNotExist(targetFile);
      QFile::copy(file, targetFile);
    }
  }
}

void FileUtils::prepairWorkSpace(const QString& testCaseName, QString& testName)
{
  QString fixturePath = QString("%1/fixtures/%2/%3/").arg(QCoreApplication::applicationDirPath(), testCaseName, testName);
  QString workspaceRoot = FileUtils::workspaceRoot(testCaseName, testName);
  FileUtils::removeDir(workspaceRoot);
  FileUtils::createDirectoryIfNotExist(workspaceRoot);
  FileUtils::recurseCopyDiretory(fixturePath, workspaceRoot);
}

QString FileUtils::workspaceRoot(const QString& testCaseName, QString& testName)
{
  return QString("%1/workspace/%2/%3/").arg(QCoreApplication::applicationDirPath(), testCaseName, testName);
}
