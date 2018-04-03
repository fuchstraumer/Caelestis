#pragma once
#ifndef VPSK_PIPELINE_SUBMISSION_HPP
#define VPSK_PIPELINE_SUBMISSION_HPP
#include "PipelineResource.hpp"

namespace vpsk {
       
    /*
        Chose this name because it represents the intent better than "renderpass", as this could be a compute
        pipeline submission outside of a renderpass. Plus, we can have different pipelines in one renderpass 
        so renderpass doesn't really conceptually translate
    */
    class PipelineSubmission {
    public:
    
    private:
    };

}

#endif // !VPSK_PIPELINE_SUBMISSION_HPP
