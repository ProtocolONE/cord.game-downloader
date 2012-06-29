#include <MessageAdapterMock.h>
#include <TestEventLoopFinisher.h>

#include <QtCore/QTimer>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QDateTime>

MessageAdapterMock::MessageAdapterMock(QObject *parent)
  : MessageAdapter(parent)
{
}

MessageAdapterMock::~MessageAdapterMock()
{
}

void MessageAdapterMock::show(const QString& title, const QString& text, Message::StandardButtons buttons, Message::Icon icon, int messageId, QObject* reciever, const char* member)
{
  this->_returnButtons[messageId] = Message::Yes; // Эмулируем нажатие кнопки "Yes"
  // Этим извратом имитим событие нажатия для завершения ожидающего eventloop.
  // Если кто найдет способ вызвать слот без коннект/эмита прошу поменять.
  connect(this, SIGNAL(testSignal(int)), reciever, member, Qt::QueuedConnection);
  emit this->testSignal(Message::Yes);
}

void MessageAdapterMock::buttonClicked(int messageId, int id)
{
}