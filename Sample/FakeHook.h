#ifndef _GGS_GAMEDOWNLOADER_SAMPLE_FAKEHOOK_H_
#define _GGS_GAMEDOWNLOADER_SAMPLE_FAKEHOOK_H_

#include <GameDownloader/HookBase.h>

#include <QtCore/QString>
#include <QtCore/QObject>

using GGS::GameDownloader::HookBase;
using GGS::GameDownloader::ServiceState;
using GGS::GameDownloader::GameDownloadService;

class FakeHook : public GGS::GameDownloader::HookBase
{
public:
  FakeHook(const QString& id, QObject *parent = 0);
  ~FakeHook();

  virtual HookBase::HookResult beforeDownload(GameDownloadService *, ServiceState *state) override;
  virtual HookBase::HookResult afterDownload(GameDownloadService *, ServiceState *state) override;

};

#endif // _GGS_GAMEDOWNLOADER_SAMPLE_FAKEHOOK_H_