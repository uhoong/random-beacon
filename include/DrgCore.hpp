#pragma once
#include <sodium.h>
#include <memory>
#include "pvss.hpp"
#include "merklecpp.h"
#include "erasure.h"
#include <unordered_map>
#include <salticidae/ref.h>
#include <salticidae/netaddr.h>
#include <salticidae/stream.h>
#include "ReplicaConfig.hpp"

struct Start : public salticidae::Serializable
{
    Start(){};
    void serialize(salticidae::DataStream &s) const override
    {
    }
    void unserialize(salticidae::DataStream &s) override
    {
    }
};

struct Share : public salticidae::Serializable
{
    ReplicaID replicaId;
    /** proof of validity for the share */
    // Decryption;
    bytearray_t bt;

    //    Share(): cert(nullptr), hsc(nullptr) {}
    Share() {}
    Share(ReplicaID replicaId, bytearray_t &&bt) : replicaId(replicaId), bt(std::move(bt))
    {
    }

    Share(const Share &other) : replicaId(other.replicaId), bt(std::move(other.bt))
    {
    }

    Share(Share &&other) = default;

    void serialize(salticidae::DataStream &s) const override
    {
        s << replicaId;
        s << salticidae::htole((uint32_t)bt.size()) << bt;
    }

    void unserialize(salticidae::DataStream &s) override
    {
        uint32_t n;
        s >> replicaId;

        s >> n;
        n = salticidae::letoh(n);
        auto base = s.get_data_inplace(n);
        bt = bytearray_t(base, base + n);
    }

    operator std::string() const
    {
        salticidae::DataStream s;
        s << "<share "
          << "rid=" << std::to_string(replicaId);
        return std::move(s);
    }
};

/** Abstraction for proposal messages. */
struct ShareChunk : public salticidae::Serializable
{
    ReplicaID replicaID;
    uint32_t idx;
    uint32_t round;
    chunk_t chunk;
    uint256_t merkle_root;
    bytearray_t merkle_proof;

    ShareChunk() : chunk(nullptr) {}
    ShareChunk(ReplicaID replicaID,
               uint32_t idx,
               uint32_t round,
               uint256_t merkle_root,
               bytearray_t merkle_proof,
               const chunk_t &chunk)
        : replicaID(replicaID),
          round(round),
          idx(idx),
          merkle_root(merkle_root),
          merkle_proof(merkle_proof),
          chunk(chunk)
    {
    }

    void serialize(salticidae::DataStream &s) const override
    {
        s << replicaID << idx << round << merkle_root;
        s << salticidae::htole((uint32_t)merkle_proof.size()) << merkle_proof;
        s << *chunk;
    }

    inline void unserialize(salticidae::DataStream &s) override
    {
        s >> replicaID;
        s >> idx;
        s >> round;
        s >> merkle_root;
        uint32_t n;
        s >> n;
        n = salticidae::letoh(n);
        if (n == 0)
        {
            merkle_proof.clear();
        }
        else
        {
            auto base = s.get_data_inplace(n);
            merkle_proof = bytearray_t(base, base + n);
        }
        Chunk _chunk;
        s >> _chunk;
        chunk = new Chunk(std::move(_chunk));
    }
};

static bool unsigned_char_compare(std::unique_ptr<unsigned char[]> &a, std::unique_ptr<unsigned char[]> &b, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (a[i] > b[i])
        {
            return false;
        }
    }
    return true;
}

class DrgCore
{
    ReplicaID id;

    std::unique_ptr<unsigned char[]> random;
    unsigned long long random_len;

    int round;
    std::unique_ptr<unsigned char[]> difficulty;

    std::unique_ptr<unsigned char[]> vrfpk;
    std::unique_ptr<unsigned char[]> vrfsk;

    pvss_crypto::Context pvss_context;

    ReplicaConfig config;

    std::unordered_map<ReplicaID, unordered_map<uint32_t, chunk_t>> sharechunk_matrix;
    std::unordered_map<ReplicaID, pvss_crypto::pvss_sharing_t> sharing_map;
    std::vector<pvss_crypto::decryption_t> dec_share_vec;

public:
    DrgCore(ReplicaID rid, const pvss_crypto::Context &pvss_ctx);

    virtual ~DrgCore()
    {
    }

    /** Call to initialize the protocol, should be called once before all other
     * functions. */
    void on_init();

    /** Add a replica to the current configuration. This should only be called
     * before running HotStuffCore protocol. */
    void add_replica(ReplicaID rid, const salticidae::NetAddr &addr);
    const ReplicaConfig &get_config() { return config; }

    void deliver_chunk();

    void on_receive_shareChunk(const ShareChunk &sharechunk);
    void on_receive_share(const Share &share);
    void on_receive_start();

    virtual void do_sharechunk(const ShareChunk &sharechunk, ReplicaID dest) = 0;
    virtual void do_broadcast_sharechunk(const ShareChunk &sharechunk) = 0;
    virtual void do_share(const Share &share, ReplicaID dest) = 0;
    virtual void do_broadcast_share(const Share &share) = 0;

private:
    void vrf_hash(std::unique_ptr<unsigned char[]> &hash);

    void vrf();
};
