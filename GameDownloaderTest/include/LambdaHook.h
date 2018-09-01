#pragma once

#include <Core/Service.h>
#include <GameDownloader/HookBase.h>
#include <functional>

#define DOWNLOADERHOOK(x) std::function<P1::GameDownloader::HookBase::HookResult (int, const P1::Core::Service *)> x = [&](int hookId, const P1::Core::Service *service) mutable -> P1::GameDownloader::HookBase::HookResult


namespace P1 {
  namespace GameDownloader {
    class GameDownloadService;
  }
}

/*!
  \class LambdaHook
  \brief Реализуется хук для GameDownloaderService. На вход принимает идентификатор хука и два функтора, 
  вызываемых в фуках. Идентифиактор используется для удобство рапознавания однотипных хуков в тестах. 
  \author Ilya.Tkachenko
  \date 18.04.2012
*/
class LambdaHook : public P1::GameDownloader::HookBase
{
public:
  LambdaHook(int id, std::function<P1::GameDownloader::HookBase::HookResult (int id, const P1::Core::Service *)> pre,
    std::function<P1::GameDownloader::HookBase::HookResult (int id, const P1::Core::Service *)>post);
  ~LambdaHook();

  virtual HookResult beforeDownload(P1::GameDownloader::GameDownloadService *gameDownloader, const P1::Core::Service *service );
  virtual HookResult afterDownload(P1::GameDownloader::GameDownloadService *gameDownloader, const P1::Core::Service *service );

private:
  int _id;
  std::function<P1::GameDownloader::HookBase::HookResult (int id, const P1::Core::Service *)> preHook;
  std::function<P1::GameDownloader::HookBase::HookResult (int id, const P1::Core::Service *)> postHook;
};
