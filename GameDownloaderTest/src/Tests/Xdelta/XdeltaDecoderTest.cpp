#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>
#include <UpdateSystem/Hasher/Md5FileHasher.h>

#include <FileUtils.h>
#include <ExternalToolHelper.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <gtest/gtest.h>
#include <Windows.h>

#define SHOW_GENERATION_PROGRESS 1

using namespace GGS::GameDownloader::XdeltaWrapper;

#define assertFileEqual(f1, f2) { \
GGS::Hasher::Md5FileHasher hasher; \
  ASSERT_EQ(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

#define assertFileUnEqual(f1, f2) { \
  GGS::Hasher::Md5FileHasher hasher; \
  ASSERT_NE(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

void showProgress(int current, int max)
{
#if SHOW_GENERATION_PROGRESS == 1 
  if (max <= 50) {
    qDebug() << "Progress " << int(100.0f * float(current) / float(max)) << "%";
    return;
  }

  if (current % (max / 20) == 0)
    qDebug() << "Progress " << int(100.0f * float(current) / float(max)) << "%";
#endif
}


TEST(XdeltaDecoder, SyncTest)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, SyncTest);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, SyncTest);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  XdeltaDecoder decoder;
  decoder.applySync(source, patch, actualTarget);
  assertFileEqual(actualTarget, expectedTarget);
}

TEST(XdeltaDecoder, BigFileGenerated)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, BigFileGenerated);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, BigFileGenerated);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString decodedActualTarget = QString("%1%2").arg(root, "decodedActualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  QFile sourceFile(source);
  QFile expectedFile(expectedTarget);
  sourceFile.open(QFile::ReadWrite);
  expectedFile.open(QFile::ReadWrite);
  
  QByteArray b1;
  QByteArray b2;

  for (int i = 0; i < 1000; ++i) {
    b1.append("0123456789");
    b2.append("0123456789");
    b2.append(QString::number(qrand()));
  }

  int size = 2000;
  for (int i = 0; i < size; i++) {
    showProgress(i, size);
    sourceFile.write(b1);
    expectedFile.write(b2);
  }

  sourceFile.close();
  expectedFile.close();

  QString xdeltaPath = QString("%1/fixtures/xdelta3.exe").arg(QCoreApplication::applicationDirPath());

  QStringList arg;
  arg << "-f" << "-9" << "-e" << "-s" << source << expectedTarget << patch;
  
  ExternalTool::startAndWait(xdeltaPath, arg, 300);
  DEBUG_LOG << "Big file generation patch finished.";
  
  XdeltaDecoder decoder;
  DWORD s1 = GetTickCount();
  decoder.applySync(source, patch, actualTarget);
  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";

  assertFileEqual(actualTarget, expectedTarget);

  arg.clear();
  arg << "-f" << "-d" << "-s" << source << patch << decodedActualTarget;

  DWORD s2 = GetTickCount();
  ExternalTool::startAndWait(xdeltaPath, arg, 300);
  int dif2 = GetTickCount() - s2;

  DEBUG_LOG << "Xdelta original time " << dif1 << "ms";
  assertFileEqual(decodedActualTarget, expectedTarget);
}

TEST(XdeltaDecoder, FromEmptySource)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, FromEmptySource);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, FromEmptySource);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  QFile expectedFile(expectedTarget);
  expectedFile.open(QFile::ReadWrite);

  QByteArray b2;

  for (int i = 0; i < 1000; ++i)  
    b2.append("0123456789");

  int size = 2000;
  for (int i = 0; i < size; i++) {
    showProgress(i, size);
    expectedFile.write(b2);
  }

  expectedFile.close();

  QString xdeltaPath = QString("%1/fixtures/xdelta3.exe").arg(QCoreApplication::applicationDirPath());
  QStringList arg;
  arg << "-f" << "-9" << "-e" << "-s" << "NUL" << expectedTarget << patch;
  ExternalTool::startAndWait(xdeltaPath, arg, 300);
  DEBUG_LOG << "Big file generation patch finished.";

  XdeltaDecoder decoder;
  DWORD s1 = GetTickCount();
  decoder.applySync(source, patch, actualTarget);
  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";
  assertFileEqual(actualTarget, expectedTarget);
}

TEST(XdeltaDecoder, ToEmptyFile)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, ToEmptyFile);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, ToEmptyFile);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  QFile sourceFile(source);
  sourceFile.open(QFile::ReadWrite);
  QFile expectedFile(expectedTarget);
  expectedFile.open(QFile::ReadWrite);
  expectedFile.close();
  QByteArray b1;

  for (int i = 0; i < 1000; ++i)
    b1.append("0123456789");

  int size = 2000;
  for (int i = 0; i < size; i++) {
    showProgress(i, size);
    sourceFile.write(b1);
  }

  sourceFile.close();

  QString xdeltaPath = QString("%1/fixtures/xdelta3.exe").arg(QCoreApplication::applicationDirPath());

  QStringList arg;
  arg << "-f" << "-9" << "-e" << "-s" << source << "NUL" << patch;
  
  ExternalTool::startAndWait(xdeltaPath, arg, 300);
  DEBUG_LOG << "Big file generation patch finished.";

  XdeltaDecoder decoder;
  DWORD s1 = GetTickCount();
  decoder.applySync(source, patch, actualTarget);
  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";
  assertFileEqual(actualTarget, expectedTarget);
}


TEST(XdeltaDecoder, WrongCrcTest)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, WrongCrcTest);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, WrongCrcTest);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");
  QString brokenSource = QString("%1%2").arg(root, "brokenSource.txt");

  QFile sourceFile(source);
  QFile brokenSourceFile(brokenSource);
  QFile expectedFile(expectedTarget);
  sourceFile.open(QFile::ReadWrite);
  expectedFile.open(QFile::ReadWrite);
  brokenSourceFile.open(QFile::ReadWrite);

  QByteArray b1;
  QByteArray b2;
  QByteArray b3;

  for (int i = 0; i < 1000; ++i) {
    b1.append("0123456789");
    b2.append("0123456789");
    b2.append(QString::number(qrand()));

    for (int j = 0; j < 1000; ++j)
      b3.append(QString::number(qrand() % 10));
  }

  int size = 2000;
  for (int i = 0; i < size; i++) {
    showProgress(i, size);
    sourceFile.write(b1);

    if (i % 5 == 0) {
      brokenSourceFile.write(b3);
    } else {
      brokenSourceFile.write(b1);
      brokenSourceFile.write("qqqq234 234234 234ertw ertr2 34234 23 01234567890w ty wryrwty");
    }

    expectedFile.write(b2);
  }

  sourceFile.close();
  expectedFile.close();
  brokenSourceFile.close();

  QString xdeltaPath = QString("%1/fixtures/xdelta3.exe").arg(QCoreApplication::applicationDirPath());

  QStringList arg;
  arg << "-f" << "-9" << "-e" << "-s" << source << expectedTarget << patch;
  
  ExternalTool::startAndWait(xdeltaPath, arg, 300);

  XdeltaDecoder decoder;
  DWORD s1 = GetTickCount();
  decoder.applySync(brokenSource, patch, actualTarget);
  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";

  assertFileUnEqual(actualTarget, expectedTarget);
}
