/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/StageProgressCalculator.h>
#include <QtCore/QDebug>

namespace GGS {
  namespace GameDownloader {

    StageProgressCalculator::StageProgressCalculator(QObject *parent)
      : QObject(parent)
      , _checkUpdateStageSize(1)
      , _preHookStageSize(14)
      , _torrentRehashingStageSize(20)
      , _torrentDownloadingStageSize(40)
      , _extractionStageSize(15)
      , _postHookStageSize(10)
    {
      this->recalculateTotalProgressSize();
    }

    StageProgressCalculator::~StageProgressCalculator()
    {
    }

    void StageProgressCalculator::checkUpdateProgress(const QString& serviceId, quint8 progress)
    {
      qint8 totalProgress = static_cast<qint8>(static_cast<qreal>(progress) * this->_checkUpdateStageSize / this->_totalProgressSize);
      emit this->progressChanged(serviceId, totalProgress);
    }

    void StageProgressCalculator::preHookProgress(const QString& serviceId, const QString& hookId, quint8 progress)
    {
      if (!this->_preHookStateMap.contains(serviceId))
        return;

      if (!this->_preHookStateMap[serviceId].contains(hookId))
        return;

      qreal beforeHook = this->_preHookStateMap[serviceId][hookId].first;
      qreal current = this->_preHookStateMap[serviceId][hookId].second;
      qreal preHookProgress = (beforeHook + current * progress / 100.0f) / this->_totalPreHookWeight;
      qint8 totalProgress = static_cast<qint8>(
        qRound(100 * ((this->_checkUpdateStageSize + preHookProgress * this->_preHookStageSize) / this->_totalProgressSize)));
      emit this->progressChanged(serviceId, totalProgress);
    }

    void StageProgressCalculator::postHookProgress(const QString& serviceId, const QString& hookId, quint8 progress)
    {
      if (!this->_postHookStateMap.contains(serviceId))
        return;

      if (!this->_postHookStateMap[serviceId].contains(hookId))
        return;

      qreal beforeHook = this->_postHookStateMap[serviceId][hookId].first;
      qreal current = this->_postHookStateMap[serviceId][hookId].second;
      qreal postHookProgress = (beforeHook + current * progress / 100.0f) / this->_totalPostHookWeight;
      qreal totalBeforePostHooks = this->_checkUpdateStageSize
        + this->_preHookStageSize
        + this->_torrentRehashingStageSize
        + this->_torrentDownloadingStageSize
        + this->_extractionStageSize;

      qint8 totalProgress = static_cast<qint8>(
        qRound(100 * ((totalBeforePostHooks + postHookProgress * this->_postHookStageSize) / this->_totalProgressSize)));
      emit this->progressChanged(serviceId, totalProgress);
    }

    void StageProgressCalculator::torrentProgress(GGS::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      using GGS::Libtorrent::EventArgs::ProgressEventArgs;
      if (!(args.status() == ProgressEventArgs::Downloading || args.status() == ProgressEventArgs::CheckingFiles))
        return;

      qint8 totalProgress;
      if (args.status() == ProgressEventArgs::Downloading) {
        qreal totalBeforeStagesSize = this->_checkUpdateStageSize
          + this->_preHookStageSize
          + this->_torrentRehashingStageSize;

        totalProgress = static_cast<qint8>(
          qRound(100 * ((totalBeforeStagesSize + args.progress() * this->_torrentDownloadingStageSize) / this->_totalProgressSize)));
        emit this->progressDownloadChanged(args.id(), totalProgress, args);

      } else {
        qreal totalBeforeStagesSize = this->_checkUpdateStageSize + this->_preHookStageSize;
        totalProgress = static_cast<qint8>(
          qRound(100 * ((totalBeforeStagesSize + args.progress() * this->_torrentRehashingStageSize) / this->_totalProgressSize)));
        emit this->progressChanged(args.id(), totalProgress);

      }
    }

    void StageProgressCalculator::extractionProgress(const QString& serviceId, qint8 progress, qint64 current, qint64 total)
    {
      qreal totalBeforeStagesSize =  
        this->_checkUpdateStageSize + 
        this->_preHookStageSize +
        this->_torrentRehashingStageSize +
        this->_torrentDownloadingStageSize;
      qint8 totalProgress = static_cast<qint8>(qRound(
        100 * ((totalBeforeStagesSize + progress * this->_extractionStageSize / 100) / this->_totalProgressSize)));
      emit this->progressExtractionChanged(serviceId, totalProgress, current, total);
    }

    void StageProgressCalculator::registerHook(const QString& serviceId, int preHookPriority, int postHookPriority, HookBase *hook)
    {
      if (!hook)
        return;
        
      if (hook->beforeProgressWeight() > 0)
        this->recalculateHooksStages(preHookPriority, hook, true, this->_preHookPriorityMap[serviceId], this->_preHookStateMap[serviceId]);
      
      if (hook->afterProgressWeight() > 0)
        this->recalculateHooksStages(postHookPriority, hook, false, this->_postHookPriorityMap[serviceId], this->_postHookStateMap[serviceId]);
    }

    void StageProgressCalculator::recalculateTotalProgressSize()
    {
      this->_totalProgressSize = 
        this->_checkUpdateStageSize + 
        this->_preHookStageSize +
        this->_torrentRehashingStageSize +
        this->_torrentDownloadingStageSize +
        this->_postHookStageSize +
        this->_extractionStageSize;
    }

    void StageProgressCalculator::setCheckUpdateStageSize(quint8 size)
    {
      this->_checkUpdateStageSize = size;
      this->recalculateTotalProgressSize();
    }

    void StageProgressCalculator::setPreHookStageSize(quint8 size)
    {
      this->_preHookStageSize = size;
      this->recalculateTotalProgressSize();
    }

    void StageProgressCalculator::setTorrentRehashingStageSize(quint8 size)
    {
      this->_torrentRehashingStageSize = size;
      this->recalculateTotalProgressSize();
    }

    void StageProgressCalculator::setTorrentDownloadinngStageSize(quint8 size)
    {
      this->_torrentDownloadingStageSize = size;
      this->recalculateTotalProgressSize();
    }

    void StageProgressCalculator::setPostHookStageSize(quint8 size)
    {
      this->_postHookStageSize = size;
      this->recalculateTotalProgressSize();
    }

    void StageProgressCalculator::setExtractionStageSize(quint8 size)
    {
      this->_extractionStageSize = size;
      this->recalculateTotalProgressSize();
    }

    void StageProgressCalculator::recalculateHooksStages(
      int priority, 
      HookBase *hook, 
      bool isBefore, 
      QMap<int, HookBase* > &priorityMap, 
      QHash<QString, QPair<qint32, qint32> > &hookStateMap)
    {
      priorityMap[-priority] = hook;
      QString hookId = hook->hookId();
      int current = 0;

      Q_FOREACH(HookBase *ihook, priorityMap) {
        hookStateMap[ihook->hookId()].first = current;
        int weight = isBefore ? ihook->beforeProgressWeight() : ihook->afterProgressWeight();
        hookStateMap[ihook->hookId()].second = weight;
        current += weight;
      }

      if (isBefore)
        this->_totalPreHookWeight = current;
      else 
        this->_totalPostHookWeight = current;
    }

  }
}