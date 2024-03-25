#include "DrgBase.hpp"

bool DrgBase::conn_handler(const salticidae::ConnPool::conn_t &conn, bool connected)
{
    if (connected)
    {
    }
    return true;
}

DrgBase::~DrgBase() {}

void DrgBase::start(std::vector<salticidae::NetAddr> &replicas)
{
    for (size_t i = 0; i < replicas.size(); i++)
    {
        auto &addr = replicas[i];
        DrgCore::add_replica(i, addr);
        if (addr != listen_addr)
        {
            peers.push_back(addr);
            pn.add_peer(addr);
        }
    }

    on_init();

    // if (ec_loop)
    ec.dispatch();
}