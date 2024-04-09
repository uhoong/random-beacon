#include "DrgCore.hpp"

DrgCore::DrgCore(ReplicaID rid, const pvss_crypto::Context &pvss_ctx) : id(int(rid)), pvss_context(pvss_ctx)
{
    round = 0;

    // random_len = 32;
    // random = std::make_unique<unsigned char[]>(random_len);
    // randombytes(random.get(), random_len);

    // difficulty = std::make_unique<unsigned char[]>(crypto_vrf_outputbytes());
    // difficulty[0] = 128;
    // for (int i = 1; i < crypto_vrf_outputbytes(); i++)
    // {
    //     difficulty[i] = 0;
    // }

    // vrfpk = std::make_unique<unsigned char[]>(crypto_vrf_publickeybytes());
    // vrfsk = std::make_unique<unsigned char[]>(crypto_vrf_secretkeybytes());
}

void DrgCore::vrf_hash(std::unique_ptr<unsigned char[]> &hash)
{
    // crypto_vrf_keypair(vrfpk.get(), vrfsk.get());

    // auto proof = std::make_unique<unsigned char[]>(crypto_vrf_proofbytes());
    // crypto_vrf_prove(proof.get(), vrfsk.get(), random.get(), random_len);

    // crypto_vrf_proof_to_hash(hash.get(), proof.get());
}

void DrgCore::vrf()
{
    // auto hash = std::make_unique<unsigned char[]>(crypto_vrf_outputbytes());
    // vrf_hash(hash);
    // if (unsigned_char_compare(hash, difficulty, crypto_vrf_outputbytes()))
    // {
    //     deliver_chunk();
    // }
}

void DrgCore::deliver_chunk()
{
    SALTICIDAE_LOG_INFO("start");
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
            ShareChunk shareChunk(id, (uint32_t)i, 0, hash, patharr, chunk_array[i]);
            sharechunk_matrix[shareChunk.replicaID][shareChunk.idx] = shareChunk.chunk;
            do_sharechunk(shareChunk, (ReplicaID)i);
        }
    }
    sharing_map[id] = sharing;
}

void DrgCore::on_receive_start()
{
    if (id < config.nreconthres)
    {
        deliver_chunk();
    }
}

void DrgCore::on_receive_shareChunk(const ShareChunk &shareChunk)
{
    uint32_t _round = shareChunk.round;
    if (sharechunk_matrix[shareChunk.replicaID].find(shareChunk.idx) == sharechunk_matrix[shareChunk.replicaID].end())
    {
        do_broadcast_sharechunk(shareChunk);
    }
    size_t qsize = sharechunk_matrix[shareChunk.replicaID].size();
    if (qsize > config.nreconthres)
        return;
    if (sharechunk_matrix[shareChunk.replicaID].find(shareChunk.idx) == sharechunk_matrix[shareChunk.replicaID].end())
    {
        bytearray_t bt(shareChunk.merkle_root);
        merkle::Hash root(bt);
        merkle::Path path(shareChunk.merkle_proof);

        if (path.verify(root))
        {
            sharechunk_matrix[shareChunk.replicaID][shareChunk.idx] = shareChunk.chunk;
            qsize++;
        }
    }
    unsigned long chunksize = shareChunk.chunk->get_data().size();
    if (qsize == config.nreconthres)
    {
       
        chunkarray_t arr;
        intarray_t erasures;
        for(int i=0; i < (int) config.nreplicas; i++){
            if (sharechunk_matrix[shareChunk.replicaID][i]){
                arr.push_back(sharechunk_matrix[shareChunk.replicaID][i]);
            }else{
                arr.push_back(new Chunk(shareChunk.chunk->get_size(), bytearray_t(chunksize)));
                erasures.push_back(i);
            }
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
        sharing_map[shareChunk.replicaID] = sharing;

        // 检查前 nreconthres 个节点的 sharing 是否都已收到
        for (int i = 0; i < (int)config.k; i++)
        {
            if (sharing_map.find((ReplicaID)i) == sharing_map.end())
            {
                return;
            }
        }
        // 如果收到，进行聚合，解密并传播
        vector<pvss_crypto::pvss_sharing_t> sharing_vec;
        vector<size_t> id_vec;
        for (int i = 0; i < (int)config.k; i++)
        {
            sharing_vec.push_back(sharing_map[(ReplicaID)i]);
            id_vec.push_back((size_t)i);
        }
        auto agg = pvss_context.aggregate(sharing_vec, id_vec);
        auto decryption = pvss_context.decrypt(agg);
        std::stringstream ss;
        ss.str(std::string{});
        ss << decryption;

        auto str = ss.str();
        bytearray_t dec_bytes(str.begin(), str.end());
        Share share(id, std::move(dec_bytes));

        on_receive_share(share);
        do_broadcast_share(share);
    }
}

void DrgCore::on_receive_share(const Share &share)
{
    // LOG_PROTO("got %s", std::string(share).c_str());
    // LOG_PROTO("now state: %s", std::string(*this).c_str());
    size_t qsize = dec_share_vec.size();

    if (qsize >= config.nreconthres)
        return;

    std::string str(share.bt.begin(), share.bt.end());
    std::stringstream ss;
    ss.str(str);

    pvss_crypto::decryption_t dec_share;

    ss >> dec_share;

    // ReplicaID proposer = get_proposer(share.view);

    // if (!pvss_context.verify_decryption(agg_queue[proposer], dec_share))
    // {
    //     throw std::runtime_error("Decryption Verification failed in View");
    // }
    dec_share_vec.push_back(dec_share);
    // view_shares[share.view].push_back(dec_share);

    if (qsize + 1 == config.nreconthres)
    {
        // Todo: reconstruct the secret and broadcast it.
        SALTICIDAE_LOG_INFO("reconstruct");
        auto beacon = pvss_context.reconstruct(dec_share_vec);

        // if (!pvss_context.verify_beacon(agg_queue[proposer], beacon))
        // {
        //     throw std::runtime_error("Beacon Verification failed.");
        //     return;
        // }

        std::stringstream ss2;
        ss2.str(std::string{});
        ss2 << beacon;

        auto str = ss2.str();

        bytearray_t beacon_bytes(str.begin(), str.end());
        SALTICIDAE_LOG_INFO("beacon: %d",beacon_bytes.size());

        // Beacon beacon1(id, view, std::move(beacon_bytes), this);
        // do_broadcast_beacon(beacon1);
        // // Not a warning; just using LOG_WARN to print beacon output.
        // LOG_WARN("beacon view %d", view);
        // last_view_shares_received = view;
        // last_view_beacon_received = view;
        // _try_enter_view();
    }
}

void DrgCore::add_replica(ReplicaID rid, const salticidae::NetAddr &addr)
{
    config.add_replica(rid, ReplicaInfo(rid, addr));
}

/*** end DrgCore protocol logic ***/
void DrgCore::on_init()
{
    config.nreconthres = (size_t)floor(config.nreplicas / 3.0) + 1;
    config.k = 5;
}