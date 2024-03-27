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

    ec.dispatch();
}

void DrgBase::stop(){

}

void DrgBase::share_handler(MsgShare &&msg, const Net::conn_t &conn){
    const salticidae::NetAddr &peer = conn->get_peer_addr();
    if (peer.is_null()) return;
    msg.parse();
    auto &share = msg.share;
    on_receive_share(share);
}