/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/ProgressCalculator.h>
#include <GameDownloader/Behavior/BaseBehavior.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service.h>

using GGS::GameDownloader::Behavior::BaseBehavior;

namespace GGS {
  namespace GameDownloader {

    ProgressCalculator::ProgressCalculator(QObject *parent)
      : QObject(parent)
      , _currentMax(0)
    {
    }

    ProgressCalculator::~ProgressCalculator()
    {
    }

    void ProgressCalculator::registerBehavior(BaseBehavior *behavior, float size)
    {
      ProgressBlock block;
      block.startPoint = this->_currentMax;
      block.size = size;
      this->_currentMax += size;
      this->_map[behavior] = block;

      SIGNAL_CONNECT_CHECK(QObject::connect(
        behavior, SIGNAL(totalProgressChanged(GGS::GameDownloader::ServiceState *, qint8)),
        this, SLOT(totalProgressSlot(GGS::GameDownloader::ServiceState *, qint8))));
    }

    void ProgressCalculator::totalProgressSlot(GGS::GameDownloader::ServiceState *state, qint8 progress)
    {
      BaseBehavior *behavior = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!behavior)
        return;

      Q_ASSERT(this->_map.contains(behavior));
      qint8 totalProgress = this->getProgress(state, behavior, progress);
      emit this->totalProgressChanged(state, totalProgress);
    }

    void ProgressCalculator::downloadSlot(
      GGS::GameDownloader::ServiceState *state, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      BaseBehavior *behavior = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!behavior)
        return;

      Q_ASSERT(this->_map.contains(behavior));
      qint8 totalProgress = this->getProgress(state, behavior, progress);
      emit this->downloadProgressChanged(state, totalProgress, args);
    }

    qint8 ProgressCalculator::getProgress(GGS::GameDownloader::ServiceState *state, BaseBehavior *behavior, qint8 progress)
    {
      ProgressBlock block = this->_map[behavior];
      float resultTmp = block.startPoint + block.size * (static_cast<float>(progress) / 100.0f);
      resultTmp = 100 * (resultTmp / this->_currentMax);
      qint8 result = static_cast<qint8>(resultTmp);

      qint8 lastResult = this->lastProgress(state);
      if (lastResult <= result) {
        this->setLastProgress(state, result);
        lastResult = result;
      }
        
      return lastResult;
    }

    qint8 ProgressCalculator::lastProgress(GGS::GameDownloader::ServiceState *state)
    {
      if (this->_lastProgress.contains(state->id()))
        return this->_lastProgress[state->id()];

      this->_lastProgress[state->id()] = 0;
      return 0;
    }

    void ProgressCalculator::setLastProgress(GGS::GameDownloader::ServiceState *state, qint8 value)
    {
      this->_lastProgress[state->id()] = value;
    }

    void ProgressCalculator::resetProgress(GGS::GameDownloader::ServiceState *state)
    {
      this->_lastProgress[state->id()] = 0;
    }

  }
}