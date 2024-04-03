#pragma once
#include <salticidae/netaddr.h>
#include <salticidae/network.h>
#include "DrgBase.hpp"
#include <salticidae/event.h>
#include <vector>

using opcode_t = uint8_t;

class Client
{
    using Net = salticidae::PeerNetwork<opcode_t>;

public:
    std::vector<salticidae::NetAddr> peers;

    Net pn;

    Client(const salticidae::EventContext &ec, salticidae::NetAddr listen_addr);

    bool conn_handler(const salticidae::ConnPool::conn_t &conn, bool connected);

    void start(const std::vector<std::string> &replicas);
    void add_peer(const std::string &addr);
    void send_start();
};