#pragma once

#include <GameDownloader/GameDownloader_global.h>

#include <QtCore/QObject>
#include <QtCore/QString>

namespace P1 {
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
        void internalReadPatchFinished(P1::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalReadPatch(P1::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalDecodeInput(P1::GameDownloader::XdeltaWrapper::DecodeState *state);

        void internalFailed(P1::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalFinished(P1::GameDownloader::XdeltaWrapper::DecodeState *state);

        void failed();
        void crcFailed();
        void finished();

      private slots:
        void readPatch(P1::GameDownloader::XdeltaWrapper::DecodeState *state);
        void decodeInput(P1::GameDownloader::XdeltaWrapper::DecodeState *state);

        void internalFailedSlot(P1::GameDownloader::XdeltaWrapper::DecodeState *state);
        void internalFinishedSlot(P1::GameDownloader::XdeltaWrapper::DecodeState *state);
      };

    }
  }
}