#pragma once
#include <memory>
#include "Model.h"

class AssetImporter
{
public:
    static std::shared_ptr<Model> import(const std::string& filename);
    static std::shared_ptr<Model> importObjPositions(const std::string& filename);
};
