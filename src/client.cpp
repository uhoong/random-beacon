#include "client.hpp"
#include <salticidae/util.h>

#include <cstdio>

const opcode_t MsgStart::opcode;

bool Client::conn_handler(const salticidae::ConnPool::conn_t &conn, bool connected)
{
    std::cout << std::string(conn->get_addr()).c_str() << std::endl;
    return true;
}

void Client::start()
{
    pn.start();
    pn.listen(salticidae::NetAddr("127.0.0.1:20001"));
}

void Client::send_start(int peer_id)
{
    if (peer_id < peers.size())
    {
        SALTICIDAE_LOG_INFO("send");
        pn.send_msg(MsgStart(), peers[peer_id]);
    }
}

void Client::add_peer(const std::string &addr)
{
    peers.push_back(salticidae::NetAddr(addr));
    pn.add_peer(salticidae::NetAddr(addr));
    pn.set_peer_addr(salticidae::NetAddr(addr),salticidae::NetAddr(addr));
}

void masksigs()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

int main()
{
    salticidae::EventContext ec;
    Client c(ec);
    c.start();
    c.add_peer("127.0.0.1:20000");
    c.pn.set_peer_addr(salticidae::NetAddr("127.0.0.1:20000"),salticidae::NetAddr("127.0.0.1:20000"));
    c.pn.conn_peer(salticidae::NetAddr("127.0.0.1:20000"));
    c.send_start(0);
    ec.dispatch();
}