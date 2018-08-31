#pragma once

#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/StartType.h>

#include <LibtorrentWrapper/Wrapper.h>
#include <LibtorrentWrapper/TorrentConfig>
#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>

#include <Core/Service>
#include <Settings/Settings>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>
#include <QtCore/QHash>
#include <QtCore/QFile>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtWidgets/QMainWindow>


class Program : public QObject
{
  Q_OBJECT
    Q_PROPERTY(int progress1 READ progress1 NOTIFY progress1Changed)
    Q_PROPERTY(int progress2 READ progress2 NOTIFY progress2Changed)

    Q_PROPERTY(QString statusText1 READ statusText1 NOTIFY statusText1Changed)
    Q_PROPERTY(QString statusText2 READ statusText2 NOTIFY statusText2Changed)

    Q_PROPERTY(QString port READ port WRITE setPort NOTIFY portChanged)

public:

  Program();
  ~Program();

  int progress1() const { return this->_progress1; }
  int progress2() const { return this->_progress2; }

  QString statusText1() const { return this->_statusText1; }
  QString statusText2() const { return this->_statusText2; }

  QString port() const { return this->_port; }

public slots:
  void setPort(QString port);

  void setUploadRateLimit(QString bytesPerSecond);
  void setDownloadRateLimit(QString bytesPerSecond);

  void startTorrent(QString id);
  void stopTorrent(QString id);
  void restartTorrent(QString id);
  void recheckTorrent(QString id);

  void torrentChangePort();
  void shutdown();
  void shutdownCompleted();

  void progressChanged(QString serviceId, qint8 progress);
  void progressDownloadChanged(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args);
  void progressExtractionChanged(QString serviceId, qint8 progress, qint64 current, qint64 total);

  void totalProgressChanged(const GGS::Core::Service *service, qint8 progress);
  void downloadProgressChanged(
    const GGS::Core::Service *service, 
    qint8 progress, 
    GGS::Libtorrent::EventArgs::ProgressEventArgs args);

  void gameDownloaderStarted(const GGS::Core::Service *service);
  void gameDownloaderFinished(const GGS::Core::Service *service);
  void gameDownloaderStopped(const GGS::Core::Service *service);
  void gameDownloaderStopping(const GGS::Core::Service *service);
  void gameDownloaderFailed(const GGS::Core::Service *service);

  void changeDirectory(QString serviceId, QString downloadPath, QString installPath);

  void gameDownloaderStatusChanged(const GGS::Core::Service *service, const QString& message);

signals:
  void progress1Changed();
  void progress2Changed();
  void statusText1Changed();
  void statusText2Changed();
  void portChanged();

  void serviceProgressChanged(QString id, int progress);

  void serviceProgressChanged2(QString id, int progress);

  void serviceStatusChanged(QString id, QString status);

private:
  void setProgress1(int progress);
  void setProgress2(int progress);
  void setStatusText1(QString status);
  void setStatusText2(QString status);

  void initDatabase();
  void initServices();
  void initService(const QString& id, const QString& torrentUrl);

  GGS::Core::Service* getService(const QString& id);

  int _progress1;
  int _progress2;

  QString _port;

  QString _statusText1;
  QString _statusText2;

  QDeclarativeView *nQMLContainer;
  GGS::GameDownloader::GameDownloadService _gameDownloaderService;
  QHash<QString, GGS::Core::Service *> _serviceMap;

};
