#include "MapObjectGroup.h"

#include <Utils/StringUtils.h>
#include <Containers/StringTable.h>

#include "../Root/MapObjectRoot.h"
#include "../../../../MAP/MapObject.h"

#include <fstream>
#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

vec2 OctNormalWrap(vec2 v)
{
    vec2 wrap;
    wrap.x = (1.0f - glm::abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
    wrap.y = (1.0f - glm::abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
    return wrap;
}

vec2 OctNormalEncode(vec3 n)
{
    n /= (glm::abs(n.x) + glm::abs(n.y) + glm::abs(n.z));

    vec2 wrapped = OctNormalWrap(n);

    vec2 result;
    result.x = n.z >= 0.0f ? n.x : wrapped.x;
    result.y = n.z >= 0.0f ? n.y : wrapped.y;

    result.x = result.x * 0.5f + 0.5f;
    result.y = result.y * 0.5f + 0.5f;

    return result;
}

vec3 OctNormalDecode(vec2 encN)
{
    encN = encN * 2.0f - 1.0f;

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3(encN.x, encN.y, 1.0f - abs(encN.x) - abs(encN.y));
    float t = glm::clamp(-n.z, 0.0f, 1.0f);

    n.x += n.x >= 0.0f ? -t : t;
    n.y += n.y >= 0.0f ? -t : t;

    return normalize(n);
}

void WMO_OBJECT::SaveToDisk(const fs::path& filePath, const WMO_ROOT& root)
{
    ZoneScoped;

    // We want to convert the WMO_OBJECT to a MapObject and save it to disk
    static MapObject* mapObjectTemplate = new MapObject(); // Don't change this one, we will use it in a memcpy to "reset" object
    thread_local MapObject* mapObject = new MapObject();

    memcpy(mapObject, mapObjectTemplate, sizeof(MapObject));

    // Fix vertex colors
    if (mocv.data.size() > 0 && !mocv.data[0].isAlphaOnly)
    {
        if (mocv.data[0].vertexColors.size() > 0)
        {
            u32 endOfTransBatch = 0;

            if (mogp.transBatchCount > 0)
            {
                endOfTransBatch = moba.data[mogp.transBatchCount - 1u].lastVertex + 1;
            }

            bool hasOutdoorsGroup = root.mohd.flags & 0x08;
            if (hasOutdoorsGroup)
            {
                for (u32 i = endOfTransBatch; i < mocv.data[0].vertexColors.size(); i++)
                {
                    mocv.data[0].vertexColors[i].a = mogp.flags & 0x08 ? 255 : 0;
                }
            }
            else
            {
                // First batch
                for (u32 i = 0; i < endOfTransBatch; i++)
                {
                    f32 b = static_cast<f32>(mocv.data[0].vertexColors[i].b) / 255.0f;
                    f32 g = static_cast<f32>(mocv.data[0].vertexColors[i].g) / 255.0f;
                    f32 r = static_cast<f32>(mocv.data[0].vertexColors[i].r) / 255.0f;
                    f32 a = static_cast<f32>(mocv.data[0].vertexColors[i].a) / 255.0f;

                    f32 ambientB = static_cast<f32>(root.mohd.ambientColor.b) / 255.0f;
                    f32 ambientG = static_cast<f32>(root.mohd.ambientColor.g) / 255.0f;
                    f32 ambientR = static_cast<f32>(root.mohd.ambientColor.r) / 255.0f;

                    // Subtract ambientColor.bgr
                    b -= ambientB;
                    g -= ambientG;
                    r -= ambientR;

                    b = b * (1.0f - a);
                    g = g * (1.0f - a);
                    r = r * (1.0f - a);

                    // Apply it back clamped
                    mocv.data[0].vertexColors[i].b = static_cast<u8>(Math::Clamp(b * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].g = static_cast<u8>(Math::Clamp(g * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].r = static_cast<u8>(Math::Clamp(r * 255.0f, 0.0f, 255.0f));
                }

                // The second two batches
                for (u32 i = endOfTransBatch; i < mocv.data[0].vertexColors.size(); i++)
                {
                    f32 b = static_cast<f32>(mocv.data[0].vertexColors[i].b) / 255.0f;
                    f32 g = static_cast<f32>(mocv.data[0].vertexColors[i].g) / 255.0f;
                    f32 r = static_cast<f32>(mocv.data[0].vertexColors[i].r) / 255.0f;
                    f32 a = static_cast<f32>(mocv.data[0].vertexColors[i].a) / 255.0f;

                    f32 ambientB = static_cast<f32>(root.mohd.ambientColor.b) / 255.0f;
                    f32 ambientG = static_cast<f32>(root.mohd.ambientColor.g) / 255.0f;
                    f32 ambientR = static_cast<f32>(root.mohd.ambientColor.r) / 255.0f;

                    b += ((b * a / 64.0f) - ambientB);
                    g += ((g * a / 64.0f) - ambientG);
                    r += ((r * a / 64.0f) - ambientR);

                    // Apply it back clamped
                    mocv.data[0].vertexColors[i].b = static_cast<u8>(Math::Clamp(b * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].g = static_cast<u8>(Math::Clamp(g * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].r = static_cast<u8>(Math::Clamp(r * 255.0f, 0.0f, 255.0f));
                }
            }
        }
    }

    // Convert render batches
    for (u32 i = 0; i < moba.data.size(); i++)
    {
        RenderBatch& renderBatch = mapObject->renderBatches.emplace_back();

        renderBatch.startIndex = moba.data[i].startIndex;
        renderBatch.indexCount = moba.data[i].indexCount;
        renderBatch.materialID = moba.data[i].materialId;
    }

    // Convert vertices
    u32 numVertexPositions = static_cast<u32>(movt.vertexPosition.size());
    u32 numVertexNormals = static_cast<u32>(monr.vertexNormals.size());
    assert(numVertexPositions == numVertexNormals); // AFAIK, the number of these should always be the same, if this ever hits talk to Pursche

    u32 numVertices = numVertexPositions;

    for (u32 i = 0; i < numVertices; i++)
    {
        MapObjectVertex& vertex = mapObject->vertices.emplace_back();

        // Position
        vec3 pos = movt.vertexPosition[i];
        vertex.position = vec3(-pos.x, pos.z, -pos.y);

        // Normal
        vec3 normal = monr.vertexNormals[i];
        normal = vec3(-normal.x, normal.z, -normal.y);

        vec2 octNormal = OctNormalEncode(normal);

        vertex.octNormal[0] = static_cast<u8>(glm::round(octNormal.x * 255.0f));
        vertex.octNormal[1] = static_cast<u8>(glm::round(octNormal.y * 255.0f));

        // UV
        if (motv.data.size() > 0)
        {
            vec2 uv = motv.data[0].vertexUVs[i];

            vertex.uv.x = uv.x;
            vertex.uv.y = uv.y;
        }
        if (motv.data.size() > 1)
        {
            vec2 uv = motv.data[1].vertexUVs[i];

            vertex.uv.z = uv.x;
            vertex.uv.w = uv.y;
        }
    }

    // Create a file
    std::ofstream output(filePath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create MapObjectRoot file. Check admin permissions\n");
        return;
    }

    // Write header
    output.write(reinterpret_cast<char const*>(&mapObject->header), sizeof(mapObject->header));

    // Write flags
    output.write(reinterpret_cast<char const*>(&mogp.flags), sizeof(u32));

    // Write number of indices
    u32 numIndices = static_cast<u32>(movi.indices.size());
    output.write(reinterpret_cast<char const*>(&numIndices), sizeof(u32));

    // Write indices
    output.write(reinterpret_cast<char const*>(movi.indices.data()), sizeof(u16) * numIndices);

    // Write number of vertices
    output.write(reinterpret_cast<char const*>(&numVertices), sizeof(u32));

    // Write vertices
    output.write(reinterpret_cast<char const*>(mapObject->vertices.data()), sizeof(MapObjectVertex) * numVertices);

    // Write number of vertex color sets
    u32 numVertexColorSets = static_cast<u32>(mocv.data.size());
    output.write(reinterpret_cast<char const*>(&numVertexColorSets), sizeof(u32));

    for (u32 i = 0; i < numVertexColorSets; i++)
    {
        // Write number of vertex colors
        u32 numVertexColors = static_cast<u32>(mocv.data[i].vertexColors.size());
        output.write(reinterpret_cast<char const*>(&numVertexColors), sizeof(u32));

        // Write vertex colors
        if (numVertexColors > 0)
        {
            assert(numVertexPositions == numVertexColors); // AFAIK, if there are vertex colors there should always be the same number of them as vertex positions, if this ever hits talk to Pursche

            // Write vertexColors
            output.write(reinterpret_cast<char const*>(mocv.data[i].vertexColors.data()), sizeof(IntColor) * numVertexColors);
        }
    }

    // Write number of MOPYData
    u32 numMOPYData = static_cast<u32>(mopy.data.size());
    output.write(reinterpret_cast<char const*>(&numMOPYData), sizeof(u32));

    // Write MOPYData
    if (numMOPYData > 0)
    {
        output.write(reinterpret_cast<char const*>(mopy.data.data()), sizeof(MOPY::MOPYData) * numMOPYData);
    }

    // Write number of render batches
    u32 numRenderBatches = static_cast<u32>(mapObject->renderBatches.size());
    output.write(reinterpret_cast<char const*>(&numRenderBatches), sizeof(u32));

    if (numRenderBatches > 0)
    {
        output.write(reinterpret_cast<char const*>(mapObject->renderBatches.data()), sizeof(RenderBatch) * numRenderBatches);
    }

    output.close();

    mapObject->vertices.clear();
    mapObject->renderBatches.clear();
}