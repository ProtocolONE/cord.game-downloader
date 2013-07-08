#ifndef _GGS_GAMEDOWNLOADER_TEST_SIMPLECONTINUEHOOK_H_
#define _GGS_GAMEDOWNLOADER_TEST_SIMPLECONTINUEHOOK_H_

#include <GameDownloader/HookBase.h>
#include <QtCore/QList>
#include <QtCore/QString>

namespace GGS {
  namespace GameDownloader {
    class GameDownloadService;
  }
}

using GGS::GameDownloader::HookBase;
using GGS::GameDownloader::ServiceState;
using GGS::GameDownloader::GameDownloadService;

class SimpleContinueHook : public GGS::GameDownloader::HookBase
{
public:
  SimpleContinueHook(int hookId, QList<int> *preList, QList<int> *postList);
  ~SimpleContinueHook();

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

#endif // _GGS_GAMEDOWNLOADER_TEST_SIMPLECONTINUEHOOK_H_