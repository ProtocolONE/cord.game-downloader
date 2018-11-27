#include "FakeCheckUpdateAdapter.h"
#include <QtCore/QTimer>

FakeCheckUpdateAdapter::FakeCheckUpdateAdapter(P1::GameDownloader::GameDownloadService *service)
  : _downloadService(service)
  , _isUpdate(false)
  , _updateTime(0)
{
  QObject::connect(
    service, SIGNAL(checkUpdateRequest(const P1::Core::Service *, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType)),
    this, SLOT(checkUpdateRequest(const P1::Core::Service *, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType))
    , Qt::QueuedConnection);

  QObject::connect(
    this, SIGNAL(checkUpdateRequestCompleted(const P1::Core::Service *, bool)),
    service, SLOT(checkUpdateRequestCompleted(const P1::Core::Service *, bool))
    , Qt::QueuedConnection);
}


FakeCheckUpdateAdapter::~FakeCheckUpdateAdapter()
{
}

void FakeCheckUpdateAdapter::sendResultSlot()
{
  emit this->checkUpdateRequestCompleted(this->_service, this->_isUpdate);
}

void FakeCheckUpdateAdapter::setPreHook(const std::function<bool(const P1::Core::Service*, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType)>& f)
{
  this->_preHook = f;
}

void FakeCheckUpdateAdapter::checkUpdateRequest(const P1::Core::Service *service, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType type)
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
