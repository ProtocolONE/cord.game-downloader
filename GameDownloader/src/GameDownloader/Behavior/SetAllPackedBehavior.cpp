#include <GameDownloader/Behavior/SetAllPackedBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <QtConcurrent/QtConcurrentRun>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      SetAllPackedBehavior::SetAllPackedBehavior(QObject *parent /*= 0*/)
        : BaseBehavior(parent)
      {

      }

      SetAllPackedBehavior::~SetAllPackedBehavior()
      {

      }

      void SetAllPackedBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        const P1::Core::Service *service = state->service();

        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        Q_CHECK_PTR(extractor);

        QtConcurrent::run(extractor, &ExtractorBase::setAllUnpacked, state);
      }

      void SetAllPackedBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

      ExtractorBase* SetAllPackedBehavior::getExtractorByType(const QString& type)
      {
        if (!this->_extractorMap.contains(type))
          return 0;

        return this->_extractorMap[type];
      }

      void SetAllPackedBehavior::registerExtractor(ExtractorBase *extractor)
      {
        Q_CHECK_PTR(extractor);

        QString type = extractor->extractorId();
        Q_ASSERT(!this->_extractorMap.contains(type));

        this->_extractorMap[type] = extractor;

        QObject::connect(extractor, SIGNAL(unpackStateSaved(P1::GameDownloader::ServiceState *)), 
          this, SLOT(internalUnpackStateSaved(P1::GameDownloader::ServiceState *)), Qt::QueuedConnection);

        QObject::connect(extractor, SIGNAL(unpackStateSaveFailed(P1::GameDownloader::ServiceState *)), 
          this, SLOT(internalUnpackStateSaveFailed(P1::GameDownloader::ServiceState *)), Qt::QueuedConnection);

      }

      void SetAllPackedBehavior::internalUnpackStateSaved(P1::GameDownloader::ServiceState* state)
      {
        emit this->next(Finished, state);
      }

      void SetAllPackedBehavior::internalUnpackStateSaveFailed(P1::GameDownloader::ServiceState* state)
      {
        emit this->failed(state);
      }

    }
  }
}