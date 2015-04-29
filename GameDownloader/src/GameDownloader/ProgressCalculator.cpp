/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2015, Syncopate Limited and/or affiliates.
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
    {
    }

    ProgressCalculator::~ProgressCalculator()
    {
    }

    void ProgressCalculator::registerBehavior(BaseBehavior *behavior)
    {
      QObject::connect(behavior, &BaseBehavior::totalProgressChanged,
        this, &ProgressCalculator::totalProgressSlot);
    }

    void ProgressCalculator::setBehaviorValue(ProgressType type, Behavior::BaseBehavior *behavior, float size)
    {
      ProgressStrategy& strategy(this->_strategies[type]);

      ProgressBlock block;
      block.startPoint = strategy.total;
      block.size = size;

      strategy.total += size;
      strategy.map[behavior] = block;
    }

    void ProgressCalculator::totalProgressSlot(GGS::GameDownloader::ServiceState *state, qint8 progress)
    {
      BaseBehavior *behavior = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!behavior)
        return;

      qint8 totalProgress = this->getProgress(state, behavior, progress);
      emit this->totalProgressChanged(state, totalProgress);
    }

    void ProgressCalculator::downloadSlot(
      GGS::GameDownloader::ServiceState *state, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      BaseBehavior *behavior = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!behavior)
        return;

      qint8 totalProgress = this->getProgress(state, behavior, progress);
      emit this->downloadProgressChanged(state, totalProgress, args);
    }

    qint8 ProgressCalculator::getProgress(GGS::GameDownloader::ServiceState *state, BaseBehavior *behavior, qint8 progress)
    {
      const QString& id(state->id());

      ItemProgress &itemProgress(this->_itemStates[id]);
      Q_ASSERT(this->_strategies.contains(itemProgress.progressType));

      const ProgressStrategy& strategy(this->_strategies[itemProgress.progressType]);

      if (strategy.map.contains(behavior)) {
        const ProgressBlock& block(strategy.map[behavior]);
        float resultTmp = block.startPoint + block.size * (static_cast<float>(progress) / 100.0f);
        resultTmp = 100 * (resultTmp / strategy.total);
        qint8 result = static_cast<qint8>(resultTmp);

        if (itemProgress.lastProgress < result)
          itemProgress.lastProgress = result;
      }

      return itemProgress.lastProgress;
    }

    void ProgressCalculator::resetProgress(GGS::GameDownloader::ServiceState *state)
    {
      ItemProgress &itemProgress(this->_itemStates[state->id()]);

      itemProgress.lastProgress = 0;

      if (state->startType() == GGS::GameDownloader::Uninstall) {
        itemProgress.progressType = ProgressCalculator::UninstallGame;
        return;
      }

      if (state->isInstalled()) {
        itemProgress.progressType = ProgressCalculator::UpdateGame;
      } else {
        itemProgress.progressType = (state->service()->extractorType() == "3A3AC78E-0332-45F4-A466-89C2B8E8BB9C") // DummyExtractor
          ? ProgressCalculator::InstallGameWithoutArchive
          : ProgressCalculator::InstallGameWithArchive;
      }
    }

    ProgressCalculator::ProgressBlock::ProgressBlock()
    {
      this->startPoint = 0.0f;
      this->size = 0.0f;
    }

    ProgressCalculator::ProgressStrategy::ProgressStrategy()
    {
      this->total = 0.0f;
    }

    ProgressCalculator::ItemProgress::ItemProgress()
    {
      this->lastProgress = 0;
      this->progressType = ProgressCalculator::UpdateGame;
    }

  }
}