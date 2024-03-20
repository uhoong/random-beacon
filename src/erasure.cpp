#include "erasure.h"
#include <salticidae/stream.h>
#include <salticidae/type.h>

void Chunk::serialize(salticidae::DataStream &s) const
{
    s << salticidae::htole(size);
    s << salticidae::htole((uint32_t)data.size()) << data;
}

void Chunk::unserialize(salticidae::DataStream &s)
{
    uint32_t n;
    s >> n;
    size = salticidae::letoh(n);

    s >> n;
    n = salticidae::letoh(n);

    if (n == 0)
    {
        data.clear();
    }
    else
    {
        auto base = s.get_data_inplace(n);
        data = bytearray_t(base, base + n);
    }
}