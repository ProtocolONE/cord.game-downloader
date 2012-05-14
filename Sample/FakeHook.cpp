#include "FakeHook.h"

#include <QtCore/QDebug>
#include <Windows.h>

FakeHook::FakeHook(const QString& id, QObject *parent)
  : GGS::GameDownloader::HookBase(id, parent)
{
}

FakeHook::~FakeHook()
{
}

GGS::GameDownloader::HookBase::HookResult FakeHook::beforeDownload(const GGS::Core::Service *service)
{
  qDebug() << "pre  hook " << this->hookId();
  emit this->beforeProgressChanged(service->id(), this->hookId(), 0);
  for (int i = 0; i < 10; ++i) {
    Sleep(500);
    emit this->beforeProgressChanged(service->id(), this->hookId(), 10 * i );
  }

  return GGS::GameDownloader::HookBase::HookResult::Continue;
}

GGS::GameDownloader::HookBase::HookResult FakeHook::afterDownload(const GGS::Core::Service *service)
{
  qDebug() << "post hook " << this->hookId();
  emit this->afterProgressChanged(service->id(), this->hookId(), 0);
  for (int i = 0; i < 10; ++i) {
    Sleep(500);
    emit this->afterProgressChanged(service->id(), this->hookId(), 10 * i );
  }

  return GGS::GameDownloader::HookBase::HookResult::Continue;
}
