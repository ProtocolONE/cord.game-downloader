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

#include <GameDownloader/Behavior/CheckUpdateBehavior.h>
#include <GameDownloader/Behavior/TorrentDownloadBehavior.h>
#include <GameDownloader/Behavior/ExtractorBehavior.h>
#include <GameDownloader/Behavior/FinishBehavior.h>

#include <QtCore/QObject>

namespace GGS {
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
