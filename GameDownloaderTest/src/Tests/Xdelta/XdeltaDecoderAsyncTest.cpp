#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>
#include <UpdateSystem/Hasher/Md5FileHasher.h>

#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <ExternalToolHelper.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QEventLoop>

#include <Qttest/QSignalSpy>

#include <gtest/gtest.h>
#include <Windows.h>

using namespace P1::GameDownloader::XdeltaWrapper;

#define assertFileEqual(f1, f2) { \
  P1::Hasher::Md5FileHasher hasher; \
  ASSERT_EQ(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

#define assertFileUnEqual(f1, f2) { \
  P1::Hasher::Md5FileHasher hasher; \
  ASSERT_NE(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

// defined in XdeltaDecoderTest.cpp
extern void showProgress(int current, int max);

TEST(XdeltaDecoder, AsyncTest)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, AsyncTest);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, AsyncTest);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  XdeltaDecoder decoder;
  decoder.apply(source, patch, actualTarget);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  QSignalSpy finishSpy(&decoder, SIGNAL(finished()));
  QSignalSpy failSpy(&decoder, SIGNAL(failed()));
  QSignalSpy crcFailSpy(&decoder, SIGNAL(crcFailed()));

  ASSERT_TRUE(killer.setTerminateSignal(&decoder, SIGNAL(finished())));
  ASSERT_TRUE(killer.setTerminateSignal(&decoder, SIGNAL(failed())));
  ASSERT_TRUE(killer.setTerminateSignal(&decoder, SIGNAL(crcFailed())));

  loop.exec();

  ASSERT_EQ(1, finishSpy.count());
  ASSERT_EQ(0, failSpy.count());
  ASSERT_EQ(0, crcFailSpy.count());

  assertFileEqual(actualTarget, expectedTarget);
}

TEST(XdeltaDecoder, AsyncBigFileGenerated)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, AsyncBigFileGenerated);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, AsyncBigFileGenerated);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString decodedActualTarget = QString("%1%2").arg(root, "decodedActualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  QFile sourceFile(source);
  QFile expectedFile(expectedTarget);
  ASSERT_TRUE(sourceFile.open(QFile::ReadWrite));
  ASSERT_TRUE(expectedFile.open(QFile::ReadWrite));

  QByteArray b1;
  QByteArray b2;

  for (int j = 0; j < 1000; ++j)
    for (int i = 0; i < 1000; ++i) {
      b1.append("0123456789");
      b2.append("0123456789");
      b2.append(QString::number(qrand()));
    }

  int size = 2;
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

  DWORD s1 = GetTickCount();

  XdeltaDecoder decoder;
  decoder.apply(source, patch, actualTarget);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(finished()), &killer, SLOT(terminateLoop())));
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(failed()), &killer, SLOT(terminateLoop())));
  loop.exec();

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

TEST(XdeltaDecoder, AsyncFromEmptySource)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, AsyncFromEmptySource);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, AsyncFromEmptySource);

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
  decoder.apply(source, patch, actualTarget);
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(finished()), &killer, SLOT(terminateLoop())));
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(failed()), &killer, SLOT(terminateLoop())));
  loop.exec();

  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";
  assertFileEqual(actualTarget, expectedTarget);
}

TEST(XdeltaDecoder, AsyncToEmptyFile)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, AsyncToEmptyFile);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, AsyncToEmptyFile);

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
  decoder.apply(source, patch, actualTarget);
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(finished()), &killer, SLOT(terminateLoop())));
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(failed()), &killer, SLOT(terminateLoop())));
  loop.exec();

  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";
  assertFileEqual(actualTarget, expectedTarget);
}


TEST(XdeltaDecoder, AsyncWrongCrcTest)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, AsyncWrongCrcTest);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, AsyncWrongCrcTest);

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
  decoder.apply(brokenSource, patch, actualTarget);
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(finished()), &killer, SLOT(terminateLoop())));
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(failed()), &killer, SLOT(terminateLoop())));
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(crcFailed()), &killer, SLOT(terminateLoop())));

  QSignalSpy spy(&decoder, SIGNAL(crcFailed()));
  QSignalSpy spy2(&decoder, SIGNAL(failed()));

  loop.exec();

  int dif1 = GetTickCount() - s1;
  DEBUG_LOG << "Xdelta wrapper time " << dif1 << "ms";

  ASSERT_EQ(1, spy.count());
  ASSERT_EQ(0, spy2.count());
  assertFileUnEqual(actualTarget, expectedTarget);
}

TEST(XdeltaDecoder, AsyncFullRandom)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, AsyncFullRandom);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, AsyncFullRandom);

  QString source = QString("%1%2").arg(root, "sourceFile.txt");
  QString patch = QString("%1%2").arg(root, "patch.dif");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString decodedActualTarget = QString("%1%2").arg(root, "decodedActualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "expectedTarget.txt");

  QFile sourceFile(source);
  QFile expectedFile(expectedTarget);
  ASSERT_TRUE(sourceFile.open(QFile::ReadWrite));
  ASSERT_TRUE(expectedFile.open(QFile::ReadWrite));

  QByteArray b1;
  QByteArray b2;

  for (int j = 0; j < 1000; ++j) {
    for (int i = 0; i < 1000; ++i) {
      b1.append((char)(qrand() % 255));
      b2.append((char)(qrand() % 255));
    }
  }

  int size = 20;
  for (int i = 0; i < size; i++) {
    showProgress(i, size);
    sourceFile.write(b1);
    expectedFile.write(b1);
  }

  for (int i = 0; i < size; i++) {
    showProgress(i, size);
    expectedFile.write(b2);
  }

  sourceFile.close();
  expectedFile.close();

  QString xdeltaPath = QString("%1/fixtures/xdelta3.exe").arg(QCoreApplication::applicationDirPath());

  QStringList arg;
  arg << "-f" << "-9" << "-e" << "-s" << source << expectedTarget << patch;

  ExternalTool::startAndWait(xdeltaPath, arg, 300);
  DEBUG_LOG << "Big file generation patch finished.";

  DWORD s1 = GetTickCount();

  XdeltaDecoder decoder;
  decoder.apply(source, patch, actualTarget);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(finished()), &killer, SLOT(terminateLoop())));
  ASSERT_TRUE(QObject::connect(&decoder, SIGNAL(failed()), &killer, SLOT(terminateLoop())));
  loop.exec();

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


TEST(XdeltaDecoder, FailTest)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, FailTest);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, FailTest);

  QString source("NUL");
  QString patch = QString("%1%2").arg(root, "ATTACH_T.rez.7z.diff");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "ATTACH_T.rez.7z");

  XdeltaDecoder decoder;
  decoder.applySync(source, patch, actualTarget);
  assertFileEqual(actualTarget, expectedTarget);
}

TEST(XdeltaDecoder, FailTestAsync)
{
  PREPAIR_WORK_SPACE(XdeltaDecoder, FailTest);
  QString root = WORKSPACE_ROOT(XdeltaDecoder, FailTest);

  QString source("NUL");
  QString patch = QString("%1%2").arg(root, "ATTACH_T.rez.7z.diff");
  QString actualTarget = QString("%1%2").arg(root, "actualTarget.txt");
  QString expectedTarget = QString("%1%2").arg(root, "ATTACH_T.rez.7z");

  XdeltaDecoder decoder;
  decoder.apply(source, patch, actualTarget);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);

  QSignalSpy finishSpy(&decoder, SIGNAL(finished()));
  QSignalSpy failSpy(&decoder, SIGNAL(failed()));
  QSignalSpy crcFailSpy(&decoder, SIGNAL(crcFailed()));

  ASSERT_TRUE(killer.setTerminateSignal(&decoder, SIGNAL(finished())));
  ASSERT_TRUE(killer.setTerminateSignal(&decoder, SIGNAL(failed())));
  ASSERT_TRUE(killer.setTerminateSignal(&decoder, SIGNAL(crcFailed())));

  loop.exec();
  ASSERT_EQ(1, finishSpy.count());
  ASSERT_EQ(0, failSpy.count());
  ASSERT_EQ(0, crcFailSpy.count());
  assertFileEqual(actualTarget, expectedTarget);
}
