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

#include <GameDownloader/GameDownloader_global.h>

#include <QtCore/QFile>

#include <xdelta3.h>

namespace GGS {
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