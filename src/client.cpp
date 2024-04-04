#include "client.hpp"
#include <salticidae/util.h>

#include <cstdio>

const opcode_t MsgStart::opcode;

Client::Client(const salticidae::EventContext &ec, salticidae::NetAddr listen_addr) : pn(ec, Net::Config())
{
    pn.reg_conn_handler(salticidae::generic_bind(&Client::conn_handler, this, _1, _2));
    pn.start();
    pn.listen(listen_addr);
}

bool Client::conn_handler(const salticidae::ConnPool::conn_t &conn, bool connected)
{
    std::cout << std::string(conn->get_addr()).c_str() << std::endl;
    return true;
}

void Client::start(const std::vector<std::string> &replicas)
{
    for (size_t i = 0; i < replicas.size(); i++)
    {
        
        auto addr = salticidae::NetAddr(replicas[i]);
        peers.push_back(addr);
        pn.add_peer(addr);
        pn.set_peer_addr(addr, addr);
        pn.conn_peer(addr);
        SALTICIDAE_LOG_INFO("addr %d",i);
    }
}

void Client::send_start()
{
    for (auto addr : peers)
    {
        pn.send_msg(MsgStart(), addr);
    }
}

void Client::add_peer(const std::string &addr)
{
    peers.push_back(salticidae::NetAddr(addr));
    pn.add_peer(salticidae::NetAddr(addr));
    pn.set_peer_addr(salticidae::NetAddr(addr), salticidae::NetAddr(addr));
}

void masksigs()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

int main(int argc, char **argv)
{
    salticidae::Config config("drg.conf");

    auto opt_replicas = salticidae::Config::OptValStrVec::create();
    auto opt_client = salticidae::Config::OptValStr::create("127.0.0.1:30000");
    auto opt_nworker = salticidae::Config::OptValInt::create(1);

    config.add_opt("replica", opt_replicas, salticidae::Config::APPEND, 'a', "add an replica to the list");
    config.add_opt("client", opt_client, salticidae::Config::SET_VAL, 'c', "client");
    config.add_opt("nworker", opt_nworker, salticidae::Config::SET_VAL, 'n', "the number of threads for verification");
    config.parse(argc, argv);
    std::vector<std::string> replicas;
    for (const auto &s : opt_replicas->get())
    {
        replicas.push_back(s);
    }
    auto client = opt_client->get();

    salticidae::EventContext ec;
    Client c(ec, salticidae::NetAddr(client));
    c.start(replicas);
    c.send_start();
    ec.dispatch();
}