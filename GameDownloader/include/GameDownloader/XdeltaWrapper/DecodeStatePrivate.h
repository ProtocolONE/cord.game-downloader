#pragma once
#include <GameDownloader/GameDownloader_global.h>

#include <QtCore/QFile>

#include <xdelta3.h>

namespace P1 {
  namespace GameDownloader {
    namespace XdeltaWrapper {

      class DecodeStatePrivate : public QObject
      {
      public:
        friend class DecodeState;
        friend class XdeltaDecoder;

        explicit DecodeStatePrivate(QObject *parent = 0);
        ~DecodeStatePrivate();

        bool init(const QString& sourcePath, const QString& patchPath, const QString& targetPath);
        void close();

      private:
        bool _initialized;
        bool _hasSource;
        bool _patchFinished;
        int _bufferSize;

        QFile _inputFile;
        QFile _patchFile;
        QFile _targetFile;

        xd3_stream _stream;
        xd3_config _config;
        xd3_source _source;
        QScopedPointer<uint8_t> _sourceBufferPtr;
        QScopedPointer<uint8_t> _patchBufferPtr;
      };

    }
  }
}