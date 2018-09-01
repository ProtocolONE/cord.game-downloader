#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <Core/Service.h>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QFile>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <gtest/gtest.h>

#include <Settings/Settings.h>

void initDatabase() 
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  QSqlQuery query;
  QString dbSettingsPath = QString("%1/settings.sql").arg(QCoreApplication::applicationDirPath());
  if (QFile::exists(dbSettingsPath)) 
    QFile::remove(dbSettingsPath);

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

  P1::Settings::Settings::setConnection(db.connectionName());
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList plugins;
    QString path = QCoreApplication::applicationDirPath();

    plugins << path + "/plugins";
    a.setLibraryPaths(plugins);

    initDatabase();

    qRegisterMetaType<P1::GameDownloader::HookBase::HookResult>("P1::GameDownloader::HookBase::HookResult");
    qRegisterMetaType<P1::GameDownloader::StartType>("P1::GameDownloader::StartType");
    qRegisterMetaType<P1::GameDownloader::CheckUpdateHelper::CheckUpdateType>("P1::GameDownloader::CheckUpdateHelper::CheckUpdateType");
    qRegisterMetaType<P1::Libtorrent::EventArgs::ProgressEventArgs>("P1::Libtorrent::EventArgs::ProgressEventArgs");

    qRegisterMetaType<const P1::Core::Service *>("const P1::Core::Service *");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
