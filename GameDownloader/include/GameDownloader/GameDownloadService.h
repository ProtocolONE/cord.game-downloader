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
#include <GameDownloader/HookBase.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/ProgressCalculator.h>

#include <GameDownloader/Algorithms/SimpleVersion.h>
#include <GameDownloader/Algorithms/BindiffVersion.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>
#include <LibtorrentWrapper/Wrapper.h>

#include <GameDownloader/StateMachine.h>
#include <GameDownloader/Behavior/PreHookBehavior.h>
#include <GameDownloader/Behavior/PostHookBehavior.h>

#include <QtCore/QObject>
#include <QtCore/QMultiMap>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QMutex>

#include <windows.h>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class ServiceState;
    class ExtractorBase;

    class DOWNLOADSERVICE_EXPORT GameDownloadService : public QObject
    {
      Q_OBJECT
    public:
      friend class Algorithms::SimpleVersion;
      friend class Algorithms::BindiffVersion;

      GameDownloadService(QObject *parent = 0);
      ~GameDownloadService();

      // Управление настройками торрента теперь осуществялется через GameDownloadService
      void setListeningPort(unsigned short port);
      void setTorrentConfigDirectoryPath(const QString& path);
      void changeListeningPort(unsigned short port);
      void setUploadRateLimit(int bytesPerSecond);
      void setDownloadRateLimit(int bytesPerSecond);
      void setMaxConnection(int maxConnection);
      void setSeedEnabled(bool value);

      /*!
        \fn void GameDownloadService::init();
        \brief Инициализация этого объекта. Необходимо вызвать перед любой работой с объектом.
        \author Ilya.Tkachenko
        \date 25.05.2012
      */
      void init();

      /*!
        \fn void GameDownloadService::shutdown();
        \brief Завешает работу сервиса. Необходимо вызвать перед закрытием приложения.
        \author Ilya.Tkachenko
        \date 25.05.2012
      */
      void shutdown();

      /*!
        \fn void GameDownloadService::registerHook(const QString& serviceId, int preHookPriority,
          int postHookPriority, HookBase *hook);
        \brief Добавление хука для сервиса. Хуки выполняются в соответсвии с приоритетом от больших к меньшим.
        \author Ilya.Tkachenko
        \date 25.05.2012
        \param serviceId        Идентификатор сервиса.
        \param preHookPriority  Приоритет хука перед скаичванием.
        \param postHookPriority Приоритет хука после скачивания.
        \param [in,out] hook    Хук.
      */
      void registerHook(const QString& serviceId, int preHookPriority, int postHookPriority, HookBase *hook);
      
      bool isInProgress(const GGS::Core::Service *service);
      bool isAnyServiceInProgress();

      void setTimeoutForResume(quint32 seconds);

      bool isInstalled(const QString& serviceId);
      bool isInstalled(const GGS::Core::Service *service);

      void release();

    public slots:
      void start(const GGS::Core::Service *service, GGS::GameDownloader::StartType startType);
      void stop(const GGS::Core::Service *service);
      void directoryChanged(const GGS::Core::Service *service);
      void pauseSession();
      void resumeSession();

    signals:
      void started(const GGS::Core::Service *service, GGS::GameDownloader::StartType startType);
      void finished(const GGS::Core::Service *service);
      void stopped(const GGS::Core::Service *service);
      void stopping(const GGS::Core::Service *service);
      void failed(const GGS::Core::Service *service);
      void shutdownCompleted();
      void serviceInstalled(const GGS::Core::Service *service);
      void serviceUpdated(const GGS::Core::Service *service);

      void statusMessageChanged(const GGS::Core::Service *service, const QString& message);

      void progressChanged(QString serviceId, qint8 progress);
      void progressDownloadChanged(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args);
      void progressExtractionChanged(QString serviceId, qint8 progress, qint64 current, qint64 total);

      void totalProgressChanged(const GGS::Core::Service *service, qint8 progress);
      void downloadProgressChanged(
        const GGS::Core::Service *service, 
        qint8 progress, 
        GGS::Libtorrent::EventArgs::ProgressEventArgs args);

      void listeningPortChanged(unsigned short port);

    private slots:
      void internalStatusMessageChanged(GGS::GameDownloader::ServiceState *state, const QString& message);

      void internalProgressChangedSlot(QString serviceId, qint8 progress);
      void internalProgressDownloadChangedSlot(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args);
      void internalProgressExtractionChangedSlot(QString serviceId, qint8 progress, qint64 current, qint64 total);

      void internalTotalProgressChanged(GGS::GameDownloader::ServiceState *state, qint8 progress);
      void internalDownloadProgressChanged(
        GGS::GameDownloader::ServiceState *state,
        qint8 progress, 
        GGS::Libtorrent::EventArgs::ProgressEventArgs args);

      void internalFinished(GGS::GameDownloader::ServiceState *state);
      void internalStopped(GGS::GameDownloader::ServiceState *state);
      void internalFailed(GGS::GameDownloader::ServiceState *state);

    private:
      ServiceState* getStateById(const QString& id);
      void internalShuttingDownComplete();
      bool isAllStopped();
      int getDiskFreeSpaceInMb(LPCWSTR drive);
      
      QMutex _stateLock;
      QHash<QString, ServiceState* > _stateMap;

      ProgressCalculator _progressCalculator;

      bool _isShuttingDown;
      bool _isShutdownFinished;

      QSet<HookBase *> _registredHooks;

      //Algorithms::SimpleVersion _downloadAlgorithm;
      Algorithms::BindiffVersion _downloadAlgorithm;

      GGS::Libtorrent::Wrapper* _wrapper;

      Behavior::PreHookBehavior _preHookBehavior;
      Behavior::PostHookBehavior _postHook;

      StateMachine _machine;
    };

  }
}
