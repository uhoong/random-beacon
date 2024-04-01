#include "DrgBase.hpp"
#include <salticidae/util.h>

const opcode_t MsgStart::opcode;

const opcode_t MsgShare::opcode;
MsgShare::MsgShare(const Share &share) { serialized << share; }
void MsgShare::parse() { serialized >> share; }

DrgBase::DrgBase(ReplicaID rid,
                 // privkey_bt &&priv_key,
                 salticidae::NetAddr listen_addr,
                 const pvss_crypto::Context &pvss_ctx,
                 salticidae::EventContext ec,
                 size_t nworker,
                 const Net::Config &netconfig) : DrgCore(rid, pvss_ctx),
                                                 listen_addr(listen_addr),
                                                 ec(ec),
                                                 tcall(ec),
                                                 pn(ec, netconfig)
{
    /* register the handlers for msg from replicas */
    pn.reg_conn_handler(salticidae::generic_bind(&DrgBase::conn_handler, this, _1, _2));
    pn.reg_handler(salticidae::generic_bind(&DrgBase::share_handler, this, _1, _2));
    pn.reg_handler(salticidae::generic_bind(&DrgBase::start_handler, this, _1, _2));
    pn.start();
    pn.listen(listen_addr);
}

DrgBase::~DrgBase() {}

void DrgBase::stop()
{
}

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

void DrgBase::do_share(const Share &share, ReplicaID dest)
{
    pn.send_msg(MsgShare(share), get_config().get_addr(dest));
}

bool DrgBase::conn_handler(const salticidae::ConnPool::conn_t &conn, bool connected)
{
    if (connected)
    {
        SALTICIDAE_LOG_INFO("connect");
    }
    return true;
}

void DrgBase::start_handler(MsgStart &&msg, const Net::conn_t &conn)
{
    SALTICIDAE_LOG_INFO("addr");
    const salticidae::NetAddr &peer = conn->get_peer_addr();
    if (peer.is_null())
        return;
}

void DrgBase::share_handler(MsgShare &&msg, const Net::conn_t &conn)
{
    const salticidae::NetAddr &peer = conn->get_peer_addr();
    if (peer.is_null())
        return;
    msg.parse();
    auto &share = msg.share;
    on_receive_share(share);
}