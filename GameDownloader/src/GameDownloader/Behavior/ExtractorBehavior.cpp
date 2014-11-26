/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/ExtractorBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <QtConcurrent/QtConcurrentRun>

#include <Core/Service.h>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      ExtractorBehavior::ExtractorBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      ExtractorBehavior::~ExtractorBehavior()
      {
      }

      void ExtractorBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        const GGS::Core::Service *service = state->service();
        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        QtConcurrent::run(extractor, &ExtractorBase::extract, state, state->startType());
      }

      void ExtractorBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        const GGS::Core::Service *service = state->service();
        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        Q_CHECK_PTR(extractor);

        QMetaObject::invokeMethod(
          extractor, 
          "pauseRequestSlot", 
          Qt::QueuedConnection, 
          Q_ARG(GGS::GameDownloader::ServiceState *, state));
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

      void ExtractorBehavior::extractionCompleted(GGS::GameDownloader::ServiceState *state)
      {
        emit this->next(Finished, state);
      }

      void ExtractorBehavior::extractionFailed(GGS::GameDownloader::ServiceState *state)
      {
        emit this->failed(state);
      }

      void ExtractorBehavior::pauseRequestCompleted(GGS::GameDownloader::ServiceState *state)
      {
        emit this->next(Paused, state);
      }

      void ExtractorBehavior::extractionProgressChanged(GGS::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total)
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