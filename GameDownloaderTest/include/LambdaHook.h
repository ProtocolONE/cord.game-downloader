#ifndef _GGS_GAMEDOWNLOADER_TEST_LAMBDAHOOK_H_
#define _GGS_GAMEDOWNLOADER_TEST_LAMBDAHOOK_H_

#include <Core/Service>
#include <GameDownloader/HookBase.h>
#include <functional>

#define DOWNLOADERHOOK(x) std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int, const GGS::Core::Service *)> x = [&](int hookId, const GGS::Core::Service *service) mutable -> GGS::GameDownloader::HookBase::HookResult

/*!
  \class LambdaHook
  \brief ����������� ��� ��� GameDownloaderService. �� ���� ��������� ������������� ���� � ��� ��������, 
  ���������� � �����. ������������� ������������ ��� �������� ������������ ���������� ����� � ������. 
  \author Ilya.Tkachenko
  \date 18.04.2012
*/
class LambdaHook : public GGS::GameDownloader::HookBase
{
public:
  LambdaHook(int id, std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int id, const GGS::Core::Service *)> pre,
    std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int id, const GGS::Core::Service *)>post);
  ~LambdaHook();

  virtual HookResult beforeDownload( const GGS::Core::Service *service );
  virtual HookResult afterDownload( const GGS::Core::Service *service );

private:
  int _id;
  std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int id, const GGS::Core::Service *)> preHook;
  std::tr1::function<GGS::GameDownloader::HookBase::HookResult (int id, const GGS::Core::Service *)> postHook;
};

#endif // _GGS_GAMEDOWNLOADER_TEST_LAMBDAHOOK_H_