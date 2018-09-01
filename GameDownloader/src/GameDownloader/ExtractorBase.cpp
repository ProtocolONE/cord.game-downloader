#include <GameDownloader/ExtractorBase.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    ExtractorBase::ExtractorBase(const QString &extractorId, QObject *parent)
      : QObject(parent)
      , _extractorId(extractorId)
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

    const QString& ExtractorBase::extractorId()
    {
      return this->_extractorId;
    }
  }
}