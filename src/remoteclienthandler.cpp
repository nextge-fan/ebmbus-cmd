#include "remoteclienthandler.h"

RemoteClientHandler::RemoteClientHandler(QObject *parent, QTcpSocket *socket, FFUdatabase *ffuDB) : QObject(parent)
{
    this->socket = socket;
    m_ffuDB = ffuDB;

#ifdef DEBUG
    QString debugStr;

    debugStr += "New connection ";

    if (socket->socketType() == QAbstractSocket::TcpSocket)
        debugStr += "tcp from ";
    else if (socket->socketType() == QAbstractSocket::UdpSocket)
        debugStr += "udp from ";
    else if (socket->socketType() == QAbstractSocket::UnknownSocketType)
        debugStr += " - unknown socket type - ";

    debugStr += socket->peerAddress().toString() + ":" + QString().setNum(socket->peerPort()) + " \r\n";

    printf("%s", debugStr.toLatin1().data());
#endif
    socket->write("Hello\r\n");

    connect(socket, SIGNAL(readyRead()), this, SLOT(slot_read_ready()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
}

void RemoteClientHandler::slot_read_ready()
{
    while (socket->canReadLine())
    {
        // Data format:
        // COMMAND [--key][=value] [--key][=value]...
        QString line = QString::fromUtf8(this->socket->readLine());
        line.remove(QRegExp("[\\r\\n]"));      // Strip newlines at the beginning and at the end
        int commandLength = line.indexOf(' ');
        if (commandLength == -1) commandLength = line.length();
        QString command = line.left(commandLength);
        line.remove(0, commandLength + 1);  // Remove command and optional space

#ifdef DEBUG
        printf("Received data: \r\n");
#endif

        // Parse key/value pairs now
        QStringList commandChunks = line.split(' ', QString::SkipEmptyParts);
        QMap<QString, QString> data;

        foreach(QString commandChunk, commandChunks)
        {
#ifdef DEBUG
            printf("Decoding chunk: %s\r\n", commandChunk.toUtf8().data());
#endif
            QStringList key_value_pair = commandChunk.split('=');
            if (key_value_pair.length() != 2)
            {
                socket->write("ERROR: key_value_pair length invalid\r\n");
                continue;
            }
//            // key and value are base64 encoded.
//            QString key = QString::fromUtf8(QByteArray::fromBase64(key_value_pair.at(0).toUtf8()));
//            QString value = QString::fromUtf8(QByteArray::fromBase64(key_value_pair.at(1).toUtf8()));
            QString key = key_value_pair.at(0);
            key.remove(0, 2);   // Remove leading "--"
            QString value = key_value_pair.at(1);
            data.insert(key, value);
        }

        // message is distributed to other clients in this way
        //emit signal_broadcast(this->socket, line);

        if (command == "help")
        {
            socket->write("This is the commandset of the openFFUcontrol remote unit:\r\n"
                          "\r\n"
                          "<COMMAND> [--key[=value]]\r\n"
                          "\r\n"
                          "COMMANDS:\r\n"
                          "    list\r\n"
                          "        Show the list of currently configured ffus from the controller database.\r\n"
                          "\r\n"
                          "    add-ffu\r\n"
                          "        Add a new ffu to the controller database.\r\n"
                          "\r\n"
                          "    broadcast --bus=BUSNR\r\n"
                          "        Broadcast data to all buses and all units.\r\n"
                          "        Possible keys: speed, ...tbd.r\n"
                          "\r\n"
                          "    dci-address --bus=BUSNR\r\n"
                          "        Start daisy-chain addressing of bus-line BUSNR.\r\n"
                          "\r\n"
                          "    raw-set --bus=BUSNR --KEY=VALUE\r\n"
                          "\r\n"
                          "    raw-get --bus=BUSNR --KEY1 [--KEY2 ...]\r\n");
        }
        else if (command == "list")
        {
            QList<FFU*> ffus = m_ffuDB->getFFUs();
            foreach(FFU* ffu, ffus)
            {
                QString line;

                line.sprintf("FFU id=%i busID=%i rpm=%i\r\n", ffu->getId(), ffu->getBusID(), ffu->getSpeed());

                socket->write(line.toUtf8());
            }
        }
        else if (command == "add-ffu")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

            bool ok;
            QString idString = data.value("id");
            int id = idString.toInt(&ok);
            if (idString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"id\" not specified or id can not be parsed. Abort.\r\n");
                continue;
            }

            socket->write("add-ffu bus=" + bus.toUtf8() + " id=" + QString().setNum(id).toUtf8() + "\r\n");
            QString response = m_ffuDB->addFFU(id);
            socket->write(response.toUtf8());
        }
        else if (command == "broadcast")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

            QString speed = data.value("speed");
            if (speed.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"speed\" not specified. Abort.\r\n");
                continue;
            }

            socket->write("broadcast bus=" + bus.toUtf8() + " speed=" + speed.toUtf8() + "\r\n");
        }
        else if (command == "dci-address")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

            QString startAdr = data.value("startAdr");
            if (startAdr.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"startAdr\" not specified. Abort.\r\n");
                continue;
            }

            socket->write("dci-address bus=" + bus.toUtf8() + " startAdr=" + startAdr.toUtf8() + "\r\n");
        }
        else if (command == "raw-set")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

            socket->write("raw-set bus=" + bus.toUtf8() + "\r\n");
        }
        else if (command == "raw-get")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

            socket->write("raw-get bus=" + bus.toUtf8() + "\r\n");
        }
        else if (command == "set")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString id = data.value("id");
            if (id.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"id\" not specified. Abort.\r\n");
                continue;
            }

            socket->write("set id=" + id.toUtf8() + "\r\n");
        }
        else if (command == "get")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString id = data.value("id");
            if (id.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"id\" not specified. Abort.\r\n");
                continue;
            }

            socket->write("get id=" + id.toUtf8() + "\r\n");
        }
        else
        {
            // If control reaches this point, we have an unsupported command
            socket->write("ERROR: Command not supported: " + command.toUtf8() + "\r\n");
        }
    }
}

void RemoteClientHandler::slot_disconnected()
{
#ifdef DEBUG
    QString debugStr;

    debugStr += "Closed connection ";

    if (socket->socketType() == QAbstractSocket::TcpSocket)
        debugStr += "tcp from ";
    else if (socket->socketType() == QAbstractSocket::UdpSocket)
        debugStr += "udp from ";
    else if (socket->socketType() == QAbstractSocket::UnknownSocketType)
        debugStr += " - unknown socket type - ";

    debugStr += socket->peerAddress().toString() + ":" + QString().setNum(socket->peerPort()) + " \r\n";

    printf("%s", debugStr.toLatin1().data());
#endif
    emit signal_connectionClosed(this->socket, this);
}
