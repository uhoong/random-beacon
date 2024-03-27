#pragma once

#include "DrgCore.hpp"
#include <salticidae/network.h>
#include <salticidae/netaddr.h>

using salticidae::_1;
using salticidae::_2;

struct MsgShare
{
    static const opcode_t opcode = 0x1;
    salticidae::DataStream serialized;
    Share share;
    MsgShare(const Share &) { serialized << share; }
    MsgShare(salticidae::DataStream &&s) : serialized(std::move(s)) {}
    void parse() { serialized >> share; }
};

class DrgBase : public DrgCore
{

public:
    using Net = salticidae::PeerNetwork<opcode_t>;

protected:
    /** the binding address in replica network */
    salticidae::NetAddr listen_addr;
    /** libevent handle */
    salticidae::EventContext ec;
    salticidae::ThreadCall tcall;
    std::vector<salticidae::NetAddr> peers;
    /** network stack */
    Net pn;

    void share_handler(MsgShare &&, const Net::conn_t &);
    bool conn_handler(const salticidae::ConnPool::conn_t &, bool);

    // M是源消息，T是打包后的消息
    template <typename T, typename M>
    void _do_broadcast(const T &t)
    {
        pn.multicast_msg(M(t), peers);
    }

    void do_share(const Share &share, ReplicaID dest) override {
        pn.send_msg(MsgShare(share), get_config().get_addr(dest));
    }

public:
    DrgBase(ReplicaID rid,
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
        pn.start();
        pn.listen(listen_addr);
    }

    ~DrgBase();

    void start(std::vector<salticidae::NetAddr> &replicas);

    void stop();
};