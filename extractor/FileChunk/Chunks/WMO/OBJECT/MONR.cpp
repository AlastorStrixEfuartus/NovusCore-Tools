#include "MONR.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

bool MONR::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    size_t num = header.size / sizeof(vec3);
    if (num == 0)
        return true;

    wmoObject.monr.vertexNormals.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        vec3& normal = wmoObject.monr.vertexNormals.emplace_back();
        if (!buffer->Get(normal))
        {
            assert(false);
            return false;
        }
    }

    return true;
}