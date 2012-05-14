#include "SignalCounter.h"


SignalCounter::SignalCounter(QObject *parent)
  : QObject(parent)
  , _eventCount(0)
{
}

SignalCounter::~SignalCounter()
{
}

void SignalCounter::eventSlot()
{
  this->_eventCount++;
}

int SignalCounter::eventCount() const
{
  return this->_eventCount;
}
