#include "LambdaHook.h"
#include <GameDownloader/GameDownloadService.h>


LambdaHook::LambdaHook(int id, 
  std::function<P1::GameDownloader::HookBase::HookResult (int id, const P1::Core::Service *)> pre,
  std::function<P1::GameDownloader::HookBase::HookResult (int id, const P1::Core::Service *)>post)
  : HookBase(QString("hookId_%1").arg(id))
  , _id(id)
  , preHook(pre)
  , postHook(post)
{
}


LambdaHook::~LambdaHook()
{
}

P1::GameDownloader::HookBase::HookResult LambdaHook::beforeDownload(P1::GameDownloader::GameDownloadService *gameDownloader, const P1::Core::Service *service)
{
  return this->preHook(this->_id, service);
}

P1::GameDownloader::HookBase::HookResult LambdaHook::afterDownload(P1::GameDownloader::GameDownloadService *gameDownloader, const P1::Core::Service *service)
{
  return this->postHook(this->_id, service);
}
