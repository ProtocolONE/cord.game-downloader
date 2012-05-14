#ifndef _GGS_LIBTORRENTWRAPPERTEST_FILEUTRILS_H_
#define _GGS_LIBTORRENTWRAPPERTEST_FILEUTRILS_H_

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileInfoList>

class FileUtils
{
public:
  static bool removeDir(const QString &dirName);
};

#endif // _GGS_LIBTORRENTWRAPPERTEST_FILEUTRILS_H_