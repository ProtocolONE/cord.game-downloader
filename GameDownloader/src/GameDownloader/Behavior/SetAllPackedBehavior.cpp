/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/SetAllPackedBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <QtConcurrent/QtConcurrentRun>

#include <Core/Service.h>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      SetAllPackedBehavior::SetAllPackedBehavior(QObject *parent /*= 0*/)
        : BaseBehavior(parent)
      {

      }

      SetAllPackedBehavior::~SetAllPackedBehavior()
      {

      }

      void SetAllPackedBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        const GGS::Core::Service *service = state->service();

        ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
        Q_CHECK_PTR(extractor);

        QtConcurrent::run(extractor, &ExtractorBase::setAllUnpacked, state);
      }

      void SetAllPackedBehavior::stop(GGS::GameDownloader::ServiceState *state)
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

        QObject::connect(extractor, SIGNAL(unpackStateSaved(GGS::GameDownloader::ServiceState *)), 
          this, SLOT(internalUnpackStateSaved(GGS::GameDownloader::ServiceState *)), Qt::QueuedConnection);

        QObject::connect(extractor, SIGNAL(unpackStateSaveFailed(GGS::GameDownloader::ServiceState *)), 
          this, SLOT(internalUnpackStateSaveFailed(GGS::GameDownloader::ServiceState *)), Qt::QueuedConnection);

      }

      void SetAllPackedBehavior::internalUnpackStateSaved(GGS::GameDownloader::ServiceState* state)
      {
        emit this->next(Finished, state);
      }

      void SetAllPackedBehavior::internalUnpackStateSaveFailed(GGS::GameDownloader::ServiceState* state)
      {
        emit this->failed(state);
      }

    }
  }
}