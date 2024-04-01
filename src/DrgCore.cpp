#include "DrgCore.hpp"

DrgCore::DrgCore(ReplicaID rid, const pvss_crypto::Context &pvss_ctx) :id(int(rid)), pvss_context(pvss_ctx)
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

void DrgCore::vrf_hash(std::unique_ptr<unsigned char[]> &hash)
{
    crypto_vrf_keypair(vrfpk.get(), vrfsk.get());

    auto proof = std::make_unique<unsigned char[]>(crypto_vrf_proofbytes());
    crypto_vrf_prove(proof.get(), vrfsk.get(), random.get(), random_len);

    crypto_vrf_proof_to_hash(hash.get(), proof.get());
}

void DrgCore::vrf()
{
    auto hash = std::make_unique<unsigned char[]>(crypto_vrf_outputbytes());
    vrf_hash(hash);
    if (unsigned_char_compare(hash, difficulty, crypto_vrf_outputbytes()))
    {
        deliver_share();
    }
}

void DrgCore::deliver_share()
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
            Share share(id, (uint32_t)i, 0, hash, patharr, chunk_array[i]);
            do_share(share, (ReplicaID)i);
        }
    }
}

void DrgCore::on_receive_start(){
    vrf();
}

void DrgCore::on_receive_share(const Share &share)
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
        shares_map[share.replicaID] = sharing;
    }
}

void DrgCore::add_replica(ReplicaID rid, const salticidae::NetAddr &addr)
{
    config.add_replica(rid, ReplicaInfo(rid, addr));
}

/*** end DrgCore protocol logic ***/
void DrgCore::on_init()
{
    config.nreconthres = (size_t)floor(config.nreplicas / 4.0) + 1;
}