#pragma once

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
  explicit FakeLibtorrentWrapperAdapter(P1::GameDownloader::GameDownloadService *service);
  virtual ~FakeLibtorrentWrapperAdapter();

  int downloadEventCount() const;
  int pauseEventCount() const;

  void setDownloadTime(int msec);

private slots:
  void downloadRequestCounter(const P1::Core::Service *service);
  void pauseRequestCounter(const P1::Core::Service *service);
  void downloadCompletedTimerTick();

private:
  int _downloadEventCount;
  int _pauseEventCount;

  int _downloadTimeInMs;

  bool _downloadInProgress;
  P1::GameDownloader::GameDownloadService *_downloadService;
  const P1::Core::Service *_service;

  QTimer _timer;
  QMutex _mutex;
};
