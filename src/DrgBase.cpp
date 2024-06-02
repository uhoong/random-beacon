#include "DrgBase.hpp"
#include <salticidae/util.h>

const opcode_t MsgStart::opcode;

const opcode_t MsgShareChunk::opcode;
MsgShareChunk::MsgShareChunk(const ShareChunk &share) { serialized << share; }
void MsgShareChunk::parse() { serialized >> share; }

const opcode_t MsgShare::opcode;
MsgShare::MsgShare(const Share &share) { serialized << share; }
void MsgShare::parse()
{
    serialized >> share;
}

DrgBase::DrgBase(ReplicaID rid,
                 // privkey_bt &&priv_key,
                 salticidae::NetAddr listen_addr,
                 const pvss_crypto::Context &pvss_ctx,
                 salticidae::EventContext ec,
                 size_t nworker,
                 const Net::Config &netconfig) : DrgCore(rid, pvss_ctx),
                                                 listen_addr(listen_addr),
                                                 ec(ec),
                                                //  tcall(ec),
                                                 pn(ec, netconfig),
                                                 rng(std::random_device{}()), distribution(0, 99)
{
    /* register the handlers for msg from replicas */
    pn.reg_conn_handler(salticidae::generic_bind(&DrgBase::conn_handler, this, _1, _2));
    pn.reg_handler(salticidae::generic_bind(&DrgBase::sharechunk_handler, this, _1, _2));
    pn.reg_handler(salticidae::generic_bind(&DrgBase::start_handler, this, _1, _2));
    pn.reg_handler(salticidae::generic_bind(&DrgBase::share_handler, this, _1, _2));
    pn.start();
    pn.listen(listen_addr);
}

DrgBase::~DrgBase() {}

void DrgBase::stop()
{
    ec.stop();
}

void DrgBase::start(std::vector<salticidae::NetAddr> &replicas, const salticidae::NetAddr &client,std::unordered_set<ReplicaID> &replicas_notSharing,std::unordered_set<ReplicaID> &replicas_notForward)
{
    for (size_t i = 0; i < replicas.size(); i++)
    {
        auto &addr = replicas[i];
        DrgCore::add_replica(i, addr);
        if (addr != listen_addr)
        {
            peers.push_back(addr);
            pn.add_peer(addr);
            pn.set_peer_addr(addr, addr);
            pn.conn_peer(addr);
        }
    }

    pn.add_peer(client);
    pn.set_peer_addr(client, client);

    on_init(replicas_notSharing,replicas_notForward);

    ec.dispatch();
}

void DrgBase::do_sharechunk(const ShareChunk &sharechunk, ReplicaID dest)
{
    auto a = MsgShareChunk(sharechunk);
    // SALTICIDAE_LOG_INFO("%d: send sharechunk to %d, sharechunk info from %d to %d,size: %d",int(id),int(dest),int(sharechunk.replicaID),int(sharechunk.idx),MsgShareChunk(sharechunk).serialized.size());
    pn.send_msg(MsgShareChunk(sharechunk), get_config().get_addr(dest));
}

void DrgBase::do_share(const Share &share, ReplicaID dest)
{
    // SALTICIDAE_LOG_INFO("%d: send share to %d, sharechunk info %d",int(id),int(dest),int(share.replicaId));
    pn.send_msg(MsgShare(share), get_config().get_addr(dest));
}

bool DrgBase::conn_handler(const salticidae::ConnPool::conn_t &conn, bool connected)
{
    if (connected)
    {
    }
    return true;
}

void DrgBase::start_handler(MsgStart &&msg, const Net::conn_t &conn)
{
    on_receive_start();
}

void DrgBase::sharechunk_handler(MsgShareChunk &&msg, const Net::conn_t &conn)
{
    
    const salticidae::NetAddr &peer = conn->get_peer_addr();
    if (peer.is_null())
        return;
    msg.parse();
    auto &share = msg.share;
    // SALTICIDAE_LOG_INFO("%d receive sharechunk from %s, sharechunk info from %d to %d",int(id),string(conn->get_peer_addr()).c_str(),int(share.replicaID),int(share.idx));
    on_receive_shareChunk(share);
}

void DrgBase::share_handler(MsgShare &&msg, const Net::conn_t &conn)
{
    const salticidae::NetAddr &peer = conn->get_peer_addr();
    if (peer.is_null())
        return;
    msg.parse();
    auto &share = msg.share;
    // SALTICIDAE_LOG_INFO("%d receive share from %s, share info %d",int(id),string(conn->get_peer_addr()).c_str(),int(share.replicaId));
    on_receive_share(share);
}