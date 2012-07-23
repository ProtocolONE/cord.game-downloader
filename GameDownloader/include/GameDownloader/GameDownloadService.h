/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#ifndef _GGS_GAMEDOWNLOADER_GAMEDOWNLOADSERVICE_H_
#define _GGS_GAMEDOWNLOADER_GAMEDOWNLOADSERVICE_H_

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/StartType.h>
#include <GameDownloader/StageProgressCalculator.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>

#include <QtCore/QObject>
#include <QtCore/QMultiMap>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QMutex>

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
      GameDownloadService(QObject *parent = 0);
      ~GameDownloadService();

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


      /*!
        \fn void GameDownloadService::registerExtractor(ExtractorBase *extractor);
        \brief Добавление поддерживаемых екстракторов.
        \author Ilya.Tkachenko
        \date 25.05.2012
        \param [in,out] extractor If non-null, the extractor.
      */
      void registerExtractor(ExtractorBase *extractor);
      

      /*!
        \fn bool GameDownloadService::isStoppedOrStopping(const GGS::Core::Service *service);
        \brief Проверяет состояние закачки по сервису. Возращает true, если скачи вание остановлено или находиться в 
               процессс остановки. 
        \author Ilya.Tkachenko
        \date 25.05.2012
        \param service The service.
        \return true if stopped or stopping, false if not.
      */
      virtual bool isStoppedOrStopping(const GGS::Core::Service *service);
      
      bool isInProgress(const GGS::Core::Service *service);
      bool isAnyServiceInProgress();

      StageProgressCalculator& progressCalculator();

      void setTimeoutForResume(quint32 seconds);

      bool isInstalled(const QString& serviceId);
      bool isInstalled(const GGS::Core::Service *service);

    public slots:
      void start(const GGS::Core::Service *service, GGS::GameDownloader::StartType startType);
      void stop(const GGS::Core::Service *service);

      /*!
        \fn void GameDownloadService::downloadRequestCompleted(const GGS::Core::Service *service);
        \brief Функция должна быть вызвана по окончанию скачивания игры каким либо менеджером закачек.
               Она должна быть вызвана как результат сигнала downloadRequest.
        \author Ilya.Tkachenko
        \date 25.04.2012
        \param service The service.
      */
      void downloadRequestCompleted(const GGS::Core::Service *service);

      /*!
        \fn void GameDownloadService::pauseRequestCompleted(const GGS::Core::Service *service);
        \brief Функция должна быть вызвана по окончанию остановки скачивания игры каким либо менеджером закачек.
               Она должна быть вызвана как результат сигнала pauseRequest.
        \author Ilya.Tkachenko
        \date 25.04.2012
        \param service The service.
      */
      void pauseRequestCompleted(const GGS::Core::Service *service);

      /*!
        \fn void GameDownloadService::checkUpdateRequestCompleted(const GGS::Core::Service *service);
        \brief Функция должна быть вызвана по окончанию проверки наличия обновления игры.
               Она должна быть вызвана как результат сигнала checkUpdateRequest.
        \author Ilya.Tkachenko
        \date 25.04.2012
        \param service The service.
      */
      void checkUpdateRequestCompleted(const GGS::Core::Service *service, bool isUpdated);

      void checkUpdateFailed(const GGS::Core::Service *service);

      void extractionCompleted(const GGS::Core::Service *service);
      void extractionFailed(const GGS::Core::Service *service);

      void downloadFailed(const GGS::Core::Service *service);

      void directoryChanged(const GGS::Core::Service *service);

    signals:
      void started(const GGS::Core::Service *service);
      void finished(const GGS::Core::Service *service);
      void stopped(const GGS::Core::Service *service);
      void stopping(const GGS::Core::Service *service);
      void failed(const GGS::Core::Service *service);
      void shutdownCompleted();

      void statusMessageChanged(const GGS::Core::Service *service, const QString& message);

      void progressChanged(QString serviceId, qint8 progress);
      void progressDownloadChanged(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args);
      void progressExtractionChanged(QString serviceId, qint8 progress, qint64 current, qint64 total);

      /*!
        \fn void GameDownloadService::downloadStopRequest(const GGS::Core::Service *service);
        \brief Событие в результате которого должен остановиться торрент. Выделен особо от евента stopping не с проста.
               Это необходимо, чтобы различать какая стадия сейчас остановилась и не было двойной остановки. 
               Все стадии кроме скачивания живут короткое время не пересекающееся с другими стадиями, по-этому для них 
               не выделен отдельный сигнал.
        \author Ilya.Tkachenko
        \date 12.05.2012
        \param service The service.
      */
      void downloadStopRequest(const GGS::Core::Service *service);

      void preHooksCompleted(const GGS::Core::Service *service, GGS::GameDownloader::HookBase::HookResult result);
      void postHooksCompleted(const GGS::Core::Service *service, GGS::GameDownloader::HookBase::HookResult result);
      void checkUpdateRequest(const GGS::Core::Service *service, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType type);
      void downloadRequest(const GGS::Core::Service *service, GGS::GameDownloader::StartType startType, bool isReloadRequired);


    protected:
      void setIsInstalled(const QString& serviceId, bool isInstalled);

    private slots:
      void preHooksCompletedSlot(const GGS::Core::Service *service, GGS::GameDownloader::HookBase::HookResult result);
      void postHooksCompletedSlot(const GGS::Core::Service *service, GGS::GameDownloader::HookBase::HookResult result);

      void internalProgressChangedSlot(QString serviceId, qint8 progress);
      void internalProgressDownloadChangedSlot(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args);
      void internalProgressExtractionChangedSlot(QString serviceId, qint8 progress, qint64 current, qint64 total);

    private:
      ServiceState* getStateById(const QString& id);
      ExtractorBase* getExtractorByType(const QString& type);

      inline void startCheckUpdate(const GGS::Core::Service *service, ServiceState *state);
      inline void startPreHooks(const GGS::Core::Service *service, ServiceState *state);
      inline void startPostHooks(const GGS::Core::Service *service, ServiceState *state);
      inline void startDownload(const GGS::Core::Service *service, ServiceState *state);
      inline void startExtract(const GGS::Core::Service *service, ServiceState *state);
      inline void startFinish(const GGS::Core::Service *service, ServiceState *state);
      inline void startStopping(const GGS::Core::Service *service, ServiceState *state);

      void setStoppedService(const GGS::Core::Service *service);
      void setStoppedState(const GGS::Core::Service *service, ServiceState *state);

      void hookResultRouter(const GGS::Core::Service *service, ServiceState *state, GGS::GameDownloader::HookBase::HookResult result);
      void preHookLoop(const Core::Service *service);
      void postHookLoop(const Core::Service *service);

      void downloadServiceFailed(const Core::Service *service);

      QHash<QString, QMultiMap<int, HookBase*> > _beforeDownloadHookMap;
      QHash<QString, QMultiMap<int, HookBase*> > _afterDownloadHookMap;
      
      QMutex _stateLock;
      QHash<QString, ServiceState* > _stateMap;
      QHash<QString, ExtractorBase *> _extractorMap;
      qint64 _timeForResume;
      StageProgressCalculator _progressCalculator;

      bool _isShuttingDown;
      QSet<QString> _serviceStopList;

      QSet<HookBase *> _registredHooks;
    };

  }
}

#endif // _GGS_GAMEDOWNLOADER_GAMEDOWNLOADSERVICE_H_

