#include <GameDownloader/ExtractorBase.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    ExtractorBase::ExtractorBase(const QString &extractorId, const QString &fileExtention, QObject *parent)
      : QObject(parent)
      , _extractorId(extractorId)
      , _fileExtention(fileExtention)
    {
    }

    ExtractorBase::~ExtractorBase()
    {
    }

    void ExtractorBase::pauseRequestSlot(ServiceState* state)
    {
      Q_CHECK_PTR(state);
      emit this->pauseRequest(state);
    }

    const QString& ExtractorBase::extractorId() const
    {
      return this->_extractorId;
    }
    const QString& ExtractorBase::fileExtention() const
    {
      return this->_fileExtention;
    }
  }
}