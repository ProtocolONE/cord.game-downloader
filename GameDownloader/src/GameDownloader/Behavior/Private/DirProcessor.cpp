/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (ñ) 2011 - 2017, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/Behavior/private/DirProcessor.h>
#include <GameDownloader/ServiceState.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QDirIterator>

#include <Windows.h>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      DirProcessor::DirProcessor(QObject * parent /*= 0*/) : QObject(parent)
      {
      }

      DirProcessor::~DirProcessor()
      {

      }

      void DirProcessor::processFolders(const QStringList & folders, GGS::GameDownloader::ServiceState *state) {
        bool messageFired = false;
        bool foundBadFlag = false;

        // If it was already paused
        if (state->state() != GGS::GameDownloader::ServiceState::Started) {
          emit this->resultReady(ReadOnlyBehavior::Paused, state);
          return;
        }

        for (const auto & folder : folders) {

          if (!messageFired) {
            messageFired = true;
            emit this->showMessage(state);
          }

          if (!dropFileFlags(folder, state, foundBadFlag))
            return;
        }

        if (foundBadFlag)
          state->setForceReaload(true);

        emit this->resultReady(ReadOnlyBehavior::Finished, state);
      }


      bool DirProcessor::dropFileFlags(const QString & dirStr, GGS::GameDownloader::ServiceState *state, bool & foundFlag) {

        QDirIterator it(dirStr, QStringList() << "*.*", QDir::Files | QDir::System | QDir::Hidden, QDirIterator::Subdirectories);
        while (it.hasNext()) {

          if (state->state() != GGS::GameDownloader::ServiceState::Started) {
            emit this->resultReady(ReadOnlyBehavior::Paused, state);
            return false;
          }

          std::wstring fName = it.next().toStdWString();

          DWORD currAttr = GetFileAttributes(fName.c_str());
          DWORD newAttr = currAttr;
          DWORD probe = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

          // Drop flag
          if (currAttr & probe) {
            newAttr &= ~probe;
          }

          if (newAttr != currAttr) {
            SetFileAttributes(fName.c_str(), newAttr);
            foundFlag = true;
          }
        }

        return true;
      }

    }
  }
}