/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/ExtractorBase.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service>

namespace GGS {
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