#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <memory>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


class Server : public QTcpServer {
    Q_OBJECT

public:
    Server();
    QTcpSocket *socket;
    sql::Driver *driver;
    std::unique_ptr<sql::Connection> con;
    std::unique_ptr<sql::Statement> stmt;

private:
    QSet <QTcpSocket*> sockets;
    QByteArray data;
    quint16 next_block_size;
    void sendToClient(QString string);
    void sendToClientPersoal(QString string, QTcpSocket *socket);


public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

};

#endif // SERVER_H
