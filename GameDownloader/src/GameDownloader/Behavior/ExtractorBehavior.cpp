#include <GameDownloader/Behavior/ExtractorBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <QtConcurrent/QtConcurrentRun>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      ExtractorBehavior::ExtractorBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      ExtractorBehavior::~ExtractorBehavior()
      {
      }

      void ExtractorBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        const P1::Core::Service *service = state->service();
        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        QtConcurrent::run(extractor, &ExtractorBase::extract, state, state->startType());
      }

      void ExtractorBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        const P1::Core::Service *service = state->service();
        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        Q_CHECK_PTR(extractor);

        QMetaObject::invokeMethod(
          extractor, 
          "pauseRequestSlot", 
          Qt::QueuedConnection, 
          Q_ARG(P1::GameDownloader::ServiceState *, state));
      }

      void ExtractorBehavior::registerExtractor(ExtractorBase *extractor)
      {
        Q_CHECK_PTR(extractor);

        QString type = extractor->extractorId();
        Q_ASSERT(!this->_extractorMap.contains(type));

        this->_extractorMap[type] = extractor;

        QObject::connect(extractor, &ExtractorBase::extractFinished,
          this, &ExtractorBehavior::extractionCompleted, Qt::QueuedConnection);

        QObject::connect(extractor, &ExtractorBase::extractPaused,
          this, &ExtractorBehavior::pauseRequestCompleted, Qt::QueuedConnection);

        QObject::connect(extractor, &ExtractorBase::extractFailed,
          this, &ExtractorBehavior::extractionFailed, Qt::QueuedConnection);

        QObject::connect(extractor, &ExtractorBase::extractionProgressChanged,
          this, &ExtractorBehavior::extractionProgressChanged, Qt::QueuedConnection);
      }

      ExtractorBase* ExtractorBehavior::getExtractorByType(const QString& type)
      {
        if (!this->_extractorMap.contains(type))
          return 0;

        return this->_extractorMap[type];
      }

      void ExtractorBehavior::extractionCompleted(P1::GameDownloader::ServiceState *state)
      {
        emit this->next(Finished, state);
      }

      void ExtractorBehavior::extractionFailed(P1::GameDownloader::ServiceState *state)
      {
        emit this->failed(state);
      }

      void ExtractorBehavior::pauseRequestCompleted(P1::GameDownloader::ServiceState *state)
      {
        emit this->next(Paused, state);
      }

      void ExtractorBehavior::extractionProgressChanged(P1::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total)
      {
        if (state->currentBehavior() != this)
          return;

        emit this->totalProgressChanged(state, progress);
        if (total > 0) {
          QString msg = QObject::tr("EXTRACT_PROGRESS_STATUS_MESSAGE")
            .arg(QString::number(current))
            .arg(QString::number(total));
          this->statusMessageChanged(state, msg);
        }
      }

    }
  }
}