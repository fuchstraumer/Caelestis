#pragma once
#ifndef CURVE_H
#define CURVE_H
#include "Base.hpp"
/*

    Modifier module - Curve

    An expandable class that curves data. Has an internal vector of control points
    that can be expanded and is used to define the curve. This vector is passed
    to the CUDA kernel.

    NOTE: Unlike other modules, this module MUST be setup before using, if a vector
    of control points is not supplied in the constructor. This is due ot how we have
    to set everything in CUDA up.

*/

namespace cnoise {

    namespace modifiers {

        class Curve : public Module {
        public:

            // Doesn't add any control points. Empty constructor.
            Curve(const size_t& width, const size_t& height);

            // Adds control points from given vector and makes sure kernel is good to go ASAP
            Curve(const size_t& width, const size_t& height, const std::vector<ControlPoint>& init_points);

            // Adds a control point
            void AddControlPoint(const float& input_val, const float& output_val);

            virtual size_t GetSourceModuleCount() const override;

            // Get control points (non-mutable)
            const std::vector<ControlPoint>& GetControlPoints() const;

            // Set control points
            void SetControlPoints(const std::vector<ControlPoint>& pts);

            // Clear control points
            void ClearControlPoints();

            // Generate data by launching the CUDA kernel
            virtual void Generate() override;

        protected:

            // Set if points are added after we already prepared the CUDA array, in which case we
            // need to rebuild all the CUDA data.
            bool update;

            // Control points.
            std::vector<ControlPoint> controlPoints;
        };

    }

}
#endif // !CURVE_H
