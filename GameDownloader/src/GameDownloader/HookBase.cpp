#include <GameDownloader/HookBase.h>
#include <GameDownloader/GameDownloadService.h>

namespace P1 {
  namespace GameDownloader {
    HookBase::~HookBase()
    {
    }

    HookBase::HookBase(const QString& hookId, QObject *parent)
      : QObject(parent)
      , _hookId(hookId)
      , _beforeProgressWeight(0)
      , _afterProgressWeight(0)
    {
    }

    const QString& HookBase::hookId()
    {
      return this->_hookId;
    }

    quint8 HookBase::beforeProgressWeight() const
    {
      return this->_beforeProgressWeight;
    }

    void HookBase::setBeforeProgressWeight(quint8 weight)
    {
      this->_beforeProgressWeight = weight;
    }

    quint8 HookBase::afterProgressWeight() const
    {
      return this->_afterProgressWeight;
    }

    void HookBase::setAfterProgressWeight(quint8 weight)
    {
      this->_afterProgressWeight = weight;
    }

    void HookBase::pauseRequestSlot(P1::GameDownloader::ServiceState *state)
    {
      Q_ASSERT(state);
      emit this->pauseRequest(state);
    }

  }
}