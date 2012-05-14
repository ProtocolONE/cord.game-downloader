#include "FakeCheckUpdateAdapter.h"
#include <QtCore/QTimer>

FakeCheckUpdateAdapter::FakeCheckUpdateAdapter(GGS::GameDownloader::GameDownloadService *service)
  : _downloadService(service)
  , _preHook(0)
  , _isUpdate(false)
  , _updateTime(0)
{
  QObject::connect(
    service, SIGNAL(checkUpdateRequest(const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)),
    this, SLOT(checkUpdateRequest(const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType))
    , Qt::QueuedConnection);

  QObject::connect(
    this, SIGNAL(checkUpdateRequestCompleted(const GGS::Core::Service *, bool)),
    service, SLOT(checkUpdateRequestCompleted(const GGS::Core::Service *, bool))
    , Qt::QueuedConnection);
}


FakeCheckUpdateAdapter::~FakeCheckUpdateAdapter()
{
}

void FakeCheckUpdateAdapter::sendResultSlot()
{
  emit this->checkUpdateRequestCompleted(this->_service, this->_isUpdate);
}

void FakeCheckUpdateAdapter::setPreHook(std::tr1::function<bool (const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)> f)
{
  this->_preHook = f;
}

void FakeCheckUpdateAdapter::checkUpdateRequest(const GGS::Core::Service *service, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType type)
{
  this->_service = service;
  if (this->_preHook)
    this->_isUpdate = this->_preHook(service, type);

  QTimer::singleShot(this->_updateTime, this, SLOT(sendResultSlot()));
}

void FakeCheckUpdateAdapter::setUpdateTime(int updateTime)
{
  this->_updateTime = updateTime;
}
