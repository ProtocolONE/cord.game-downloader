#pragma once

#include <GameDownloader/HookBase.h>
#include <QtCore/QList>
#include <QtCore/QString>

namespace P1 {
  namespace GameDownloader {
    class GameDownloadService;
  }
}

using P1::GameDownloader::HookBase;
using P1::GameDownloader::ServiceState;
using P1::GameDownloader::GameDownloadService;

class SimpleContinueHook : public P1::GameDownloader::HookBase
{
public:
  SimpleContinueHook(int hookId, QList<int> *preList, QList<int> *postList);
  virtual ~SimpleContinueHook();

  virtual HookBase::HookResult beforeDownload(GameDownloadService *, ServiceState *state) override;
  virtual HookBase::HookResult afterDownload(GameDownloadService *, ServiceState *state) override;

  int beforeCallCount() const { return this->_beforeCallCount; }
  int afterCallCount() const { return this->_afterCallCount; }

private:
  QList<int> * _preList;
  QList<int> * _postList;
  int _hookId;

  int _beforeCallCount;
  int _afterCallCount;
};
