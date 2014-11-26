/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>
#include <QtCore/QMap>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {
      class BaseBehavior;
    }

    class DOWNLOADSERVICE_EXPORT ProgressCalculator : public QObject
    {
      Q_OBJECT
    public:
      enum ProgressType {
        UpdateGame = 0,
        InstallGameWithArchive = 1,
        InstallGameWithoutArchive = 2
      };

      explicit ProgressCalculator(QObject *parent = 0);
      virtual ~ProgressCalculator();

      void registerBehavior(Behavior::BaseBehavior *behavior);
      void setBehaviorValue(ProgressType type, Behavior::BaseBehavior *behavior, float size);
      void resetProgress(GGS::GameDownloader::ServiceState *state);

    public slots:
      void totalProgressSlot(GGS::GameDownloader::ServiceState *state, qint8 progress);
      void downloadSlot(
        GGS::GameDownloader::ServiceState *state, 
        qint8 progress, 
        GGS::Libtorrent::EventArgs::ProgressEventArgs args);

    signals:
      void totalProgressChanged(GGS::GameDownloader::ServiceState *state, qint8 progress);
      void downloadProgressChanged(
        GGS::GameDownloader::ServiceState *state, 
        qint8 progress, 
        GGS::Libtorrent::EventArgs::ProgressEventArgs args);

    private:
      struct ProgressBlock 
      {
        ProgressBlock();

        float startPoint;
        float size;
      };

      struct ProgressStrategy 
      {
        ProgressStrategy();

        float total;
        QMap<Behavior::BaseBehavior *, ProgressBlock> map;
      };

      struct ItemProgress
      {
        ItemProgress();

        qint8 lastProgress;
        ProgressType progressType;
      };

      QMap<ProgressType, ProgressStrategy> _strategies;
      QMap<QString, ItemProgress> _itemStates;

      qint8 getProgress(GGS::GameDownloader::ServiceState *state, Behavior::BaseBehavior* behavior, qint8 progress);
    };

  }
}
