#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>
#include <QtCore/QMap>

namespace P1 {
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
        InstallGameWithoutArchive = 2,
        UninstallGame = 3
      };

      explicit ProgressCalculator(QObject *parent = 0);
      virtual ~ProgressCalculator();

      void registerBehavior(Behavior::BaseBehavior *behavior);
      void setBehaviorValue(ProgressType type, Behavior::BaseBehavior *behavior, float size);
      void resetProgress(P1::GameDownloader::ServiceState *state);

    public slots:
      void totalProgressSlot(P1::GameDownloader::ServiceState *state, qint8 progress);
      void downloadSlot(
        P1::GameDownloader::ServiceState *state, 
        qint8 progress, 
        P1::Libtorrent::EventArgs::ProgressEventArgs args);

    signals:
      void totalProgressChanged(P1::GameDownloader::ServiceState *state, qint8 progress);
      void downloadProgressChanged(
        P1::GameDownloader::ServiceState *state, 
        qint8 progress, 
        P1::Libtorrent::EventArgs::ProgressEventArgs args);

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

      qint8 getProgress(P1::GameDownloader::ServiceState *state, Behavior::BaseBehavior* behavior, qint8 progress);
    };

  }
}
