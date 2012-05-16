/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_STAGEPROGRESCALCULATOR_H_
#define _GGS_GAMEDOWNLOADER_STAGEPROGRESCALCULATOR_H_

#include <GameDownloader/GameDownloader_global.h>
#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QPair>

namespace GGS {
  namespace GameDownloader {
    class HookBase;

    class DOWNLOADSERVICE_EXPORT StageProgressCalculator : public QObject
    {
      Q_OBJECT
    public:
      StageProgressCalculator(QObject *parent = 0);
      ~StageProgressCalculator();

      void registerHook(const QString& serviceId, int preHookPriority, int postHookPriority, HookBase *hook);

      void setCheckUpdateStageSize(quint8 size);
      void setPreHookStageSize(quint8 size);
      void setTorrentRehashingStageSize(quint8 size);
      void setTorrentDownloadinngStageSize(quint8 size);
      void setPostHookStageSize(quint8 size);
      void setExtractionStageSize(quint8 size);

    public slots:
      void checkUpdateProgress(const QString& serviceId, quint8 progress);
      void preHookProgress(const QString& serviceId, const QString& hookId, quint8 progress);
      void postHookProgress(const QString& serviceId, const QString& hookId, quint8 progress);
      void torrentProgress(GGS::Libtorrent::EventArgs::ProgressEventArgs args);
      void extractionProgress(const QString& serviceId, qint8 progress, qint64 current, qint64 total);

    signals:
      void progressChanged(QString serviceId, qint8 progress);
      void progressDownloadChanged(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args);
      void progressExtractionChanged(QString serviceId, qint8 progress, qint64 current, qint64 total);

    private:
      void recalculateTotalProgressSize();
      void recalculateHooksStages(int priority, HookBase *hook, bool isBefore,
        QMap<int, HookBase* > &priorityMap,
        QHash<QString, QPair<qint32, qint32> > &hookStateMap);

      qreal _checkUpdateStageSize;
      qreal _preHookStageSize;
      qreal _torrentRehashingStageSize;
      qreal _torrentDownloadingStageSize;
      qreal _postHookStageSize;
      qreal _extractionStageSize;

      qreal _totalProgressSize;

      qint32 _totalPreHookWeight;
      qint32 _totalPostHookWeight;

      QHash<QString, QMap<int, HookBase* > > _preHookPriorityMap;
      QHash<QString, QHash<QString, QPair<qint32, qint32> > > _preHookStateMap;

      QHash<QString, QMap<int, HookBase* > > _postHookPriorityMap;
      QHash<QString, QHash<QString, QPair<qint32, qint32> > > _postHookStateMap;
    };

  }
}
#endif // _GGS_GAMEDOWNLOADER_STAGEPROGRESCALCULATOR_H_