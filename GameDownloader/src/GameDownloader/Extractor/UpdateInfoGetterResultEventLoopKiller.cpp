#include <GameDownloader/Extractor/UpdateInfoGetterResultEventLoopKiller.h>

namespace P1 {
  namespace GameDownloader {
    namespace Extractor {

      UpdateInfoGetterResultEventLoopKiller::UpdateInfoGetterResultEventLoopKiller(QObject *parent)
        : QObject(parent)
        , _loop(0)
      {
      }

      UpdateInfoGetterResultEventLoopKiller::~UpdateInfoGetterResultEventLoopKiller()
      {
      }

      void UpdateInfoGetterResultEventLoopKiller::updateInfoCallback(P1::UpdateSystem::UpdateInfoGetterResults error)
      {
        Q_ASSERT(this->_loop != 0);
        this->_result = error;
        this->_loop->exit();
      }

      void UpdateInfoGetterResultEventLoopKiller::infoGetterUpdateProggress(quint64 current, quint64 total)
      {
      }

      void UpdateInfoGetterResultEventLoopKiller::setEventLoop(QEventLoop *loop)
      {
        this->_loop = loop;
      }

      P1::UpdateSystem::UpdateInfoGetterResults UpdateInfoGetterResultEventLoopKiller::result() const
      {
        return this->_result;
      }

    }
  }
}