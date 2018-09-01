#include <GameDownloader/Behavior/CompressorBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <QtConcurrent/QtConcurrentRun>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      CompressorBehavior::CompressorBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      CompressorBehavior::~CompressorBehavior()
      {
      }
      
      void CompressorBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        const P1::Core::Service *service = state->service();

        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        QtConcurrent::run(extractor, &ExtractorBase::compress, state);
      }

      void CompressorBehavior::stop(P1::GameDownloader::ServiceState *state)
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

      void CompressorBehavior::registerCompressor(ExtractorBase *extractor)
      {
        Q_CHECK_PTR(extractor);

        QString type = extractor->extractorId();
        Q_ASSERT(!this->_extractorMap.contains(type));

        this->_extractorMap[type] = extractor;

        SIGNAL_CONNECT_CHECK(QObject::connect(extractor, SIGNAL(compressFinished(P1::GameDownloader::ServiceState *)), 
          this, SLOT(compressCompleted(P1::GameDownloader::ServiceState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(extractor, SIGNAL(compressPaused(P1::GameDownloader::ServiceState *)), 
          this, SLOT(pauseRequestCompleted(P1::GameDownloader::ServiceState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(extractor, SIGNAL(compressFailed(P1::GameDownloader::ServiceState *)), 
          this, SLOT(compressFailed(P1::GameDownloader::ServiceState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(
          extractor, 
          SIGNAL(compressProgressChanged(P1::GameDownloader::ServiceState*, qint8, qint64, qint64)), 
          this, 
          SLOT(compressProgressChanged(P1::GameDownloader::ServiceState*, qint8, qint64, qint64)), 
          Qt::QueuedConnection));
      }

      ExtractorBase* CompressorBehavior::getExtractorByType(const QString& type)
      {
        if (!this->_extractorMap.contains(type))
          return 0;

        return this->_extractorMap[type];
      }

      void CompressorBehavior::compressCompleted(P1::GameDownloader::ServiceState *state)
      {
        emit this->next(Finished, state);
      }

      void CompressorBehavior::compressFailed(P1::GameDownloader::ServiceState *state)
      {
        emit this->failed(state);
      }

      void CompressorBehavior::pauseRequestCompleted(P1::GameDownloader::ServiceState *state)
      {
        emit this->next(Paused, state);
      }

      void CompressorBehavior::compressProgressChanged(
        P1::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total)
      {
        if (state->currentBehavior() != this)
          return;

        emit this->totalProgressChanged(state, progress);
        if (total > 0) {
          QString msg = QObject::tr("COMPRESS_PROGRESS_STATUS_MESSAGE")
            .arg(QString::number(current))
            .arg(QString::number(total));
          this->statusMessageChanged(state, msg);
        }
      }

    }
  }
}