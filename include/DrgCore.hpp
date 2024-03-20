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


/** Abstraction for proposal messages. */
struct Share : public salticidae::Serializable
{
    ReplicaID replicaID;
    uint32_t idx;
    uint32_t round;
    chunk_t chunk;
    uint256_t merkle_root;
    bytearray_t merkle_proof;

    Share() : chunk(nullptr) {}
    Share(ReplicaID replicaID,
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
    int id;

    std::unique_ptr<unsigned char[]> random;
    unsigned long long random_len;

    int round;
    std::unique_ptr<unsigned char[]> difficulty;

    std::unique_ptr<unsigned char[]> vrfpk;
    std::unique_ptr<unsigned char[]> vrfsk;

    pvss_crypto::Context pvss_context;

    ReplicaConfig config;

    std::unordered_map<ReplicaID, unordered_map<uint32_t, chunk_t>> shares_matrix;
    std::unordered_map<ReplicaID,pvss_crypto::pvss_sharing_t> shares_map;

public:
    DrgCore(pvss_crypto::Context &pvss_ctx) : pvss_context(pvss_ctx)
    {
        round = 0;

        random_len = 32;
        random = std::make_unique<unsigned char[]>(random_len);
        randombytes(random.get(), random_len);

        difficulty = std::make_unique<unsigned char[]>(crypto_vrf_outputbytes());
        difficulty[0] = 128;
        for (int i = 1; i < crypto_vrf_outputbytes(); i++)
        {
            difficulty[i] = 0;
        }

        vrfpk = std::make_unique<unsigned char[]>(crypto_vrf_publickeybytes());
        vrfsk = std::make_unique<unsigned char[]>(crypto_vrf_secretkeybytes());
    }

    virtual ~DrgCore()
    {
    }

    /** Call to initialize the protocol, should be called once before all other
     * functions. */
    void on_init();

    /** Add a replica to the current configuration. This should only be called
     * before running HotStuffCore protocol. */
    void add_replica(ReplicaID rid, const salticidae::NetAddr &addr);

    void deliver_share()
    {
        pvss_crypto::pvss_sharing_t sharing = pvss_context.create_sharing();

        std::stringstream ss;
        ss.str(std::string{});
        ss << sharing;

        auto str = ss.str();
        bytearray_t sharingbytes(str.begin(), str.end());

        salticidae::DataStream s;
        s << salticidae::htole((uint32_t)sharingbytes.size()) << sharingbytes;

        chunkarray_t chunk_array = Erasure::encode((int)config.nreconthres, (int)(config.nreplicas - config.nreconthres), 8, s);

        merkle::Tree tree;
        for (int i = 0; i < config.nreplicas; i++)
        {
            tree.insert(chunk_array[i]->get_data());
        }

        auto root = tree.root();
        bytearray_t bt;
        root.serialise(bt);
        uint256_t hash(bt);

        for (int i = 0; i < config.nreplicas; i++)
        {
            auto path = tree.path(i);
            bytearray_t patharr;
            path->serialise(patharr);
            if (i != id)
            {
                // Echo echo(id, (uint32_t)i, prop.view, (uint32_t)MessageType::PROPOSAL, hash, patharr, chunk_array[i],
                //           create_part_cert(*priv_key, hash, view), this);
                // do_echo(echo, (ReplicaID)i);
            }
            else
            {
                // Echo echo(id, (uint32_t)i, prop.view, (uint32_t)MessageType::PROPOSAL, hash, patharr, chunk_array[i],
                //           create_part_cert(*priv_key, hash, view), this);
                // do_broadcast_echo(echo);
            }
        }
    }

    void on_receive_share(const Share &share)
    {
        uint32_t _round = share.round;

        size_t qsize = shares_matrix[share.replicaID].size();
        if (qsize > config.nreconthres)
            return;

        if (shares_matrix[share.replicaID].find(share.idx) == shares_matrix[share.replicaID].end())
        {
            bytearray_t bt(share.merkle_root);
            merkle::Hash root(bt);
            merkle::Path path(share.merkle_proof);

            if (path.verify(root))
            {
                shares_matrix[share.replicaID][share.idx] = share.chunk;
                qsize++;
            }
        }

        unsigned long chunksize = share.chunk->get_data().size();

        if (qsize == config.nreconthres)
        {
            chunkarray_t arr;
            intarray_t erasures;
            for (int i = 0; i < (int)config.nreconthres; i++)
            {
                arr.push_back(shares_matrix[_round][i]);
            }
            for (int i = (int)config.nreconthres; i < (int)config.nreplicas; i++)
            {
                arr.push_back(new Chunk(share.chunk->get_size(), bytearray_t(chunksize)));
                erasures.push_back(i);
            }
            erasures.push_back(-1);
            salticidae::DataStream d;
            Erasure::decode((int)config.nreconthres, (int)(config.nreplicas - config.nreconthres), 8, arr, erasures, d);
            
            uint32_t n;
            d >> n;
            n = salticidae::letoh(n);
            auto base = d.get_data_inplace(n);
            bytearray_t extra = bytearray_t(base, base + n);
            std::string str1(extra.begin(), extra.end());
            std::stringstream ss1;
            ss1.str(str1);

            pvss_crypto::pvss_sharing_t sharing;
            ss1 >> sharing;
            shares_map[share.replicaID]=sharing;
        }
    }

private:
    void vrf_hash(std::unique_ptr<unsigned char[]> &hash)
    {
        crypto_vrf_keypair(vrfpk.get(), vrfsk.get());

        auto proof = std::make_unique<unsigned char[]>(crypto_vrf_proofbytes());
        crypto_vrf_prove(proof.get(), vrfsk.get(), random.get(), random_len);

        crypto_vrf_proof_to_hash(hash.get(), proof.get());
    }

    void vrf()
    {
        auto hash = std::make_unique<unsigned char[]>(crypto_vrf_outputbytes());
        vrf_hash(hash);
        if (unsigned_char_compare(hash, difficulty, crypto_vrf_outputbytes()))
        {
            deliver_share();
        }
    }
};
