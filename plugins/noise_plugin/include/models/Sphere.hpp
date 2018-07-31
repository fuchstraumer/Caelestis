#pragma once
#ifndef SPHERE_H
#define SPHERE_H

#include "../common/CommonInclude.h"

namespace cnoise {

	namespace models {

		/*
		
			class - Sphere

			Maps 2D noise data to a sphere.
		
		*/

		struct GeoCoord {
			float Lattitude;
			float Longitude;

			// Create geocoord from xy coords at radius r.
			GeoCoord(float x, float y, float r) {

			}
		};

		class Sphere {
		public:
		};

	}

}

#endif // !SPHERE_H
