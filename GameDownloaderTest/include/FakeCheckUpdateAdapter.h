#pragma once

#include <Core/Service.h>

#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/GameDownloadService.h>

#include <QtCore/QObject>
#include <functional>

class FakeCheckUpdateAdapter : public QObject
{
  Q_OBJECT
public:
  FakeCheckUpdateAdapter(P1::GameDownloader::GameDownloadService *service);
  ~FakeCheckUpdateAdapter();

  void setPreHook(std::tr1::function<bool (const P1::Core::Service *, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType)> f);
  void setUpdateTime(int updateTime);

public slots:
  void checkUpdateRequest(const P1::Core::Service *service, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType type);

signals:
  void checkUpdateRequestCompleted(const P1::Core::Service *service, bool isUpdated);

private slots:
  void sendResultSlot();


private:
  int _updateTime;
  const P1::Core::Service *_service;
  bool _isUpdate;
  std::tr1::function<bool (const P1::Core::Service *, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType)> _preHook;
  P1::GameDownloader::GameDownloadService *_downloadService;

};
