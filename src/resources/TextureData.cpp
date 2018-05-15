#include "resources/TextureData.hpp"

namespace vpsk {

    void TextureData::Load(const LoadRequest& req) {
        loadTextureDataFromFile(req.AbsolutePath);
        createCopyInformation();
        updateImageInfo();
        updateViewInfo();
    }

}