#include "AssetImporter.h"
#include <unordered_map>
#include <engine/util/util.h>
#include <fstream>
#include <engine/util/Logger.h>

std::shared_ptr<Model> importObj(const std::string& filename)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();

    std::ifstream stream;
    stream.open(filename);

    std::string line;
    model->subMeshes.resize(1);

    auto& subMesh = model->subMeshes[0];
    std::unordered_map<std::string, uint32_t> vertexHashMap;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    while (getline(stream, line))
    {
        std::vector<std::string> c = util::splitWhitespace(line);

        if (c.size() == 0) continue;

        if (c[0].compare("v") == 0)
        {
            positions.push_back(glm::vec3(std::stof(c[1]), std::stof(c[2]), std::stof(c[3])));
        }
        else if (c[0].compare("vt") == 0)
        {
            uvs.push_back(glm::vec2(std::stof(c[1]), std::stof(c[2])));
        }
        else if (c[0].compare("vn") == 0)
        {
            normals.push_back(glm::vec3(std::stof(c[1]), std::stof(c[2]), std::stof(c[3])));
        }
        else if (c[0].compare("f") == 0)
        {
            std::vector<uint32_t> vertexIndices;

            for (size_t i = 1; i < c.size(); ++i)
            {
                auto it = vertexHashMap.find(c[i]);

                if (it != vertexHashMap.end())
                    vertexIndices.push_back(it->second);
                else
                {
                    auto indices = util::split(c[i], '/');
                    subMesh.vertices.push_back(positions[std::stoul(indices[0]) - 1]);
                    if (indices.size() > 1)
                    {
                        subMesh.uvs.push_back(uvs[std::stoul(indices[1]) - 1]);
                        if (indices.size() > 2)
                            subMesh.normals.push_back(normals[std::stoul(indices[2]) - 1]);
                    }

                    vertexIndices.push_back(static_cast<uint32_t>(subMesh.vertices.size() - 1));
                    vertexHashMap[c[i]] = vertexIndices[vertexIndices.size() - 1];
                }
            }

            for (size_t i = 3; i < c.size(); ++i)
            {
                subMesh.indices.push_back(vertexIndices[0]);
                subMesh.indices.push_back(vertexIndices[i - 2]);
                subMesh.indices.push_back(vertexIndices[i - 1]);
            }
        }
    }

    return model;
}

std::shared_ptr<Model> AssetImporter::importObjPositions(const std::string& filename)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();

    std::ifstream stream;
    stream.open(filename);

    std::string line;
    model->subMeshes.resize(1);

    auto& subMesh = model->subMeshes[0];

    while (getline(stream, line))
    {
        std::vector<std::string> c = util::splitWhitespace(line);

        if (c.size() == 0) continue;

        if (c[0].compare("v") == 0)
        {
            subMesh.vertices.push_back(glm::vec3(std::stof(c[1]), std::stof(c[2]), std::stof(c[3])));
        }
        else if (c[0].compare("f") == 0)
        {
            std::vector<uint32_t> vertexIndices;

            for (size_t i = 1; i < c.size(); ++i)
            {
                auto indices = util::split(c[i], '/');
                vertexIndices.push_back(std::stoul(indices[0]) - 1);
            }

            for (size_t i = 2; i < vertexIndices.size(); ++i)
            {
                subMesh.indices.push_back(vertexIndices[0]);
                subMesh.indices.push_back(vertexIndices[i - 1]);
                subMesh.indices.push_back(vertexIndices[i]);
            }
        }
    }

    return model;
}

std::shared_ptr<Model> AssetImporter::import(const std::string& filename)
{
    if (filename.find(".obj") != filename.npos)
        return importObj(filename);

    SHOW_ERROR("Error: Unknown file format " << filename);
    return nullptr;
}
