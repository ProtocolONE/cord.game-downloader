#pragma once
#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class BindiffBehaviorPrivate : public QObject 
      {
        Q_OBJECT
      public:
        explicit BindiffBehaviorPrivate(QObject *parent = 0);
        ~BindiffBehaviorPrivate();

        P1::GameDownloader::XdeltaWrapper::XdeltaDecoder _decoder;
        QStringList _patchFiles;
        QString _workingFile;
        QString _rootDir;
        QString _patchDir;
        QString _patchVersion;
        P1::GameDownloader::ServiceState* _state;

        qint64 _totalFileCount;
        qint64 _filesCount;
        bool _skipMode;
        int _progressMod;
      };
    };
  };
};