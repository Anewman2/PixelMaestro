#ifndef SPARKLEANIMATION_H
#define SPARKLEANIMATION_H

#include "coloranimation.h"

namespace PixelMaestro {
	class SparkleAnimation : public ColorAnimation {
		public:
			SparkleAnimation(Section *section, Colors::RGB* colors = nullptr, unsigned short num_colors = 0, unsigned char threshold = 50);
			void update();

		private:
			/**
			 * Threshold for activating a Pixel (0 - 100).
			 * The higher the threshold, the less likely a Pixel will be activated.
			 */
			unsigned char threshold_;
	};
}

#endif // SPARKLEANIMATION_H
