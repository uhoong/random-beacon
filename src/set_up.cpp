#include <iostream>
#include "DrgCore.hpp"
#include "Factory.hpp"
#include <salticidae/util.h>
#include <salticidae/event.h>

std::pair<std::string, std::string> split_ip_port_cport(const std::string &s) {
    auto ret = salticidae::trim_all(salticidae::split(s, ";"));
    if (ret.size() != 2)
        throw std::invalid_argument("invalid cport format");
    return std::make_pair(ret[0], ret[1]);
}

int main(int argc, char **argv)
{
    //读取配置信息
    salticidae::Config config("drg.conf");
    auto opt_replicas = salticidae::Config::OptValStrVec::create();
    auto opt_idx = salticidae::Config::OptValInt::create(0);
    auto opt_pvss_ctx = salticidae::Config::OptValStr::create();
    auto opt_client_port = salticidae::Config::OptValInt::create(-1);
    // auto opt_pvss_dat = salticidae::Config::OptValStr::create();

    config.add_opt("replica", opt_replicas, salticidae::Config::APPEND, 'a', "add an replica to the list");
    config.add_opt("idx", opt_idx, salticidae::Config::SET_VAL, 'i', "specify the index in the replica list");
    config.add_opt("cport", opt_client_port, salticidae::Config::SET_VAL, 'c', "specify the port listening for clients");
    config.add_opt("pvss-ctx", opt_pvss_ctx, salticidae::Config::SET_VAL, 'z', "PVSS ctx");
    // config.add_opt("pvss-dat", opt_pvss_dat, salticidae::Config::SET_VAL, 'D', "PVSS dat");

    salticidae::EventContext ec;
    config.parse(argc, argv);

    auto idx = opt_idx->get();
    auto client_port = opt_client_port->get();
    std::vector<std::string> replicas;
    for (const auto &s: opt_replicas->get())
    {
        auto res = salticidae::trim_all(salticidae::split(s, ","));
        replicas.push_back(res[0]);
    }
    std::string binding_addr = replicas[idx];
    if (client_port == -1)
    {
        auto p = split_ip_port_cport(binding_addr);
        size_t idx;
        client_port = stoi(p.second, &idx);
    }
    salticidae::NetAddr plisten_addr{split_ip_port_cport(binding_addr).first};

    pvss_crypto::initialize();
    auto conf = pvss_crypto::SyncSystemConfig::FromNumReplicas(replicas.size());
    auto factory = pvss_crypto::Factory(std::move(conf));
    std::ifstream ctx_stream;
    ctx_stream.open(opt_pvss_ctx->get());
    if(ctx_stream.fail())
        throw std::runtime_error("PVSS Context File Error!");

    auto pvss_ctx = factory.parseContext(ctx_stream);
    ctx_stream.close();
    
    // ReplicaConfig replica_config(5,2);
    // DrgCore a(pvss_ctx)
    
    // for (const auto &s: opt_replicas->get())
    // {
    //     auto res = trim_all(split(s, ","));
    //     if (res.size() != 3)
    //         throw HotStuffError("invalid replica info");
    //     replicas.push_back(std::make_tuple(res[0], res[1], res[2]));
    // }

    ;

    // a.deliver_share();
}
