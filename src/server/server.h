/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include "nativesocket.h"
#include "protocol.h"
#include "roomconfig.h"

#include <QObject>
#include <QStringList>

class Room;
class ServerPlayer;
class LobbyPlayer;

class Server : public QObject {
    Q_OBJECT

public:
    friend class BanIpDialog;
    enum Role{
        LobbyRole,
        RoomRole
    };

    explicit Server(QObject *parent, Role role = RoomRole);

    Role getRole() const {return role;}
    void broadcastSystemMessage(const QString &msg);

    bool listen() { return server->listen(); }
    bool listen(const QHostAddress &address, ushort port) { return server->listen(address, port); }
    ushort serverPort() const {return server->serverPort(); }
    void daemonize() { server->daemonize(); }

    void connectToLobby();
    Room *createNewRoom(const RoomConfig &config);
    Room *getRoom(int room_id);
    void signupPlayer(ServerPlayer *player);

    void broadcastNotification(QSanProtocol::CommandType command, const QVariant &data = QVariant(), int destination = QSanProtocol::S_DEST_CLIENT);
    void broadcast(const QSanProtocol::AbstractPacket *packet);

    QVariant getRoomList(int page = 0);

protected slots:
    void processNewConnection(ClientSocket *socket);
    void cleanup();

    void processMessage(const QByteArray &message);
    void cleanupRoom();
    void cleanupLobbyPlayer();
    void cleanupRemoteRoom();

protected:
    void notifyClient(ClientSocket *socket, QSanProtocol::CommandType command, const QVariant &arg = QVariant());
    void notifyLobby(QSanProtocol::CommandType command, const QVariant &data = QVariant());

    void processClientSignup(ClientSocket *socket, const QSanProtocol::Packet &signup);
    void processLobbyPacket(const QSanProtocol::Packet &packet);
    void processRoomPacket(ClientSocket *socket, const QSanProtocol::Packet &packet);

    //callbacks for lobby server
    void checkVersion(const QVariant &server_version);

    //callbacks for room servers
    void setupNewRemoteRoom(ClientSocket *socket, const QVariant &data);

    Role role;
    ServerSocket *server;
    QStringList addresses;

    typedef void (Server::*LobbyFunction)(const QVariant &);
    static QHash<QSanProtocol::CommandType, LobbyFunction> lobbyFunctions;

    typedef void (Server::*RoomFunction)(ClientSocket *socket, const QVariant &);
    static QHash<QSanProtocol::CommandType, RoomFunction> roomFunctions;

    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer *> players;
    QMultiHash<QString, QString> name2objname;
    ClientSocket *lobby;

    QList<LobbyPlayer *> lobbyPlayers;
    QMap<ClientSocket *, QVariant> remoteRooms;

private:
    void initLobbyFunctions();
    void initRoomFunctions();

signals:
    void serverMessage(const QString &);
    void newPlayer(ServerPlayer *player);
};

#endif // SERVER_H
