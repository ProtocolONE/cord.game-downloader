/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#ifdef VLD_CHECK_ENABLED
#include <vld.h>
#pragma comment(lib, "vld.lib")
#endif

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QFile>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <gtest/gtest.h>

#include <curl/curl.h>


#include <Settings/Settings>
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

  GGS::Settings::Settings::setConnection(db.connectionName());
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    initDatabase();
    curl_global_init(CURL_GLOBAL_ALL);

    qRegisterMetaType<GGS::GameDownloader::HookBase::HookResult>("GGS::GameDownloader::HookBase::HookResult");
    qRegisterMetaType<GGS::GameDownloader::StartType>("GGS::GameDownloader::StartType");
    qRegisterMetaType<GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType>("GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType");
    qRegisterMetaType<GGS::Libtorrent::EventArgs::ProgressEventArgs>("GGS::Libtorrent::EventArgs::ProgressEventArgs");

    qRegisterMetaType<const GGS::Core::Service *>("const GGS::Core::Service *");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
