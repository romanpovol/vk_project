#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    socket->connectToHost("127.0.0.1", 1337);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    next_block_size = 0;
    auth = false;

    while (!auth) {
        MainWindow::on_pushButton_2_clicked();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        qDebug() << auth << '\n';
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_2_clicked()
{
    bool ok_login;
    QString login = QInputDialog::getText(0, "Login",
                                                 "Your login", QLineEdit::Normal,
                                                 "", &ok_login);
    bool ok_password;
    QString password = QInputDialog::getText(0, "Password",
                                                 "Your password", QLineEdit::Normal,
                                                 "", &ok_password);

    if (!ok_login || !ok_password) {
        return;
    }

    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << QString("auth") << login << password;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    socket->write(data);
}

void MainWindow::slotReadyRead()
{
   QDataStream in(socket);
   in.setVersion(QDataStream::Qt_6_2);
   if (in.status() == QDataStream::Ok) {
       while (true) {
            if (next_block_size == 0) {
                if (socket->bytesAvailable() < 2) {
                    break;
                }
                in >> next_block_size;
            }

            if (socket->bytesAvailable() < next_block_size) {
                break;
            }

            QString type;
            in >> type;
            if (type == "message") {
                QString string;
                QTime time;
                in >> time >> string;
                next_block_size = 0;
                ui->textBrowser->append(time.toString() + " " + string);
            } else if (type == "login") {
                QString message;
                in >> message;
                next_block_size = 0;
                if (message == "OK") {
                    auth = true;
                } else {
                    QMessageBox::warning(this, "Warning", "Wrong login or password");
                }
                break;
            }
       }
   } else {
       ui->textBrowser->append("error read");
   }
}

void MainWindow::sendToServer(QString string) {
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << QString("message") << QTime::currentTime() << string;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    socket->write(data);
    ui->lineEdit->clear();
}


void MainWindow::on_pushButton_clicked()
{
    sendToServer(ui->lineEdit->text());
}


void MainWindow::on_lineEdit_returnPressed()
{
    sendToServer(ui->lineEdit->text());
}


