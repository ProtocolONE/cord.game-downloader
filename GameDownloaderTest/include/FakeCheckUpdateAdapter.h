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
  FakeCheckUpdateAdapter(GGS::GameDownloader::GameDownloadService *service);
  ~FakeCheckUpdateAdapter();

  void setPreHook(std::tr1::function<bool (const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)> f);
  void setUpdateTime(int updateTime);

public slots:
  void checkUpdateRequest(const GGS::Core::Service *service, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType type);

signals:
  void checkUpdateRequestCompleted(const GGS::Core::Service *service, bool isUpdated);

private slots:
  void sendResultSlot();


private:
  int _updateTime;
  const GGS::Core::Service *_service;
  bool _isUpdate;
  std::tr1::function<bool (const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)> _preHook;
  GGS::GameDownloader::GameDownloadService *_downloadService;

};
