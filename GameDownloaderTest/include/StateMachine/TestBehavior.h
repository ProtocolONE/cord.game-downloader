#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <functional>
#include <QtCore/QDateTime>

using namespace GGS::GameDownloader;
using Behavior::BaseBehavior;

class TestBehavior : public BaseBehavior
{
  Q_OBJECT
public:
  TestBehavior(QObject *parent  = 0) 
    : BaseBehavior(parent)
    , bodyCalled(0)
    , stopCalled(0)
    , _id(0)
  {
  }

  ~TestBehavior(){};

  void setId(int id) {
    this->_id = id;
  }

  void setBody(std::function<void (TestBehavior *behavior, ServiceState *state) > body) {
    this->_body = body;
  }

  void setStop(std::function<void (TestBehavior *behavior, ServiceState *state) > stop) {
    this->_stop = stop;
  }

  virtual void start(ServiceState *state)
  {
    qDebug()  << QDateTime::currentMSecsSinceEpoch() << "TestBehavior Start called" << this->_id;
    bodyCalled++;
    if (this->_body)
      this->_body(this, state);
    else
      this->callFailed(state);
  }

  virtual void stop(ServiceState *state)
  {
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "TestBehavior Stop called" << this->_id;
    stopCalled++;
    if (this->_stop)
      this->_stop(this, state);
  }

  void callNext(int result, ServiceState *state)
  {
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "TestBehavior Next emit" << this->_id;
    emit this->next(result, state);
  }

  void callFailed(ServiceState *state)
  {
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "TestBehavior Failed emit" << this->_id;
    emit this->failed(state);
  }

  void callFinished(ServiceState *state)
  {
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "TestBehavior Finished emit" << this->_id;
    emit this->finished(state);
  }

  int bodyCalled;
  int stopCalled;

private:
  int _id;
  std::function<void (TestBehavior *behavior, ServiceState *state) > _body;
  std::function<void (TestBehavior *behavior, ServiceState *state) > _stop;
};

