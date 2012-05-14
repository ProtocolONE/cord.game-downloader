#ifndef _GGS_GAMEDOWNLOADER_TEST_FAKELIBTORRENTWRAPPERADAPTER_H_
#define _GGS_GAMEDOWNLOADER_TEST_FAKELIBTORRENTWRAPPERADAPTER_H_

#include "SingleShotFunctor.h"

#include <GameDownloader/GameDownloadService.h>
#include <Core/Service.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

class FakeLibtorrentWrapperAdapter : public QObject
{
  Q_OBJECT
public:
  FakeLibtorrentWrapperAdapter(GGS::GameDownloader::GameDownloadService *service);
  ~FakeLibtorrentWrapperAdapter();

  int downloadEventCount() const;
  int pauseEventCount() const;

  void setDownloadTime(int msec);

private slots:
  void downloadRequestCounter(const GGS::Core::Service *service);
  void pauseRequestCounter(const GGS::Core::Service *service);
  void downloadCompletedTimerTick();

private:
  int _downloadEventCount;
  int _pauseEventCount;

  int _downloadTimeInMs;

  bool _downloadInProgress;
  GGS::GameDownloader::GameDownloadService *_downloadService;
  const GGS::Core::Service *_service;

  QTimer _timer;
  QMutex _mutex;
};

#endif // _GGS_GAMEDOWNLOADER_TEST_FAKELIBTORRENTWRAPPERADAPTER_H_