#include "TestEventLoopFinisher.h"
#include <QtCore/QDebug>
#include <QtCore/QTimer>

TestEventLoopFinisher::TestEventLoopFinisher(QEventLoop *loop, int timeout)
  : _loop(loop)
    , _timeoutKill(false)
{
  QTimer::singleShot(timeout, this, SLOT(timeoutTick()));
}

TestEventLoopFinisher::~TestEventLoopFinisher(void)
{
}

bool TestEventLoopFinisher::isKilledByTimeout() const
{
  return this->_timeoutKill;
}

void TestEventLoopFinisher::terminateLoop()
{
  if (this->_loop) {
    QTimer::singleShot(100, this->_loop, SLOT(quit()));
    this->_loop->quit();
  } else {
    qCritical() << "loop was not initialized";
  }
}
 
void TestEventLoopFinisher::timeoutTick()
{
  this->_timeoutKill = true;
  this->terminateLoop();
}

bool TestEventLoopFinisher::setTerminateSignal(QObject *sender, const char* signal)
{
  return QObject::connect(sender, signal, this, SLOT(terminateLoop()));
}
