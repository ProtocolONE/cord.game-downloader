#include "Program.h"
#include "FakeHook.h"

using namespace GGS::Core;
using namespace GGS::GameDownloader;

Program::Program()
{
  this->setProgress1(0);
  this->setProgress2(0);

  QString root = QCoreApplication::applicationDirPath();
  QString qmlDir = root;
  qmlDir.append("/qml/main.qml");
  QUrl url(qmlDir);
  nQMLContainer = new QDeclarativeView();
  nQMLContainer->rootContext()->setContextProperty("mainWindow", this);
  nQMLContainer->setSource(url);

  nQMLContainer->setAlignment(Qt::AlignCenter);                                                                               
  nQMLContainer->setResizeMode(QDeclarativeView::SizeRootObjectToView);                                                       
  nQMLContainer->showNormal();

  this->initDatabase();
  this->initServices();
  this->_gameDownloaderBuilder.torrentWrapper().setListeningPort(11789);
  QString torrentConfigPath = root;
  torrentConfigPath.append("/torrents");
  this->_gameDownloaderBuilder.torrentWrapper().setTorrentConfigDirectoryPath(torrentConfigPath);
  this->_gameDownloaderBuilder.torrentWrapper().initEngine();

  this->_gameDownloaderBuilder.build();
  this->_gameDownloaderBuilder.gameDownloader().setTimeoutForResume(120);

  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(progressChanged(QString, qint8)), 
    this, SLOT(progressChanged(QString, qint8)));
  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(progressDownloadChanged(QString, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)), 
    this, SLOT(progressDownloadChanged(QString, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)));
  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(progressExtractionChanged(QString, qint8, qint64, qint64)), 
    this, SLOT(progressExtractionChanged(QString, qint8, qint64, qint64)));


  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(started(const GGS::Core::Service *, GGS::GameDownloader::StartType)), 
    this, SLOT(gameDownloaderStarted(const GGS::Core::Service *)));
  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(finished(const GGS::Core::Service *)), 
    this, SLOT(gameDownloaderFinished(const GGS::Core::Service *)));
  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(stopped(const GGS::Core::Service *)), 
    this, SLOT(gameDownloaderStopped(const GGS::Core::Service *)));
  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(stopping(const GGS::Core::Service *)), 
    this, SLOT(gameDownloaderStopping(const GGS::Core::Service *)));
  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(failed(const GGS::Core::Service *)), 
    this, SLOT(gameDownloaderFailed(const GGS::Core::Service *)));


  QObject::connect(&this->_gameDownloaderBuilder.gameDownloader(), SIGNAL(shutdownCompleted()),
    this, SLOT(shutdownCompleted()));
}


Program::~Program()
{
  Q_FOREACH(GGS::Core::Service *service, this->_serviceMap)
    delete service;
}

void Program::setPort(QString port)
{
  if(this->_port != port) {
    this->_port = port;
    emit this->portChanged();
  }
}

void Program::setProgress1(int progress)
{
  if(this->_progress1 != progress) {
    this->_progress1 = progress;
    emit this->progress1Changed();
  }
}

void Program::setProgress2(int progress)
{
  if(this->_progress2 != progress) {
    this->_progress2 = progress;
    emit this->progress2Changed();
  }
}

void Program::setStatusText1(QString status)
{
  if(this->_statusText1 != status) {
    this->_statusText1 = status;
    emit this->statusText1Changed();
  }
}

void Program::setStatusText2(QString status)
{
  if(this->_statusText2 != status) {
    this->_statusText2 = status;
    emit this->statusText2Changed();
  }
}

void Program::setUploadRateLimit(QString bytesPerSecond)
{
  bool ok;
  int q = bytesPerSecond.toInt(&ok);
  if (ok)
    this->_gameDownloaderBuilder.torrentWrapper().setUploadRateLimit(q);
}

void Program::setDownloadRateLimit(QString bytesPerSecond)
{
  bool ok;
  int q = bytesPerSecond.toInt(&ok);
  if (ok)
    this->_gameDownloaderBuilder.torrentWrapper().setDownloadRateLimit(q);
}

void Program::startTorrent(QString id)
{
  Service *service = this->getService(id);
  this->_gameDownloaderBuilder.gameDownloader().start(service, GGS::GameDownloader::Normal);
}

void Program::stopTorrent(QString id)
{
  Service *service = this->getService(id);
  this->_gameDownloaderBuilder.gameDownloader().stop(service);
}

void Program::restartTorrent(QString id)
{
  Service *service = this->getService(id);
  this->_gameDownloaderBuilder.gameDownloader().start(service, GGS::GameDownloader::Force);
}

void Program::recheckTorrent(QString id)
{
  Service *service = this->getService(id);
  this->_gameDownloaderBuilder.gameDownloader().start(service, GGS::GameDownloader::Recheck);
}

void Program::initServices()
{
  this->initService("300002010000000000", "http://fs0.gamenet.ru/update/aika/");
  this->initService("300003010000000000", "http://fs0.gamenet.ru/update/bs/");
  this->initService("300004010000000000", "http://fs0.gamenet.ru/update/rot/");
  this->initService("300005010000000000", "http://fs0.gamenet.ru/update/warinc/");
  this->initService("300006010000000000", "http://fs0.gamenet.ru/update/mw2/");
}

void Program::initService(const QString& id, const QString& torrentUrl)
{
  using namespace GGS::Core;
  Service *service = new Service();
  service->setArea(Service::Live);

  QString root = QCoreApplication::applicationDirPath();
  QString downloadPath = root;
  downloadPath.append(QString::fromLocal8Bit("/game/"));
  downloadPath.append(id);
  downloadPath.append("/");

  QString archive = root;
  archive.append(QString::fromLocal8Bit("/archive/"));
  archive.append(id);
  archive.append("/");

  if (id == "300002010000000000" || id == "300004010000000000") {
    service->setDownloadPath(archive);
    service->setInstallPath(downloadPath);
    service->setTorrentFilePath(archive);
    service->setExtractorType("D9E40EE5-806F-4B7D-8D5C-B6A4BF0110E9");
  } else {
    service->setDownloadPath(downloadPath);
    service->setInstallPath(downloadPath);
    service->setTorrentFilePath(downloadPath);
    service->setExtractorType("3A3AC78E-0332-45F4-A466-89C2B8E8BB9C");
  }

  service->setName(id);
  service->setId(id);
  service->setTorrentUrl(torrentUrl);

  this->_serviceMap[id] = service;

  QStringList args = QCoreApplication::arguments();
  if (!args.contains("/nohook")) {
    FakeHook *hook1 = new FakeHook("hook_1", &this->_gameDownloaderBuilder.gameDownloader());
    hook1->setBeforeProgressWeight(25);
    hook1->setAfterProgressWeight(75);
    this->_gameDownloaderBuilder.gameDownloader().registerHook(id, 100, 100, hook1);

    FakeHook *hook2 = new FakeHook("hook_2", &this->_gameDownloaderBuilder.gameDownloader());
    hook2->setBeforeProgressWeight(75);
    hook2->setAfterProgressWeight(25);
    this->_gameDownloaderBuilder.gameDownloader().registerHook(id, 0, 0, hook2);
  }
}

Service* Program::getService( const QString& id )
{
  if (!this->_serviceMap.contains(id))
    return 0;

  return this->_serviceMap[id];
}

void Program::initDatabase()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  QSqlQuery query;
  QString dbSettingsPath = QString("%1/settings.sql").arg(QCoreApplication::applicationDirPath());

  db.setDatabaseName(dbSettingsPath);
  bool needToSetDefaultSettings = false;

  if (db.open("admin", "admin")) {

    if (!db.tables().contains("app_settings")) {
      query = db.exec("CREATE TABLE app_settings "
        "( "
        "	key_column text NOT NULL, "
        "	value_column text, "
        "	CONSTRAINT app_settings_pk PRIMARY KEY (key_column) "
        ")");
      needToSetDefaultSettings = true;
      qDebug() << "No settings database found, creating...";

      if (query.lastError().isValid())
        qDebug() << query.lastError().text();
    } 
  }

  GGS::Settings::Settings::setConnection(db.connectionName());
}


void Program::torrentChangePort()
{
  qDebug() << "trying to change torrent port to " << this->_port;
  bool ok;
  unsigned short port = this->_port.toInt(&ok);
  if (!ok)
    return;

  this->_gameDownloaderBuilder.torrentWrapper().changeListeningPort(port);
}

void Program::progressChanged( QString serviceId, qint8 progress )
{
  emit this->serviceProgressChanged(serviceId, progress);
  emit this->serviceStatusChanged(serviceId, "WaitPrepairing");
}

void Program::progressDownloadChanged( QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args )
{
  QString str = QString("id %1 progress %2 status %3 d_rate %4 u_rate %5 twd %6 tw %7").arg(args.id()).arg(args.progress()).arg(args.status()).arg(args.downloadRate()).arg(args.uploadRate()).arg(args.totalWantedDone()).arg(args.totalWanted());
  emit this->serviceProgressChanged(serviceId, progress);
  emit this->serviceStatusChanged(serviceId, str);
}

void Program::progressExtractionChanged( QString serviceId, qint8 progress, qint64 current, qint64 total )
{
  emit this->serviceProgressChanged(serviceId, progress);
  char tmp[256];
  sprintf(tmp, "Extracting: %d / %d\0", (int)current, (int)total);
  QString str(tmp);
  emit this->serviceStatusChanged(serviceId, str);

}

void Program::gameDownloaderStarted( const GGS::Core::Service *service )
{
  QString serviceId = service->id();
  emit this->serviceStatusChanged(serviceId, "Started");
}

void Program::gameDownloaderFinished( const GGS::Core::Service *service )
{
  QString serviceId = service->id();
  emit this->serviceStatusChanged(serviceId, "Finished");
}

void Program::gameDownloaderStopped( const GGS::Core::Service *service )
{
  QString serviceId = service->id();
  emit this->serviceStatusChanged(serviceId, "Stopped");
}

void Program::gameDownloaderStopping( const GGS::Core::Service *service )
{
  QString serviceId = service->id();
  emit this->serviceStatusChanged(serviceId, "Stopping");
}

void Program::gameDownloaderFailed( const GGS::Core::Service *service )
{
  QString serviceId = service->id();
  emit this->serviceStatusChanged(serviceId, "Failed");
}

void Program::shutdown()
{
  this->_gameDownloaderBuilder.gameDownloader().shutdown();
}

void Program::shutdownCompleted()
{
  this->_gameDownloaderBuilder.torrentWrapper().shutdown();
  QCoreApplication::quit();
}

void Program::changeDirectory(QString serviceId, QString downloadPath, QString installPath)
{
  GGS::Core::Service *service = this->getService(serviceId);
  if (!service)
    return;

  if (service->hashDownloadPath())
    service->setDownloadPath(downloadPath);
  else
    service->setDownloadPath(installPath);
  
  service->setTorrentFilePath(service->downloadPath());
  service->setInstallPath(installPath);

  this->_gameDownloaderBuilder.gameDownloader().directoryChanged(service);
}
