#ifndef SESSION_H
#define SESSION_H

#include <vector>
#include <random>
#include <pb_encode.h>
#include <functional>
#include <climits>
#include <algorithm>
#include "Utils.h"
#include "stdlib.h"
#include "ShannonConnection.h"
#include "PlainConnection.h"
#include "PBUtils.h"
#include "Packet.h"
#include "keyexchange.pb.h"
#include "DiffieHellman.h"
#include "authentication.pb.h"

#define SPOTIFY_VERSION 0x10800000000
#define LOGIN_REQUEST_COMMAND 0xAB
#define AUTH_SUCCESSFUL_COMMAND 0xAC
#define AUTH_DECLINED_COMMAND 0xAD



class Session
{
private:
    PlainConnection *conn;
    ShannonConnection *shanConn;
    DiffieHellman *localKeys;
    std::vector<uint8_t> sendClientHelloRequest();
    void processAPHelloResponse(std::vector<uint8_t> &helloPacket);

public:
    Session();
    void connect(PlainConnection *connection);
    void authenticate(std::string login, std::string password);
};

#endif