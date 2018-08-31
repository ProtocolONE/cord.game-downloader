#include "FakeHook.h"

#include <Core/Service.h>

#include <QtCore/QDebug>
#include <Windows.h>

FakeHook::FakeHook(const QString& id, QObject *parent)
  : GGS::GameDownloader::HookBase(id, parent)
{
}

FakeHook::~FakeHook()
{
}

HookBase::HookResult FakeHook::beforeDownload(GameDownloadService *, ServiceState *state)
{
  const GGS::Core::Service *service = state->service();
  qDebug() << "pre  hook " << this->hookId();
  emit this->beforeProgressChanged(service->id(), this->hookId(), 0);
  for (int i = 0; i < 10; ++i) {
    emit this->statusMessageChanged(state, QString("Pre hook step %1").arg(i));
    Sleep(500);
    emit this->beforeProgressChanged(service->id(), this->hookId(), 10 * i );
  }

  return HookBase::Continue;
}

HookBase::HookResult FakeHook::afterDownload(GameDownloadService *, ServiceState *state)
{
  const GGS::Core::Service *service = state->service();
  qDebug() << "post hook " << this->hookId();
  emit this->afterProgressChanged(service->id(), this->hookId(), 0);
  for (int i = 0; i < 10; ++i) {
    emit this->statusMessageChanged(state, QString("Post hook step %1").arg(i));
    Sleep(500);
    emit this->afterProgressChanged(service->id(), this->hookId(), 10 * i );
  }

  return HookBase::Continue;
}
