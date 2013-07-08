#include "SimpleContinueHook.h"
#include <GameDownloader/GameDownloadService>
#include <Core/Service.h>

#include <QtCore/QDebug>

SimpleContinueHook::SimpleContinueHook(int hookId, QList<int> *preList, QList<int> *postList)
  : HookBase(QString("hookId_%1").arg(hookId))
  , _hookId(hookId)
  , _preList(preList)
  , _postList(postList)
  , _beforeCallCount(0)
  , _afterCallCount(0)
{
}

SimpleContinueHook::~SimpleContinueHook()
{
}

HookBase::HookResult SimpleContinueHook::beforeDownload(GameDownloadService *, ServiceState *state)
{
  const GGS::Core::Service *service = state->service();

  this->_beforeCallCount++;
  this->_preList->append(this->_hookId);
  qDebug() << "beforeDownload " << (service == 0 ? "service is null" : service->id());
  return HookBase::Continue;
}

HookBase::HookResult SimpleContinueHook::afterDownload(GameDownloadService *, ServiceState *state)
{
  const GGS::Core::Service *service = state->service();

  this->_afterCallCount++;
  this->_postList->append(this->_hookId);
  qDebug() << "afterDownload " << (service == 0 ? "service is null" : service->id());
  return HookBase::Continue;
}
