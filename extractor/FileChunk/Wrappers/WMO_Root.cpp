#include "WMO_ROOT.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <tracy/Tracy.hpp>

#include <Containers/StringTable.h>
#include <Utils/StringUtils.h>

#include "../../Utils/JobBatch.h"
#include "../../Utils/ServiceLocator.h"
#include "../../BLP/BLP2PNG/BlpConvert.h"

#include "../../MPQ/MPQLoader.h"
#include "../../MAP/MapObjectRoot.h"
#include "../../Extractors/TextureExtractor.h"

#include "../../FileChunk/Chunks/WMO/ROOT/MOMT.h"

namespace fs = std::filesystem;

std::string GetTextureNameByOffset(Bytebuffer& byteBuffer, size_t offset)
{
    while (offset < byteBuffer.size)
    {
        char c = byteBuffer.GetDataPointer()[offset];
        if (c != 0)
            break;

        offset++;
    }

    std::string result;
    while (offset < byteBuffer.size)
    {
        char c = byteBuffer.GetDataPointer()[offset];
        if (c == 0)
            break;

        result += c;
        offset++;
    }
    return result;
}

void WMO_ROOT::SaveToDisk(const std::string& fileName)
{
    ZoneScoped;

    // We want to convert the WMO_ROOT to a MapObjectRoot and save it to disk
    static MapObjectRoot* mapObjectRootTemplate = new MapObjectRoot(); // Don't change this one, we will use it in a memcpy to "reset" object
    thread_local MapObjectRoot* mapObjectRoot = new MapObjectRoot();

    memcpy(mapObjectRoot, mapObjectRootTemplate, sizeof(MapObjectRoot));

    Bytebuffer textureNameBuffer(motx.textureNames, motx.size);

    const StringTable& textureStringTable = ServiceLocator::GetTextureExtractor()->GetStringTable();

    std::string textureName;
    fs::path texturePath;

    for (MOMT::MOMTData& momtData : momt.data)
    {
        MapObjectMaterial& material = mapObjectRoot->materials.emplace_back();
        material.materialType = momtData.shader;
        material.transparencyMode = momtData.blendMode;
        material.flags = momtData.flags;

        if ((momtData.textureOffset1 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset1] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset1);
            texturePath.replace_extension("dds");

            std::string texturePathStr = texturePath.string();
            std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

            u32 textureNameIndex = std::numeric_limits<u32>().max();

            u32 textureNameHash = StringUtils::fnv1a_32(texturePathStr.c_str(), texturePathStr.length());
            textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);

            material.textureID[0] = textureNameIndex;
        }
        
        if ((momtData.textureOffset2 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset2] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset2);
            texturePath.replace_extension("dds");

            std::string texturePathStr = texturePath.string();
            std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

            u32 textureNameIndex = std::numeric_limits<u32>().max();

            u32 textureNameHash = StringUtils::fnv1a_32(texturePathStr.c_str(), texturePathStr.length());
            textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);

            material.textureID[1] = textureNameIndex;
        }

        if ((momtData.textureOffset3 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset3] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset3);
            texturePath.replace_extension("dds");

            std::string texturePathStr = texturePath.string();
            std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

            u32 textureNameIndex = std::numeric_limits<u32>().max();

            u32 textureNameHash = StringUtils::fnv1a_32(texturePathStr.c_str(), texturePathStr.length());
            textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);

            material.textureID[2] = textureNameIndex;
        }
    }

    // Create a file
    std::ofstream output(fileName, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create MapObjectRoot file. Check admin permissions\n");
        return;
    }

    // Write header
    output.write(reinterpret_cast<char const*>(&mapObjectRoot->header), sizeof(mapObjectRoot->header));

    // Write number of materials
    u32 numMaterials = static_cast<u32>(mapObjectRoot->materials.size());
    output.write(reinterpret_cast<char const*>(&numMaterials), sizeof(u32));
    
    // Write materials
    if (numMaterials > 0)
    {
        output.write(reinterpret_cast<char const*>(mapObjectRoot->materials.data()), sizeof(MapObjectMaterial) * numMaterials);
    }

    // Write numGroups
    u32 numGroups = mohd.groupsNum;
    output.write(reinterpret_cast<char const*>(&numGroups), sizeof(u32));

    output.close();
}