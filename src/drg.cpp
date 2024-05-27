#include <iostream>
#include "DrgBase.hpp"
#include "Factory.hpp"
#include <salticidae/util.h>
#include <salticidae/event.h>

int main(int argc, char **argv)
{
    // 读取配置信息
    salticidae::Config config("drg.conf");
    auto opt_replicas = salticidae::Config::OptValStrVec::create();
    auto opt_evil_replicas_notSharing = salticidae::Config::OptValStrVec::create();      // 不生成密钥的节点
    auto opt_evil_replicas_notForward = salticidae::Config::OptValStrVec::create();   // 收到消息不转发的节点
    auto opt_client = salticidae::Config::OptValStr::create("127.0.0.1:30000");
    auto opt_idx = salticidae::Config::OptValInt::create(0);
    auto opt_pvss_ctx = salticidae::Config::OptValStr::create();
    auto opt_nworker = salticidae::Config::OptValInt::create(1);
    auto opt_repnworker = salticidae::Config::OptValInt::create(1);
    auto opt_repburst = salticidae::Config::OptValInt::create(100);
    auto opt_help = salticidae::Config::OptValFlag::create(false);
    // auto opt_pvss_dat = salticidae::Config::OptValStr::create();

    config.add_opt("replica", opt_replicas, salticidae::Config::APPEND, 'a', "add an replica to the list");
    config.add_opt("evil-notSharing", opt_evil_replicas_notSharing, salticidae::Config::APPEND, 's', "add a not sharing evil replica to the list");
    config.add_opt("evil-notForward", opt_evil_replicas_notForward, salticidae::Config::APPEND, 'f', "add a not forward evil replica to the list");
    config.add_opt("idx", opt_idx, salticidae::Config::SET_VAL, 'i', "specify the index in the replica list");
    config.add_opt("pvss-ctx", opt_pvss_ctx, salticidae::Config::SET_VAL, 'z', "PVSS ctx");
    config.add_opt("client", opt_client, salticidae::Config::SET_VAL, 'c', "client");
    config.add_opt("nworker", opt_nworker, salticidae::Config::SET_VAL, 'n', "the number of threads for verification");
    config.add_opt("repnworker", opt_repnworker, salticidae::Config::SET_VAL, 'm', "the number of threads for replica network");
    config.add_opt("repburst", opt_repburst, salticidae::Config::SET_VAL, 'b', "");
    config.add_opt("help", opt_help, salticidae::Config::SWITCH_ON, 'h', "show this help info");
    // config.add_opt("pvss-dat", opt_pvss_dat, salticidae::Config::SET_VAL, 'D', "PVSS dat");

    config.parse(argc, argv);

    auto client = opt_client->get();

    auto idx = opt_idx->get();
    std::vector<std::string> replicas;
    for (const auto &s : opt_replicas->get())
    {
        replicas.push_back(s);
    }
    // 加入不生成份额的恶意节点
    std::unordered_set<ReplicaID> replicas_notSharing;
    for (const auto &s : opt_evil_replicas_notSharing->get())
    {
        replicas_notSharing.insert(std::stoi(s));
    }
    // 加入收到份额不转发的恶意节点
    std::unordered_set<ReplicaID> replicas_notForwarding;
    for (const auto &s : opt_evil_replicas_notForward->get())
    {
        replicas_notForwarding.insert(std::stoi(s));
    }
    salticidae::NetAddr plisten_addr{replicas[idx]};

    DrgBase::Net::Config repnet_config;
    repnet_config.max_msg_size(65536);
    pvss_crypto::initialize();
    auto conf = pvss_crypto::SyncSystemConfig::FromNumReplicas(replicas.size());
    auto factory = pvss_crypto::Factory(std::move(conf));
    std::ifstream ctx_stream;
    ctx_stream.open(opt_pvss_ctx->get());
    if (ctx_stream.fail())
        throw std::runtime_error("PVSS Context File Error!");
    auto pvss_ctx = factory.parseContext(ctx_stream);
    ctx_stream.close();

    salticidae::BoxObj<DrgBase> papp = nullptr;

    salticidae::EventContext ec;
    papp = new DrgBase(idx, plisten_addr, pvss_ctx, ec, opt_nworker->get(), repnet_config);
    std::vector<salticidae::NetAddr> reps;
    for (auto &r : replicas)
    {
        reps.push_back(salticidae::NetAddr(r));
    }
    
    auto shutdown = [&](int){ papp->stop(); };
    salticidae::SigEvent ev_sigint(ec, shutdown);
    salticidae::SigEvent ev_sigterm(ec, shutdown);
    ev_sigint.add(SIGINT);
    ev_sigterm.add(SIGTERM);
    
    papp->start(reps, salticidae::NetAddr(client),replicas_notSharing,replicas_notForwarding);
    
    return 0;
}
