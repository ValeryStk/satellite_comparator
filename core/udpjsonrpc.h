#ifndef UDPJSONRPC_H
#define UDPJSONRPC_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonValue>
#include <functional>

class UdpJsonRpc : public QObject {
    Q_OBJECT
public:
    // localPort — свой порт, peerHost+peerPort — адрес собеседника
    UdpJsonRpc(quint16 localPort,
               const QHostAddress &peerHost, quint16 peerPort,
               QObject *parent = nullptr);

    // регистрируем свои методы (на входящие запросы)
    void registerMethod(const QString &name,
                        std::function<QJsonValue(const QJsonValue&)> handler);

    // вызываем метод на удалённом
    void call(const QString &method, const QJsonValue &params);

signals:
    // получили ответ от remote
    void resultReady(const QJsonValue &result);

private slots:
    void onReadyRead();

private:
    QUdpSocket   m_sock;
    QHostAddress m_peerHost;
    quint16      m_peerPort;
    QHash<QString,
          std::function<QJsonValue(const QJsonValue&)>> m_methods;
};

#endif // UDPJSONRPC_H
