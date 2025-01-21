#define _USE_MATH_DEFINES
#define OPENEXR_DLL

#include <fstream>
#include <cfloat>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>

int main(int argc, char **argv)
{
    // check arg
    if (argc < 3)
    {
        std::cout << "Usage : vdb2raw <input_path> <output_path>" << std::endl;
        return 0;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    // open vdb file
    openvdb::initialize();
    openvdb::io::File inputFile(inputPath);
    if (!inputFile.open())
    {
        std::cout << "Can't open " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    // loop over all grids
    for (openvdb::io::File::NameIterator nameIter = inputFile.beginName(); nameIter != inputFile.endName(); ++nameIter)
    {
        // open the grid
        std::string name = nameIter.gridName();
        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(inputFile.readGrid(name));

        // if (grid->metaValue<std::string>("value_type") != "float") // only export float grid
        //     continue;

        std::cout << "Converting grid: " << name << std::endl;
        // show metadata
        for (openvdb::MetaMap::MetaIterator iter = grid->beginMeta(); iter != grid->endMeta(); ++iter)
        {
            const std::string &name = iter->first;
            openvdb::Metadata::Ptr value = iter->second;
            std::string valueAsString = value->str();
            std::cout << name << " = " << valueAsString << " | typename : " << value->typeName() << std::endl;
        }

        // get bbox size
        openvdb::CoordBBox bbox = grid->evalActiveVoxelBoundingBox();
        openvdb::Vec3i min = bbox.min().asVec3i();
        openvdb::Vec3i max = bbox.max().asVec3i();
        int size[3];
        for (int i = 0; i < 3; i++)
            size[i] = (max[i] - min[i]) + 1;
        int nbVoxels = size[0] * size[1] * size[2];

        // remove ".raw" extension if present in outputPath
        if (outputPath.find(".raw") != std::string::npos)
            outputPath = outputPath.substr(0, outputPath.find(".raw"));
        // add dims to output filename
        outputPath = outputPath + "_" + std::to_string(size[0]) + "x" + std::to_string(size[1]) + "x" + std::to_string(size[2]) + ".raw";

        std::ofstream outputFile(outputPath, std::ofstream::binary);

        // open output file and write header ( width, height and depth in int)
        outputFile.write((const char *)&size, sizeof(int) * 3);

        // write data
        openvdb::tools::GridSampler<openvdb::FloatGrid, openvdb::tools::BoxSampler> sampler(*grid);
        float vmin = FLT_MAX, vmax = -FLT_MAX;
        int progress = 0;
        for (int z = min[2]; z <= max[2]; z++)
        {
            for (int y = min[1]; y <= max[1]; y++)
            {
                for (int x = min[0]; x <= max[0]; x++)
                {
                    float v = sampler.isSample(openvdb::Vec3R(x, y, z));
                    outputFile.write((const char *)&v, sizeof(float));

                    vmin = vmin < v ? vmin : v;
                    vmax = vmax > v ? vmax : v;
                    progress++;
                }
            }
            std::cout << "\r" << "-> " << outputPath << " : " << std::setprecision(5) << float(progress) / nbVoxels * 100.f << " %        ";
        }
        std::cout << std::endl;

        outputFile.close();

        std::cout << "value min/max : " << vmin << " / " << vmax << std::endl;
    }

    inputFile.close();
    return EXIT_SUCCESS;
}
