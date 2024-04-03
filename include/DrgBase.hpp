#pragma once

#include "DrgCore.hpp"
#include <salticidae/network.h>
#include <salticidae/netaddr.h>

using salticidae::_1;
using salticidae::_2;

struct MsgStart
{
    static const opcode_t opcode = 0x0;
    salticidae::DataStream serialized;
    MsgStart() {}
    MsgStart(salticidae::DataStream &&s) {}
    void parse();
};

struct MsgShareChunk
{
    static const opcode_t opcode = 0x1;
    salticidae::DataStream serialized;
    ShareChunk share;
    MsgShareChunk(const ShareChunk &);
    MsgShareChunk(salticidae::DataStream &&s) : serialized(std::move(s)) {}
    void parse();
};

struct MsgShare
{
    static const opcode_t opcode = 0x2;
    salticidae::DataStream serialized;
    Share share;
    MsgShare(const Share &);
    MsgShare(salticidae::DataStream &&s) : serialized(std::move(s)) {}
    void parse();
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
    // salticidae::ThreadCall tcall;
    std::vector<salticidae::NetAddr> peers;
    /** network stack */
    Net pn;

    void do_sharechunk(const ShareChunk &sharechunk, ReplicaID dest) override;
    void do_share(const Share &share, ReplicaID dest) override;

    void sharechunk_handler(MsgShareChunk &&, const Net::conn_t &);
    void start_handler(MsgStart &&, const Net::conn_t &);
    bool conn_handler(const salticidae::ConnPool::conn_t &, bool);

    // // T是源消息，M是打包后的消息
    // template <typename T, typename M>
    // void _do_broadcast(const T &t)
    // {
    //     pn.multicast_msg(M(t), peers);
    // }

    void do_broadcast_share(const Share &share) override
    {
        for (auto &i : peers)
        {
            pn.send_msg(MsgShare(share), i);
        }
    }

    void do_broadcast_sharechunk(const ShareChunk &sharechunk) override
    {
        for (auto &i : peers)
        {
            pn.send_msg(MsgShareChunk(sharechunk), i);
        }
    }

public:
    DrgBase(ReplicaID rid,
            // privkey_bt &&priv_key,
            salticidae::NetAddr listen_addr,
            const pvss_crypto::Context &pvss_ctx,
            salticidae::EventContext ec,
            size_t nworker,
            const Net::Config &netconfig);

    ~DrgBase();

    void start(std::vector<salticidae::NetAddr> &replicas, const salticidae::NetAddr &client);

    void stop();
};