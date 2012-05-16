#include "LambdaHook.h"
#include <GameDownloader/GameDownloadService>


LambdaHook::LambdaHook(int id, 
  std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int id, const GGS::Core::Service *)> pre,
  std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int id, const GGS::Core::Service *)>post)
  : HookBase(QString("hookId_%1").arg(id))
  , _id(id)
  , preHook(pre)
  , postHook(post)
{
}


LambdaHook::~LambdaHook()
{
}

GGS::GameDownloader::HookBase::HookResult LambdaHook::beforeDownload(GGS::GameDownloader::GameDownloadService *gameDownloader, const GGS::Core::Service *service)
{
  return this->preHook(this->_id, service);
}

GGS::GameDownloader::HookBase::HookResult LambdaHook::afterDownload(GGS::GameDownloader::GameDownloadService *gameDownloader, const GGS::Core::Service *service)
{
  return this->postHook(this->_id, service);
}
