#ifndef MessageAdapterMock_H
#define MessageAdapterMock_H

#include <Core/UI/MessageAdapter.h>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QPair>

using namespace GGS::Core::UI;

class FakeClass : public QObject
{
  Q_OBJECT
public:
  FakeClass(){}
  ~FakeClass(){}

signals:
  void SomeSignal(int);
  void EventLoopTestSignal();
};

class MessageAdapterMock : public MessageAdapter
{
  Q_OBJECT
  Q_ENUMS(Message::StandardButton)

public:
  MessageAdapterMock(QObject *parent = 0);
  ~MessageAdapterMock();

public slots:
  void show(const QString& title, const QString& text, Message::StandardButtons buttons, Message::Icon icon, int messageId, QObject* reciever, const char* bindFunc);   
  void buttonClicked(int messageId, int id);

signals:
  void testSignal(int buttonId);
};

#endif // MessageAdapterMock_H
