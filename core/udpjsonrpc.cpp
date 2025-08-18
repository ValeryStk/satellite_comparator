#include "udpjsonrpc.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

UdpJsonRpc::UdpJsonRpc(quint16 localPort,
                       const QHostAddress &peerHost,
                       quint16 peerPort,
                       QObject *parent)
    : QObject(parent),
      m_peerHost(peerHost),
      m_peerPort(peerPort)
{
    m_sock.bind(QHostAddress::LocalHost, localPort);
    connect(&m_sock, &QUdpSocket::readyRead,
            this, &UdpJsonRpc::onReadyRead);

}

void UdpJsonRpc::registerMethod(
        const QString &name,
        std::function<QJsonValue(const QJsonValue&)> handler)
{
    m_methods[name] = handler;
}

void UdpJsonRpc::call(const QString &method,
                      const QJsonValue &params)
{
    QJsonObject req{
        {"jsonrpc","2.0"},
        {"method", method},
        {"params",  params}
    };
    auto data = QJsonDocument(req).toJson(QJsonDocument::Compact) + '\n';
    auto res = m_sock.writeDatagram(data, m_peerHost, m_peerPort);
    qDebug()<<"bytes were sended: "<<res;
}

void UdpJsonRpc::onReadyRead()
{
    while (m_sock.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_sock.pendingDatagramSize());
        QHostAddress sender;
        quint16      senderPort;
        m_sock.readDatagram(datagram.data(), datagram.size(),
                            &sender, &senderPort);

        // разбор JSON
        auto doc = QJsonDocument::fromJson(datagram);
        if (!doc.isObject()) return;
        QJsonObject obj = doc.object();

        // если запрос
        if (obj.contains("method")) {
            QString m = obj.value("method").toString();
            QJsonValue p = obj.value("params");
            QJsonValue res;
            if (m_methods.contains(m))
                res = m_methods[m](p);

        }
        // если ответ
        else if (obj.contains("result")) {
            emit resultReady(obj.value("result"));
        }
    }
}
