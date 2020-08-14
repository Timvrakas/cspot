#include "ShannonConnection.h"

ShannonConnection::ShannonConnection(PlainConnection conn, std::vector<uint8_t> sendKey, std::vector<uint8_t> recvKey) {
    this->apSock = conn.apSock;

    this->sendCipher = new Shannon();
    this->recvCipher = new Shannon();

    // Set initial nonce
    this->sendCipher->nonce(pack<uint32_t>(htonl(0)));
    this->recvCipher->nonce(pack<uint32_t>(htonl(0)));

    // Set keys
    this->sendCipher->key(sendKey);
    this->recvCipher->key(recvKey);
}

void ShannonConnection::sendPacket(uint8_t cmd, std::vector<uint8_t> data) {
    auto rawPacket = this->cipherPacket(cmd, data);

    // Shannon encrypt the packet and write it to sock
    this->sendCipher->encrypt(rawPacket);
    blockWrite(this->apSock, rawPacket);

    // Generate mac
    std::vector<uint8_t> mac(MAC_SIZE);
    this->sendCipher->finish(mac);

    // Update the nonce
	this->sendNonce += 1;
    this->sendCipher->nonce(pack<uint32_t>(htonl(this->sendNonce)));

    // Write the mac to sock
    blockWrite(this->apSock, mac);
}

Packet* ShannonConnection::recvPacket() {
    // Receive 3 bytes, cmd + int16 size
    auto data = blockRead(this->apSock, 3);

    auto packetData = std::vector<uint8_t>();

    auto readSize = ntohs(extract<uint16_t>(data, 1));

    // Read and decode if the packet has an actual body
    if (readSize > 0) {
        packetData = blockRead(this->apSock, readSize);
        this->recvCipher->decrypt(packetData);
    }

    // Read mac
    auto mac = blockRead(this->apSock, MAC_SIZE);

    // Generate mac
    std::vector<uint8_t> mac2(MAC_SIZE);
    this->recvCipher->finish(mac2);

    if (mac != mac2) {
        printf("Shannon read: Mac doesn't match\n");
    }

    // Update the nonce
	this->recvNonce += 1;
    this->recvCipher->nonce(pack<uint32_t>(htonl(this->recvNonce)));

    // data[0] == cmd
    return new Packet(data[0], packetData);
}

std::vector<uint8_t> ShannonConnection::cipherPacket(uint8_t cmd, std::vector<uint8_t> data) {
    // Generate packet structure, [Command] [Size] [Raw data]
    auto sizeRaw = pack<uint16_t>(htons(uint16_t(data.size())));

    sizeRaw.insert(sizeRaw.begin(), cmd);
    sizeRaw.insert(sizeRaw.end(), data.begin(), data.end());

    return sizeRaw;
}