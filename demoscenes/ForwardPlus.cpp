#include "scene/BaseScene.hpp"
#include "common/CreateInfoBase.hpp"
#include "core/Instance.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/Allocator.hpp"
#include "resource/Buffer.hpp"
#include "util/UtilitySphere.hpp" 
#include "math/Ray.hpp"
#include "geometries/Skybox.hpp"
#include "geometries/ObjModel.hpp"
#include <memory>

namespace forward_plus {

    using namespace vpr;
    using namespace vpsk;

    class LightBuffers {
    public:

        LightBuffers(const Device* dvc);

        std::unique_ptr<Buffer> Flags;
        std::unique_ptr<Buffer> Bounds;
        std::unique_ptr<Buffer> LightCounts;
        std::unique_ptr<Buffer> LightCountTotal;
        std::unique_ptr<Buffer> LightCountOffsets;
        std::unique_ptr<Buffer> LightList;
        std::unique_ptr<Buffer> LightCountsCompare;
    };
    

}