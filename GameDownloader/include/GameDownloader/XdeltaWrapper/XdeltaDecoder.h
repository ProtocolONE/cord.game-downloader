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

#include <GameDownloader/GameDownloader_global>

#include <QtCore/QObject>
#include <QtCore/QString>

namespace GGS {
  namespace GameDownloader {
    namespace XdeltaWrapper {
      class DecodeState;
      class DOWNLOADSERVICE_EXPORT XdeltaDecoder : public QObject
      {
        Q_OBJECT
      public:
        enum Result {
          Success = 0,
          CrcFailed = 1,
          Failed = 2
        };

        explicit XdeltaDecoder(QObject *parent = 0);
        ~XdeltaDecoder();

        /*!
          \fn Result XdeltaDecoder::applySync(const QString& source, const QString& patch,
            const QString& target);
          \brief Синхронное применение патча на файл. Алгоритм из примера xdelta.
          \author Ilya.Tkachenko
          \date 22.08.2013
          \param source Полный путь до исходного файла.
          \param patch  Полный путь до файла binnary diff.
          \param target Полный путь до файла результата.
          \return Рузальтат операции.
        */
        Result applySync(const QString& source, const QString& patch, const QString& target);
        void apply(const QString& source, const QString& patch, const QString& target);

      signals:
        void internalReadPatchFinished(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalReadPatch(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalDecodeInput(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);

        void internalFailed(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalFinished(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);

        void failed();
        void crcFailed();
        void finished();

      private slots:
        void readPatch(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);
        void decodeInput(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);

        void internalFailedSlot(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalFinishedSlot(GGS::GameDownloader::XdeltaWrapper::DecodeState *state);
      };

    }
  }
}