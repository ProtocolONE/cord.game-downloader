#pragma once

#include <GameDownloader/GameDownloader_global.h>

#include <GameDownloader/Behavior/CheckUpdateBehavior.h>
#include <GameDownloader/Behavior/TorrentDownloadBehavior.h>
#include <GameDownloader/Behavior/ExtractorBehavior.h>
#include <GameDownloader/Behavior/FinishBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {
    class GameDownloadService;

    namespace Algorithms {
      class DOWNLOADSERVICE_EXPORT SimpleVersion : public QObject
      {
        Q_OBJECT
      public:
        explicit SimpleVersion(QObject *parent = 0);
        ~SimpleVersion();

        void build(GameDownloadService* gameDownloader);

      private:
        GameDownloadService* _gameDownloader;

        Behavior::CheckUpdateBehavior _checkUpdateBehavior;
        Behavior::TorrentDownloadBehavior _downloadBehavior;
        Behavior::ExtractorBehavior _extractionBehavior;
        Behavior::FinishBehavior _finishBehavior;

        void registerExtractor(ExtractorBase *extractor);

        void setStartBehavior(Behavior::BaseBehavior *behavior);
        void registerBehavior(Behavior::BaseBehavior *behavior);
        void setProgress(Behavior::BaseBehavior *behavior, int size);

        // Эта функцию проверяет на этапе компиляции соответсвие аргументов
        template <typename T>
        void setRoute(T *from, typename T::Results result, Behavior::BaseBehavior *to) {
          this->_gameDownloader->_machine.setRoute(from, result, to);
        }
      };

    }
  }
}
