/*
	Section.cpp - Class for controlling multiple Pixels.
	Requires Pixel and Colors classes.
*/

#include "../animation/blinkanimation.h"
#include "../animation/cycleanimation.h"
#include "../animation/fireanimation.h"
#include "../animation/lightninganimation.h"
#include "../animation/mandelbrotanimation.h"
#include "../animation/mergeanimation.h"
#include "../animation/plasmaanimation.h"
#include "../animation/radialanimation.h"
#include "../animation/randomanimation.h"
#include "../animation/solidanimation.h"
#include "../animation/sparkleanimation.h"
#include "../animation/waveanimation.h"
#include "../canvas/animationcanvas.h"
#include "../canvas/colorcanvas.h"
#include "../canvas/palettecanvas.h"
#include "../utility.h"
#include "colors.h"
#include "pixel.h"
#include "section.h"

namespace PixelMaestro {

	/**
	 * Constructor. Initializes an empty Section.
	 * @param Maestro The Section's parent Maestro.
	 */
	Section::Section() : Section(0, 0) { }

	/**
	 * Constructor. Initializes the Pixel array.
	 * @param Maestro The Section's parent Maestro.
	 * @param dimensions Initial layout (rows and columns) of the Pixels.
	 * @param parent Parent Section (if this is an Layer).
	 */
	Section::Section(Point dimensions, Section* parent) : Section(dimensions.x, dimensions.y, parent) { }

	/**
	 * Constructor. Initializes the Pixel array.
	 * @param x Number of rows in the Section.
	 * @param y Number of columns in the Section.
	 * @param parent Parent Section (if this is an Layer).
	 */
	Section::Section(uint16_t x, uint16_t y, Section* parent) {
		set_dimensions(x, y);
		parent_section_ = parent;
	}

	/**
		Returns the current animation.

		@return Current animation.
	*/
	Animation* Section::get_animation() const {
		return animation_;
	}

	/**
	 * Returns the Section's Canvas.
	 * @return Section's Canvas.
	 */
	Canvas* Section::get_canvas() const {
		return canvas_;
	}

	/**
		Returns the size of the Pixel grid.

		@return Size of the Pixel grid.
	*/
	Point* Section::get_dimensions() {
		return &dimensions_;
	}

	/**
	 * Returns the Section's parent Maestro.
	 * @return Parent Maestro.
	 */
	Maestro* Section::get_maestro() const {
		return maestro_;
	}

	/**
	 * Returns the Layer (if one exists).
	 *
	 * @return Active Layer.
	 */
	Section::Layer* Section::get_layer() const {
		return layer_;
	}

	/**
	 * Returns the Section's offset.
	 * @return Offset.
	 */
	Point* Section::get_offset() {
		return &offset_;
	}

	/**
	 * Returns this Section's parent (if this is an Layer).
	 * @return Parent Section.
	 */
	Section* Section::get_parent_section() const {
		return parent_section_;
	}

	/**
	 * Returns the Pixel at the specified index.
	 * Used by AnimationCanvases to get underlying color values.
	 * @param x Pixel x coordinate.
	 * @param y Pixel y coordinate.
	 * @return Pixel.
	 */
	Pixel* Section::get_pixel(uint16_t x, uint16_t y) const {
		return &pixels_[dimensions_.get_inline_index(x, y)];
	}

	/**
		Returns the final color of the specified Pixel.

		@param x Pixel x-coordinate.
		@param y Pixel y-coordinate.
		@return RGB value of the Pixel's final color.
	*/
	Colors::RGB Section::get_pixel_color(uint16_t x, uint16_t y) {
		Colors::RGB color;

		// Adjust coordinates based on offet
		uint16_t offset_x = (x + offset_.x) % dimensions_.x;
		uint16_t offset_y = (y + offset_.y) % dimensions_.y;

		// If there's a Canvas, get the color supplied by the Canvas.
		if (canvas_ != nullptr) {
			color = canvas_->get_pixel_color(offset_x, offset_y);
		}
		else {
			color = *pixels_[dimensions_.get_inline_index(offset_x, offset_y)].get_color();
		}

		// If there's a Layer, return the Layer color mixed with the Section (or Canvas) color.
		if (layer_ != nullptr) {
			return Colors::mix_colors(color, layer_->section->get_pixel_color(x, y), layer_->mix_mode, layer_->alpha);
		}

		return color;
	}

	/**
	 * Returns the Section's scrolling behavior.
	 * @return Scrolling behavior.
	 */
	Section::Scroll* Section::get_scroll() const {
		return scroll_;
	}

	/**
	 * Deletes the current Animation.
	 */
	void Section::remove_animation() {
		delete animation_;
		animation_ = nullptr;
	}

	/**
	 * Deletes the current Canvas.
	 */
	void Section::remove_canvas() {
		delete canvas_;
		canvas_ = nullptr;
	}

	/**
	 * Deletes the current Layer.
	 */
	void Section::remove_layer() {
		delete layer_;
		layer_ = nullptr;
	}

	/**
	 * Deletes the Section's scrolling behavior.
	 */
	void Section::remove_scroll() {
		delete scroll_;
		scroll_ = nullptr;
	}

	/**
	 * Sets a new Animation.
	 * This will delete and overwrite an existing Animation.
	 * @param type Animation type.
	 * @param colors The color palette.
	 * @param num_colors The number of colors in the palette.
	 * @param preserve_settings If true, the generic configurations in the old Animation (cycle index, orientation, fade, reverse, speed, and pause) are copied to the new Animation.
	 * @return New Animation.
	 */
	Animation* Section::set_animation(AnimationType animation_type, Colors::RGB *colors, uint8_t num_colors, bool preserve_settings) {
		Animation* animation = nullptr;
		switch(animation_type) {
			case AnimationType::Blink:
				animation = new BlinkAnimation(this, colors, num_colors);
				break;
			case AnimationType::Cycle:
				animation = new CycleAnimation(this, colors, num_colors);
				break;
			case AnimationType::Fire:
				animation = new FireAnimation(this, colors, num_colors);
				break;
			case AnimationType::Lightning:
				animation = new LightningAnimation(this, colors, num_colors);
				break;
			case AnimationType::Mandelbrot:
				animation = new MandelbrotAnimation(this, colors, num_colors);
				break;
			case AnimationType::Merge:
				animation = new MergeAnimation(this, colors, num_colors);
				break;
			case AnimationType::Plasma:
				animation = new PlasmaAnimation(this, colors, num_colors);
				break;
			case AnimationType::Radial:
				animation = new RadialAnimation(this, colors, num_colors);
				break;
			case AnimationType::Random:
				animation = new RandomAnimation(this, colors, num_colors);
				break;
			case AnimationType::Solid:
				animation = new SolidAnimation(this, colors, num_colors);
				break;
			case AnimationType::Sparkle:
				animation = new SparkleAnimation(this, colors, num_colors);
				break;
			case AnimationType::Wave:
				animation = new WaveAnimation(this, colors, num_colors);
				break;
		}

		/*
		 * Check for an existing Animation.
		 * If one exists and preserve_settings is true, copy the old Animation's settings to the new Animation.
		 */
		if (this->animation_) {
			if (preserve_settings) {
				animation->set_cycle_index(this->animation_->get_cycle_index());
				animation->set_fade(this->animation_->get_fade());
				animation->set_orientation(this->animation_->get_orientation());
				animation->set_reverse(this->animation_->get_reverse());

				if (this->animation_->get_timer() != nullptr) {
					animation->set_timer(this->animation_->get_timer()->get_interval(), this->animation_->get_timer()->get_delay());
				}
			}
			remove_animation();
		}

		this->animation_ = animation;
		return animation_;
	}

	/**
	 * Sets a new Canvas of the specified type.
	 * This will overwrite the existing Canvas.
	 * @param type The type of Canvas to create.
	 * @param num_frames The number of frames in the Canvas.
	 * @return The new Canvas.
	 */
	Canvas* Section::set_canvas(CanvasType type, uint16_t num_frames) {
		remove_canvas();

		switch (type) {
			case CanvasType::AnimationCanvas:
				canvas_ = new AnimationCanvas(this, num_frames);
				break;
			case CanvasType::ColorCanvas:
				canvas_ = new ColorCanvas(this, num_frames);
				break;
			case CanvasType::PaletteCanvas:
				canvas_ = new PaletteCanvas(this, num_frames, nullptr, 0);
				break;
		}

		return canvas_;
	}

	/**
	 * Sets the size of the Section.
	 * @param x Number of Pixels along the x-coordinate.
	 * @param y Number of Pixels along the y-coordinate.
	 */
	void Section::set_dimensions(uint16_t x, uint16_t y) {
		dimensions_.x = x;
		dimensions_.y = y;

		// Resize the Pixel grid
		delete [] pixels_;
		pixels_ = new Pixel[dimensions_.size()];

		// Reinitialize the Canvas
		if (canvas_ != nullptr) {
			canvas_->initialize();
		}

		// Reinitialize the Layer
		if (layer_ != nullptr) {
			layer_->section->set_dimensions(dimensions_.x, dimensions_.y);
		}
	}

	/**
	 * Sets a new Layer.
	 * If an Layer already exists, this updates the existing Layer.
	 * @param mix_mode The method for blending the Layer.
	 * @param alpha The Layer's transparency (0 - 255.
	 * @return New Layer.
	 */
	Section::Layer* Section::set_layer(Colors::MixMode mix_mode, uint8_t alpha) {
		if (layer_ == nullptr) {
			layer_ = new Layer(this, mix_mode, alpha);
		}
		else {
			layer_->mix_mode = mix_mode;
			layer_->alpha = alpha;
		}

		return layer_;
	}

	/**
	 * Sets the Section's parent Maestro.
	 * @param maestro Parent Maestro.
	 */
	void Section::set_maestro(Maestro *maestro) {
		this->maestro_ = maestro;
	}

	/**
	 * Sets the Section's offset, which shifts the Section's output along the Pixel grid.
	 * @param x Offset along the x axis.
	 * @param y Offset along the y axis.
	 * @return Offset.
	 */
	Point* Section::set_offset(uint16_t x, uint16_t y) {
		offset_.set(x, y);
		return &offset_;
	}

	/**
		Sets the specified Pixel to a new color.

		@param pixel The index of the Pixel to update.
		@param color New color.
	*/
	void Section::set_one(uint32_t pixel, Colors::RGB* color) {
		// Only continue if Pixel is within the bounds of the array.
		if (pixel < dimensions_.size()) {
			if (animation_ != nullptr) {
				pixels_[pixel].set_next_color(color, animation_->get_timer()->get_step_count());
			}
			else {
				pixels_[pixel].set_next_color(color, 1);
			}
		}
	}

	/**
		Sets the specified Pixel to a new color.

		@param x The column number of the Pixel.
		@param y The row number of the Pixel.
		@param color New color.
	*/
	void Section::set_one(uint16_t x, uint16_t y, Colors::RGB* color) {
		set_one(dimensions_.get_inline_index(x, y), color);
	}

	/**
	 * Sets the direction and rate that the Section will scroll.
	 * The values determine the amount of time until the Section completes a single scroll along the respective axis.
	 * Using a negative value reverses the scrolling direction.
	 * Setting either axis to 0 disables scrolling on that axis.
	 *
	 * @param x Scrolling interval along the x axis.
	 * @param y Scrolling interval along the y axis.
	 * @param reverse_x If true, reverses the scrolling direction along the x axis.
	 * @param reverse_y If true, reverses the scrolling direction along the y axis.
	 */
	Section::Scroll* Section::set_scroll(uint16_t x, uint16_t y, bool reverse_x, bool reverse_y) {
		if (scroll_ == nullptr) {
			scroll_ = new Scroll();
		}

		scroll_->set(this->get_maestro()->get_timer()->get_interval(), this->get_dimensions(), x, y, reverse_x, reverse_y);

		return scroll_;
	}

	/**
	 * Sets the last update time of each component's timer.
	 * @param new_time New last refresh time.
	 */
	void Section::sync(const uint32_t &new_time) {
		if (layer_ != nullptr) {
			layer_->section->sync(new_time);
		}
		if (animation_ != nullptr) {
			animation_->get_timer()->set_last_time(new_time);
		}
		if (canvas_ != nullptr && canvas_->get_frame_timer() != nullptr) {
			canvas_->get_frame_timer()->set_last_time(new_time);
		}
		if (scroll_ != nullptr) {
			scroll_->timer_x->set_last_time(new_time);
			scroll_->timer_y->set_last_time(new_time);
		}
	}

	/**
		Main update routine.

		@param current_time Program runtime.
	*/
	void Section::update(const uint32_t& current_time) {

		/*
		 * Rendering is done in a specific order.
		 * First, update all layers.
		 * Second, update the Animation. This sets next_color_ for each Pixel.
		 * Now that next_color_ is set, update each Pixel.
		 * Next, draw the Canvas.
		 * Once everything's together, scroll the result.
		 */
		if (layer_ != nullptr) {
			layer_->section->update(current_time);
		}

		if (animation_ != nullptr) {
			animation_->update(current_time);
		}

		for (uint32_t pixel = 0; pixel < dimensions_.size(); pixel++) {
			pixels_[pixel].update();
		}

		if (canvas_ != nullptr) {
			canvas_->update(current_time);
		}

		if (scroll_ != nullptr) {
			update_scroll(current_time);
		}
	}

	/**
	 * Scrolls the Section.
	 * @param current_time The program's current runtime.
	 */
	void Section::update_scroll(const uint32_t &current_time) {

		uint16_t x_step = 0, y_step = 0;

		// Scroll x axis
		if (scroll_->timer_x != nullptr || scroll_->step_x != 0) {
			// If a timer is used (scrolling < 1 pixel per update), determine whether it's time to scroll.
			if (scroll_->timer_x != nullptr) {
				if (scroll_->timer_x->update(current_time)) {
					x_step = 1;
				}
			}
			// If a timer is not used (scrolling > 1 pixel per update), apply the scroll amount directly to the offset.
			else if (scroll_->step_x > 0) {
				x_step = scroll_->step_x;
			}

			// Check to see if we need to reset the offset
			if (!scroll_->reverse_x && offset_.x >= dimensions_.x) {
				offset_.x = 0;
			}
			else if (scroll_->reverse_x && offset_.x == 0) {
				offset_.x = dimensions_.x;
			}

			// Finally, apply the movement
			if (!scroll_->reverse_x) {
				offset_.x += x_step;
			}
			else {
				offset_.x -= x_step;
			}
		}


		// Repeat for y axis
		if (scroll_->timer_y != nullptr || scroll_->step_y != 0) {
			if (scroll_->timer_y != nullptr) {
				if (scroll_->timer_y->update(current_time)) {
					y_step = 1;
				}
			}
			else if (scroll_->step_y > 0) {
				y_step = scroll_->step_y;
			}

			// Check to see if we need to reset the offset
			if (!scroll_->reverse_y && offset_.y >= dimensions_.y) {
				offset_.y = 0;
			}
			else if (scroll_->reverse_y && offset_.y == 0) {
				offset_.y = dimensions_.y;
			}

			// Finally, apply the movement
			if (!scroll_->reverse_y) {
				offset_.y += y_step;
			}
			else {
				offset_.y -= y_step;
			}
		}
	}

	Section::~Section() {
		remove_animation();
		remove_canvas();
		remove_layer();
		remove_scroll();

		delete [] pixels_;
	}
}
