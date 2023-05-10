#include "server.h"

Server::Server() {
    if (this->listen(QHostAddress::Any, 1337)) {
        qDebug() << "start" << '\n';
    } else {
        qDebug() << "error" << '\n';
    }

    next_block_size = 0;
    driver = get_driver_instance();
    con = std::unique_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1", "root", "123"));
    stmt = std::unique_ptr<sql::Statement>(con->createStatement());

    stmt->execute("DROP DATABASE IF EXISTS vk_db");
    stmt->execute("CREATE DATABASE vk_db");
    con->setSchema("vk_db");
    stmt->execute("DROP TABLE IF EXISTS users");
    stmt->execute("CREATE TABLE users(id INT AUTO_INCREMENT PRIMARY KEY, login VARCHAR(20), password VARCHAR(20))");

    std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO users(id, login, password) VALUES (null, ?, ?)"));

    for (int i = 1; i <= 10; i++) {
        pstmt->setString(1, "PETYA_" + std::to_string(i));
        pstmt->setString(2, "1, 2, 3, 4");
        pstmt->executeUpdate();
    }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    sockets.insert(socket);
    qDebug() << "connected " << socketDescriptor << '\n';
}

void Server::slotReadyRead() {
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);
    if (in.status() == QDataStream::Ok) {
        qDebug() << "read... " << '\n';
        while (true) {
             if (next_block_size == 0) {
                 qDebug() << "next_block_size = 0" << '\n';
                 if (socket->bytesAvailable() < 2) {
                     qDebug() << "bytesAvailable < 2, break" << '\n';
                     break;
                 }
                 in >> next_block_size;
                 qDebug() << "next_block_size = " << next_block_size << '\n';
             }

             if (socket->bytesAvailable() < next_block_size) {
                 qDebug() << "bytesAvailable() < next_block_size, break" << '\n';
                 break;
             }

            QString type;
            in >> type;
            if (type == "message") {
                QString string;
                QTime time;
                in >> time >> string;
                next_block_size = 0;
                qDebug() << type << ' ' << time.toString() << ' ' << string << '\n';
                sendToClient(string);
            } else if (type == "auth") {
                QString login, password;
                in >> login >> password;
                next_block_size = 0;
                qDebug() << type << ' ' << login << ' ' << password << '\n';
                std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM users WHERE login = '" + login.toStdString() + "'"));
                std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
                if (!res->next() || res->getString("password") != password.toStdString()) {
                    sendToClientPersoal("Wrong login or password", socket);
                } else {
                    sendToClientPersoal("OK", socket);
                }
                while (res->next())
                    qDebug() << res->getInt("id") << ' ' << res->getInt("id") << '\n';
            }
        }
    } else {
        qDebug() << "DataStream error " << '\n';
    }
}


void Server::sendToClient(QString string) {
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << QString("message") << QTime::currentTime() << string;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    for (auto &client: sockets) {
        client->write(data);
    }
}

void Server::sendToClientPersoal(QString string, QTcpSocket *user_socket) {
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << QString("login") << string;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    user_socket->write(data);
}
