#include "DrgCore.hpp"

void DrgCore::add_replica(ReplicaID rid, const salticidae::NetAddr &addr) {
    config.add_replica(rid, ReplicaInfo(rid, addr));
}

/*** end DrgCore protocol logic ***/
void DrgCore::on_init() {
    config.nreconthres = (size_t) floor(config.nreplicas/4.0) + 1;
}