/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/GameDownloader_global.h>

#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>
#include <GameDownloader/XdeltaWrapper/DecodeState.h>
#include <GameDownloader/XdeltaWrapper/DecodeStatePrivate.h>

#include <GameDownloader/Common/FileUtils.h>

#include <QtCore/QScopedPointer>
#include <xdelta3.h>

namespace GGS {
  namespace GameDownloader {
    namespace XdeltaWrapper {

      XdeltaDecoder::XdeltaDecoder(QObject *parent) : QObject(parent)
      {
        SIGNAL_CONNECT_CHECK(QObject::connect(
          this, SIGNAL(internalReadPatch(GGS::GameDownloader::XdeltaWrapper::DecodeState*)),
          this, SLOT(readPatch(GGS::GameDownloader::XdeltaWrapper::DecodeState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this, SIGNAL(internalDecodeInput(GGS::GameDownloader::XdeltaWrapper::DecodeState*)),
          this, SLOT(decodeInput(GGS::GameDownloader::XdeltaWrapper::DecodeState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this, SIGNAL(internalReadPatchFinished(GGS::GameDownloader::XdeltaWrapper::DecodeState*)),
          this, SLOT(decodeInput(GGS::GameDownloader::XdeltaWrapper::DecodeState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this, SIGNAL(internalFailed(GGS::GameDownloader::XdeltaWrapper::DecodeState*)),
          this, SLOT(internalFailedSlot(GGS::GameDownloader::XdeltaWrapper::DecodeState *)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this, SIGNAL(internalFinished(GGS::GameDownloader::XdeltaWrapper::DecodeState*)),
          this, SLOT(internalFinishedSlot(GGS::GameDownloader::XdeltaWrapper::DecodeState *)), Qt::QueuedConnection));
      }

      XdeltaDecoder::~XdeltaDecoder()
      {
      }

      XdeltaDecoder::Result XdeltaDecoder::applySync(const QString& sourcePath, const QString& patchPath, const QString& targetPath)
      {
        QFile inputFile(sourcePath);
        QFile patchFile(patchPath);
        QFile targetFile(targetPath);

        if (!patchFile.open(QFile::ReadOnly)) {
          DEBUG_LOG << "Fail to open patch file: " << patchPath;
          return Failed;
        }

        Common::FileUtils::createDirectoryIfNotExist(targetPath);
        if (!targetFile.open(QFile::ReadWrite)) {
          DEBUG_LOG << "Fail to open target file: " << targetPath;
          return Failed;
        }

        bool hasSource = inputFile.exists();
        if (hasSource && !inputFile.open(QFile::ReadOnly)) {
          DEBUG_LOG << "Fail to open source file: " << sourcePath;
          return Failed;
        }

        int bufferSize = XD3_ALLOCSIZE;

        int        ret;
        xd3_stream stream;
        xd3_config config;
        xd3_source source;

        memset(&stream, 0, sizeof (stream));
        memset(&source, 0, sizeof (source));
        memset(&config, 0, sizeof (config));

        xd3_init_config(&config, XD3_ADLER32);
        config.winsize = bufferSize;

        if ((ret = xd3_config_stream(&stream, &config)) != 0) {
          DEBUG_LOG << "Failed to init stream" << ret;
          return Failed;
        }

        QScopedPointer<uint8_t> sourceBufferPtr;

        if (hasSource) {
          source.blksize = bufferSize;
          uint8_t* buffer = new uint8_t[source.blksize];
          sourceBufferPtr.reset(buffer);
          source.curblk = buffer;
          source.onblk = inputFile.read((char*)(source.curblk), source.blksize);
          source.curblkno = 0;
          if ((ret = xd3_set_source(&stream, &source)) != 0) {
            DEBUG_LOG << "Failed to init source stream" << ret;
            xd3_close_stream(&stream);
            xd3_free_stream(&stream);
            return Failed;
          }
        } else {
          source.blksize  = bufferSize;
          source.curblk   = NULL;
          source.max_winsize = bufferSize;
          source.curblkno = 0;
          source.onblk    = 0;
          if ((ret = xd3_set_source_and_size(&stream, &source, 0)) != 0) {
            DEBUG_LOG << "Failed to init source stream" << ret;
            xd3_close_stream(&stream);
            xd3_free_stream(&stream);
            return Failed;
          }
        }

        QScopedPointer<uint8_t> patchBufferPtr(new uint8_t[bufferSize]);

        int patchReaded = 0;
        int r = 0;
        do {
          patchReaded = patchFile.read((char*)patchBufferPtr.data(), bufferSize);
          if (patchReaded < bufferSize)
            xd3_set_flags(&stream, XD3_FLUSH | stream.flags);

          xd3_avail_input(&stream, patchBufferPtr.data(), patchReaded);          
process:  // Кому не нравиться goto предлагаю на досуге самостоятельно поправить так,
          // чтобы было все еще читабельно, красиво, работало правильно и без goto.
          ret = xd3_decode_input(&stream);

          switch (ret)
          {
          case XD3_GOTHEADER:
          case XD3_WINSTART:
          case XD3_WINFINISH:
            goto process;

          case XD3_INPUT:
            continue;

          case XD3_OUTPUT: {
            r = targetFile.write((char *)stream.next_out, stream.avail_out);
            if (r != (int)stream.avail_out) {
              DEBUG_LOG << "Fail to write output";
              xd3_close_stream(&stream);
              xd3_free_stream(&stream);
              return Failed;
            }

            xd3_consume_output(&stream);
            goto process;
          }

          case XD3_GETSRCBLK: {
            if (hasSource) {
              r = inputFile.seek(source.blksize * source.getblkno);
              if (!r) {
                DEBUG_LOG << "Fail to read input";
                xd3_close_stream(&stream);
                xd3_free_stream(&stream);
                return Failed;
              }

              source.onblk = inputFile.read((char*)(source.curblk), source.blksize);
              source.curblkno = source.getblkno;
            }
            goto process;
          }

          case XD3_INVALID_INPUT: // invalid input/decoder error - crc mismatch
            xd3_close_stream(&stream);
            xd3_free_stream(&stream);
            return CrcFailed;
          case XD3_INTERNAL: // internal error
          case XD3_INVALID: // invalid config
          case XD3_NOSECOND: // when secondary compression finds no improvement.
          case XD3_UNIMPLEMENTED: // currently VCD_TARGE
            DEBUG_LOG << "Xdelta decode failed" << ret;
            xd3_close_stream(&stream);
            xd3_free_stream(&stream);
            return Failed;

          default:
            DEBUG_LOG << "Unknown xdelta ret value" << ret;
            xd3_close_stream(&stream);
            xd3_free_stream(&stream);
            return Failed;
          };

        } while(patchReaded == bufferSize);

        xd3_close_stream(&stream);
        xd3_free_stream(&stream);
        return Success;
      }

      void XdeltaDecoder::apply(const QString& source, const QString& patch, const QString& target)
      {
        DecodeState *state = new DecodeState();
        state->_d->init(source, patch, target);
        emit this->internalReadPatch(state);
      }

      void XdeltaDecoder::readPatch(GGS::GameDownloader::XdeltaWrapper::DecodeState *state)
      {
        int patchReaded = state->_d->_patchFile.read(
          reinterpret_cast<char*>(state->_d->_patchBufferPtr.data()),
          state->_d->_bufferSize);

        if (patchReaded < state->_d->_bufferSize)
          xd3_set_flags(&state->_d->_stream, XD3_FLUSH | state->_d->_stream.flags);

        state->_d->_patchFinished = patchReaded != state->_d->_bufferSize;
        xd3_avail_input(&state->_d->_stream, state->_d->_patchBufferPtr.data(), patchReaded);  

        emit this->internalReadPatchFinished(state);
      }

      void XdeltaDecoder::decodeInput(GGS::GameDownloader::XdeltaWrapper::DecodeState *state)
      {
        int r = 0;
        int ret = xd3_decode_input(&state->_d->_stream);
        
        switch (ret)
        {
        case XD3_GOTHEADER:
        case XD3_WINSTART:
        case XD3_WINFINISH:
          emit this->internalDecodeInput(state);
          return;

        case XD3_INPUT:
          if (state->_d->_patchFinished)
            emit this->internalFinished(state);
          else
            emit this->internalReadPatch(state);

          return;

        case XD3_OUTPUT: {
          r = state->_d->_targetFile.write(
            reinterpret_cast<char *>(state->_d->_stream.next_out), 
            state->_d->_stream.avail_out);

          if (r != (int)state->_d->_stream.avail_out) {
            DEBUG_LOG << "Fail to write output";
            emit this->internalFailed(state);
            return;
          }

          xd3_consume_output(&state->_d->_stream);
          emit this->internalDecodeInput(state);
          return;
                         }

        case XD3_GETSRCBLK: {
          if (state->_d->_hasSource) {
            r = state->_d->_inputFile.seek(state->_d->_source.blksize * state->_d->_source.getblkno);
            if (!r) {
              DEBUG_LOG << "Fail to read input";
              emit this->internalFailed(state);
              return;
            }

            state->_d->_source.onblk = state->_d->_inputFile.read(
              reinterpret_cast<char *>(const_cast<uint8_t*>(state->_d->_source.curblk)), 
              state->_d->_source.blksize);

            state->_d->_source.curblkno = state->_d->_source.getblkno;
          }
          
          emit this->internalDecodeInput(state);
          return;
                            }

        case XD3_INVALID_INPUT: // invalid input/decoder error - crc mismatch
            DEBUG_LOG << "Xdelta decode failed" << ret;
            emit this->crcFailed();
            return;
        case XD3_INTERNAL: // internal error
        case XD3_INVALID: // invalid config
        case XD3_NOSECOND: // when secondary compression finds no improvement.
        case XD3_UNIMPLEMENTED: // currently VCD_TARGE
          DEBUG_LOG << "Xdelta decode failed" << ret;
          emit this->internalFailed(state);
          return;

        default:
          DEBUG_LOG << "Unknown xdelta ret value" << ret;
          emit this->internalFailed(state);
          return;
        };
      }

      void XdeltaDecoder::internalFailedSlot(GGS::GameDownloader::XdeltaWrapper::DecodeState *state)
      {
        state->_d->close();
        state->deleteLater();
        emit this->failed();
      }

      void XdeltaDecoder::internalFinishedSlot(GGS::GameDownloader::XdeltaWrapper::DecodeState *state)
      {
        state->_d->close();
        state->deleteLater();
        emit this->finished();
      }
    }
  }
}
