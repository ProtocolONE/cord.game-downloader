#include <GameDownloader/XdeltaWrapper/DecodeStatePrivate.h>
#include <GameDownloader/Common/FileUtils.h>

namespace P1 {
  namespace GameDownloader {
    namespace XdeltaWrapper {

      DecodeStatePrivate::DecodeStatePrivate(QObject *parent /*= 0*/) : QObject(parent)
        , _hasSource(false)
        , _bufferSize(0)
        , _patchFinished(false)
        , _initialized(false)
      {

      }

      DecodeStatePrivate::~DecodeStatePrivate()
      {
        if (this->_initialized) 
          return;

        xd3_close_stream(&this->_stream);
        xd3_free_stream(&this->_stream);
      }

      bool DecodeStatePrivate::init(const QString& sourcePath, const QString& patchPath, const QString& targetPath)
      {
        memset(&this->_stream, 0, sizeof(this->_stream));
        memset(&this->_source, 0, sizeof(this->_source));
        memset(&this->_config, 0, sizeof(this->_config));

        this->_inputFile.setFileName(sourcePath);
        this->_patchFile.setFileName(patchPath);
        this->_targetFile.setFileName(targetPath);

        if (!this->_patchFile.open(QFile::ReadOnly)) {
          DEBUG_LOG << "Fail to open patch file: " << patchPath;
          return false;
        }

        Common::FileUtils::createDirectoryIfNotExist(targetPath);

        if (!this->_targetFile.open(QFile::ReadWrite)) {
          DEBUG_LOG << "Fail to open target file: " << targetPath;
          return false;
        }

        this->_hasSource = QFile::exists(sourcePath);
        if (this->_hasSource && !this->_inputFile.open(QFile::ReadOnly)) {
          DEBUG_LOG << "Fail to open source file: " << sourcePath;
          return false;
        }

        this->_bufferSize = XD3_ALLOCSIZE;

        xd3_init_config(&this->_config, XD3_ADLER32);
        this->_config.winsize = this->_bufferSize;

        int ret = 0;
        if ((ret = xd3_config_stream(&this->_stream, &this->_config)) != 0) {
          DEBUG_LOG << "Failed to init stream" << ret;
          return false;
        }

        this->_initialized = true;

        if (this->_hasSource) {
          this->_source.blksize = this->_bufferSize;
          uint8_t* buffer = new uint8_t[this->_source.blksize];
          this->_sourceBufferPtr.reset(buffer);
          this->_source.curblk = buffer;
          this->_source.onblk = this->_inputFile.read((char*)(this->_source.curblk), this->_source.blksize);
          this->_source.curblkno = 0;
          if ((ret = xd3_set_source_and_size(&this->_stream, &this->_source, this->_inputFile.size())) != 0) {
            DEBUG_LOG << "Failed to init source stream" << ret;
            return false;
          }
        } else {
          this->_source.blksize  = this->_bufferSize;
          this->_source.curblk   = NULL;
          this->_source.max_winsize = this->_bufferSize;
          this->_source.curblkno = 0;
          this->_source.onblk    = 0;
          if ((ret = xd3_set_source_and_size(&this->_stream, &this->_source, 0)) != 0) {
            DEBUG_LOG << "Failed to init source stream" << ret;
            return false;
          }
        }

        this->_patchBufferPtr.reset(new uint8_t[this->_bufferSize]);
        return true;
      }

      void DecodeStatePrivate::close()
      {
        this->_inputFile.close();
        this->_patchFile.close();
        this->_targetFile.close();
      }

    }
  }
}