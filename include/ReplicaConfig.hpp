#pragma once

#include <vector>
#include <unordered_map>
#include <salticidae/stream.h>
#include "drgerror.hpp"

using ReplicaID = uint16_t;
using opcode_t = uint8_t;

struct ReplicaInfo
{
    ReplicaID id;
    salticidae::NetAddr addr;

    ReplicaInfo(ReplicaID id, const salticidae::NetAddr &addr) : id(id), addr(addr) {}
};

class ReplicaConfig
{
public:
    size_t nreplicas;
    size_t nreconthres;

    std::unordered_map<ReplicaID, ReplicaInfo> replica_map;

    ReplicaConfig(size_t _nreplicas = 0, size_t _nreconthres = 0) : nreplicas(_nreplicas), nreconthres(0) {}

    void add_replica(ReplicaID rid, const ReplicaInfo &info)
    {
        replica_map.insert(std::make_pair(rid, info));
        nreplicas++;
    }

    const ReplicaInfo &get_info(ReplicaID rid) const
    {
        auto it = replica_map.find(rid);
        if (it == replica_map.end())
            throw DrgError("rid %s not found", salticidae::get_hex(rid).c_str());
        return it->second;
    }

    // const PubKey &get_pubkey(ReplicaID rid) const
    // {
    //     return *(get_info(rid).pubkey);
    // }

    const salticidae::NetAddr &get_addr(ReplicaID rid) const
    {
        return get_info(rid).addr;
    }
};