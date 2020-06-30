#include "InterfaceLoader.h"
#include "../Utils/ServiceLocator.h"
#include "../MPQ/MPQLoader.h"
#include "../MPQ/MPQFileJobBatch.h"
#include "../BLP/BLP2PNG/BlpConvert.h"

void InterfaceLoader::LoadInterface()
{
    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    MPQFileJobBatch mpqFileJob;

    mpqLoader->GetFirstFile("Interface\\Glues\\*.blp", [&mpqFileJob](std::string fileName)
        {
            mpqFileJob.AddFileJob(fileName, [fileName](std::shared_ptr<Bytebuffer> buffer)
                {
                    std::filesystem::path outputPath = std::filesystem::current_path().append("ExtractedData/Textures").append(fileName);
                    outputPath = outputPath.make_preferred().replace_extension("dds");

                    std::filesystem::create_directories(outputPath.parent_path());

                    // Convert from BLP to DDS
                    BLP::BlpConvert blpConvert;
                    blpConvert.Convert(buffer->GetDataPointer(), buffer->size, outputPath.string());
                });
        }
    );

    mpqFileJob.Process();
}
