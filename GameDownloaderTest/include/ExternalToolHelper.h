#pragma once
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <Windows.h>

class ExternalTool {
public:

  static QString createCommandLine(const QString &program, const QStringList &arguments)
  {
    QString args;
    if (!program.isEmpty()) {
      QString programName = program;
      if (!programName.startsWith(QLatin1Char('\"')) && !programName.endsWith(QLatin1Char('\"')) && programName.contains(QLatin1Char(' ')))
        programName = QLatin1Char('\"') + programName + QLatin1Char('\"');
      programName.replace(QLatin1Char('/'), QLatin1Char('\\'));

      // add the prgram as the first arg ... it works better
      args = programName + QLatin1Char(' ');
    }

    for (int i=0; i<arguments.size(); ++i) {
      QString tmp = arguments.at(i);
      // Quotes are escaped and their preceding backslashes are doubled.
      tmp.replace(QRegExp(QLatin1String("(\\\\*)\"")), QLatin1String("\\1\\1\\\""));
      if (tmp.isEmpty() || tmp.contains(QLatin1Char(' ')) || tmp.contains(QLatin1Char('\t'))) {
        // The argument must not end with a \ since this would be interpreted
        // as escaping the quote -- rather put the \ behind the quote: e.g.
        // rather use "foo"\ than "foo\"
        int i = tmp.length();
        while (i > 0 && tmp.at(i - 1) == QLatin1Char('\\'))
          --i;
        tmp.insert(i, QLatin1Char('"'));
        tmp.prepend(QLatin1Char('"'));
      }
      args += QLatin1Char(' ') + tmp;
    }
    return args;
  }

  /*!
    \fn static void ExternalTool::startAndWait(const QString& path, const QStringList& args,
      int waitTime)
    \brief Start external process and wait for exit. If test crashed or closed than child process will closed too.
    \author Ilya.Tkachenko
    \date 08.07.2013
    \param path     Full pathname of the file.
    \param args     The arguments.
    \param waitTime Time of the wait.
  */
  static void startAndWait(const QString& path, const QStringList& args, int waitTime) 
  {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb=sizeof(STARTUPINFO);

    QString cmd = createCommandLine(path, args);

    DWORD dwCreationFlags = CREATE_NO_WINDOW;
    dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
    dwCreationFlags |= CREATE_BREAKAWAY_FROM_JOB;

    bool success = CreateProcessW(0, (wchar_t*)cmd.utf16(),
      0, 0, TRUE, dwCreationFlags,
      0,
      0,
      &si, &pi);

    HANDLE syncJob = CreateJobObject(NULL, NULL);
    if (syncJob) {
      JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
      jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
      if (!SetInformationJobObject(syncJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
        qDebug() << "Fail to set information for job";

      if (!AssignProcessToJobObject(syncJob, pi.hProcess))
        qDebug() << "Fail to assign process for job";
    }

    WaitForSingleObject(pi.hProcess, waitTime * 1000);
    CloseHandle(pi.hProcess);
  }
};