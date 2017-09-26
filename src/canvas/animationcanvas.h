/*
 * AnimationCanvas - Draws a pattern using the Section's underlying Animation.
 */

#ifndef ANIMATIONCANVAS_H
#define ANIMATIONCANVAS_H

#include "canvas.h"

namespace PixelMaestro {
	class AnimationCanvas : public Canvas {
		public:
			AnimationCanvas(Section* section);
			~AnimationCanvas();
			void activate(unsigned int pixel);
			void deactivate(unsigned int pixel);
			Colors::RGB get_pixel_color(unsigned int pixel);
			CanvasType::Type get_type();
			void initialize_pattern();

		protected:
			/**
				The pattern to display.
				Stored as an array of booleans where 'true' indicates a drawn Pixel.
			*/
			bool* pattern_ = nullptr;
	};
}

#endif // ANIMATIONCANVAS_H