#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ServiceState.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {

    class GameDownloadService;
    class DOWNLOADSERVICE_EXPORT HookBase : public QObject
    {
      Q_OBJECT
      Q_ENUMS(P1::GameDownloader::HookBase::HookResult);
    public:
      
      // UNDONE Пересмотреть и вохможно уменьшить число результатов
      enum HookResult {
        Continue = 0,
        Abort = 1,
        CheckUpdate,
        PreHookStartPoint,
        PreHookEndPoint,
        PostHookStartPoint,
        PostHookEndPoint,
        DownloadStartPoint,
        DownloadEndPoint,
        ExtractionStartPoint,
        ExtractionEndPoint,
        Finish,
      };

      HookBase(const QString& hookId, QObject *parent = 0);
      virtual ~HookBase();

      virtual HookResult beforeDownload(GameDownloadService *gameDownloader, ServiceState *state) = 0;
      virtual HookResult afterDownload(GameDownloadService *gameDownloader, ServiceState *state) = 0;

      const QString& hookId();

      quint8 beforeProgressWeight() const;
      void setBeforeProgressWeight(quint8 weight);
      
      quint8 afterProgressWeight() const;
      void setAfterProgressWeight(quint8 weight);

    public slots:
      void pauseRequestSlot(P1::GameDownloader::ServiceState *state);

    signals:
      void beforeProgressChanged(const QString& serviceId, const QString& hookId, quint8 progress);
      void afterProgressChanged(const QString& serviceId, const QString& hookId, quint8 progress);
      void statusMessageChanged(P1::GameDownloader::ServiceState *state, const QString& message);

      void pauseRequest(P1::GameDownloader::ServiceState *state);

    protected:
      quint8 _beforeProgressWeight;
      quint8 _afterProgressWeight;
      QString _hookId;

    };
  }
}
