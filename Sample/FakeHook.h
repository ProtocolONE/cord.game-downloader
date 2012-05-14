#ifndef _GGS_GAMEDOWNLOADER_SAMPLE_FAKEHOOK_H_
#define _GGS_GAMEDOWNLOADER_SAMPLE_FAKEHOOK_H_

#include <GameDownloader/HookBase.h>

#include <QtCore/QString>
#include <QtCore/QObject>

class FakeHook : public GGS::GameDownloader::HookBase
{
public:
  FakeHook(const QString& id, QObject *parent = 0);
  ~FakeHook();

  virtual GGS::GameDownloader::HookBase::HookResult beforeDownload(const GGS::Core::Service *service);

  virtual GGS::GameDownloader::HookBase::HookResult afterDownload(const GGS::Core::Service *service);

};

#endif // _GGS_GAMEDOWNLOADER_SAMPLE_FAKEHOOK_H_