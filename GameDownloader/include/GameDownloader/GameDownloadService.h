#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/ProgressCalculator.h>

#include <GameDownloader/Algorithms/SimpleVersion.h>
#include <GameDownloader/Algorithms/BindiffVersion.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>
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

namespace P1 {
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

      explicit GameDownloadService(QObject *parent = 0);
      virtual ~GameDownloadService();

      enum TorrentProfile {
        DEFAULT_PROFILE = 0,
        HIGH_PERFORMANCE_SEED = 1,
        MIN_MEMORY_USAGE = 2
      };

      // Управление настройками торрента теперь осуществялется через GameDownloadService
      void setListeningPort(unsigned short port);
      void setTorrentConfigDirectoryPath(const QString& path);
      void changeListeningPort(unsigned short port);
      void setUploadRateLimit(int bytesPerSecond);
      void setDownloadRateLimit(int bytesPerSecond);
      void setMaxConnection(int maxConnection);
      void setSeedEnabled(bool value);
      void setTorrentProfile(GameDownloadService::TorrentProfile value);
      
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
      virtual void registerHook(const QString& serviceId, int preHookPriority, int postHookPriority, HookBase *hook);
      
      virtual bool isInProgress(const P1::Core::Service *service);
      virtual bool isAnyServiceInProgress();

      void setTimeoutForResume(quint32 seconds);

      virtual bool isInstalled(const QString& serviceId);
      virtual bool isInstalled(const P1::Core::Service *service);

      void release();

      /*!
      \fn GameDownloadService::setCredentials(const QString &userId, const QString &hash);
      \brief Устанавливает идентификатор пользователя, выполняеющего загрузку торрента. Подробнее в QGNA-1319.

      \param userId Идентификатор пользователя
      \param hash Хеш для текущего идентификатора
      */
      void setCredentials(const QString &userId, const QString &hash);

      /*!
      \fn GameDownloadService::resetCredentials();
      \brief Сбрасывает выставленные с помощью setCredentials данные пользователя.
      */
      void resetCredentials();

    public slots:
      virtual void start(const P1::Core::Service *service, P1::GameDownloader::StartType startType);
      virtual void stop(const P1::Core::Service *service);
      virtual void directoryChanged(const P1::Core::Service *service);
      virtual void pauseSession();
      virtual void resumeSession();

    signals:
      void started(const P1::Core::Service *service, P1::GameDownloader::StartType startType);
      void finished(const P1::Core::Service *service);
      void stopped(const P1::Core::Service *service);
      void stopping(const P1::Core::Service *service);
      void failed(const P1::Core::Service *service);
      void shutdownCompleted();
      void serviceInstalled(const P1::Core::Service *service);
      void serviceUpdated(const P1::Core::Service *service);
      void serviceUninstalled(const P1::Core::Service *service);

      void statusMessageChanged(const P1::Core::Service *service, const QString& message);

      // INFO 01.09.2014 сигнал не эмититься нигде - удалить в будущем
      void progressChanged(QString serviceId, qint8 progress); 
      // INFO 01.09.2014 сигнал не эмититься нигде - удалить в будущем
      void progressDownloadChanged(QString serviceId, qint8 progress, P1::Libtorrent::EventArgs::ProgressEventArgs args);
      // INFO 01.09.2014 сигнал не эмититься нигде - удалить в будущем
      void progressExtractionChanged(QString serviceId, qint8 progress, qint64 current, qint64 total);

      void totalProgressChanged(const P1::Core::Service *service, qint8 progress);
      void downloadProgressChanged(
        const P1::Core::Service *service, 
        qint8 progress, 
        P1::Libtorrent::EventArgs::ProgressEventArgs args);

      void listeningPortChanged(unsigned short port);
      void finishedDownloading(const P1::Core::Service *service);

      void accessRequired(const P1::Core::Service *service);

    private slots:
      void internalStatusMessageChanged(P1::GameDownloader::ServiceState *state, const QString& message);

      // INFO 01.09.2014 удалить - нигде не вызывается
      void internalProgressChangedSlot(QString serviceId, qint8 progress);
      // INFO 01.09.2014 удалить - нигде не вызывается
      void internalProgressDownloadChangedSlot(QString serviceId, qint8 progress, P1::Libtorrent::EventArgs::ProgressEventArgs args);
      // INFO 01.09.2014 удалить - нигде не вызывается
      void internalProgressExtractionChangedSlot(QString serviceId, qint8 progress, qint64 current, qint64 total);

      void internalTotalProgressChanged(P1::GameDownloader::ServiceState *state, qint8 progress);
      void internalDownloadProgressChanged(
        P1::GameDownloader::ServiceState *state,
        qint8 progress, 
        P1::Libtorrent::EventArgs::ProgressEventArgs args);

      void internalFinished(P1::GameDownloader::ServiceState *state);
      void internalStopped(P1::GameDownloader::ServiceState *state);
      void internalFailed(P1::GameDownloader::ServiceState *state);
      void internalTorrentDownloadFinished(P1::GameDownloader::ServiceState *state);

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

      P1::Libtorrent::Wrapper* _wrapper;

      Behavior::PreHookBehavior _preHookBehavior;
      Behavior::PostHookBehavior _postHook;

      StateMachine _machine;
    };
  }
}
