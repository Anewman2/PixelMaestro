/*
	Section.cpp - Class for controlling multiple Pixels.
	Requires Pixel and Colors classes.
*/

#include <cmath>
#include <limits.h>
#include <stdlib.h>
#include "Colors.h"
#include "Pixel.h"
#include "Section.h"

namespace PixelMaestro {
	/**
		Default constructor.
		Note that if you use the default constructor, you MUST use setPixels() to initialize the Pixel array.
	*/
    Section::Section() {}

	/**
		Constructor. A Section is a collection of initialized Pixels.

		@param pixels Initial Pixel array.
		@param rows Number of rows of Pixels.
		@param columns Number of Pixels per row.
	*/
	Section::Section(Pixel *pixels, unsigned short rows, unsigned short columns) {
        setPixels(pixels, rows, columns);
	}

	/**
		Returns whether the Section is fading.

		@return Whether the Section is fading.
	*/
	bool Section::getFade() {
		return fade_;
	}

	/**
		Returns the number of Pixels in the Section.

		@return Number of Pixels.
	*/
	unsigned int Section::getNumPixels() {
		return layout_.rows * layout_.columns;
	}

	/**
		Returns the layout of the Pixel array.

		@return Layout of the array.
	*/
	Section::Layout *Section::getLayout() {
		return &layout_;
	}

	/**
		Returns the Pixel at the specified row and column.

		@param row Row number that the Pixel is in.
		@param column Column that the Pixel is in.
		@return Pixel Reference to Pixel.
	*/
	Pixel *Section::getPixel(unsigned short row, unsigned short column) {
		return &pixels_[(row * layout_.columns) + column];
	}

	/**
		Returns the Pixel at the specified index.

		@param pixel The index of the Pixel in the pixels_ array.
	*/
	Pixel *Section::getPixel(unsigned int pixel) {
		return &pixels_[pixel];
	}

	/**
		Returns the RGB value of the specified Pixel after applying post-processing effects.

		@param row Row that the Pixel is in.
		@param column Column that the Pixel is in.
		@return RGB value of the Pixel's de facto color.
	*/
	Colors::RGB Section::getPixelColor(unsigned short row, unsigned short column) {
		return this->getPixelColor((row * layout_.columns) + column);
	}

	/**
		Returns the RGB value of the specified Pixel after applying post-processing effects.

		@param pixel Index of the Pixel.
		@return RGB value of the Pixel's de facto color.
	*/
	Colors::RGB Section::getPixelColor(unsigned int pixel) {
		if (overlay_.section != nullptr) {
			return Colors::mixColors(this->getPixel(pixel)->getColor(), overlay_.section->getPixel(pixel)->getColor(), overlay_.mixMode, overlay_.alpha);
		}
		else {
			return *this->getPixel(pixel)->getColor();
		}
	}

	/**
		Returns the speed of the array.

		@return speed The current speed.
	*/
	unsigned char Section::getUpdateSpeed() {
		return speed_;
	}

	/**
		Returns the current color animation.

		@return Current color animation.
	*/
	Section::ColorAnimations Section::getColorAnimation() {
		return color_animation_;
	}

	/**
		Sets all Pixels to the specified color.

		@param color New color.
	*/
	void Section::setAll(Colors::RGB *color) {
		for (unsigned short row = 0; row < layout_.rows; row++) {
			for (unsigned short column = 0; column < layout_.columns; column++) {
				setOne(row, column, color);
			}
		}
	}

	/**
		Changes the current color animation.

		@param animation Animation selection.
		@param reverseAnimation Whether to display the animation in reverse (only works for certain animations).
	*/
	void Section::setColorAnimation(ColorAnimations animation, bool reverseAnimation) {
		// If animation was supplied, change to the desired animation.
		// Otherwise, increment the current animation to the next one.
		if (animation) {
			color_animation_ = animation;
		}
		else {
			unsigned char animationNum = color_animation_ + 1;

			if (animationNum > Section::ColorAnimations::PATTERN) {
				animationNum = Section::ColorAnimations::SOLID;
			}

			color_animation_ = ColorAnimations(animationNum);
		}

		reverse_animation_ = reverseAnimation;

		// TODO: Verify whether PONG *needs* cycle_index_ be reset to 0
	}

	/**
		Replaces the current color array.

		@param colors New color array.
		@param numColors Size of the array.
	*/
	void Section::setColors(Colors::RGB *colors, unsigned int numColors) {
		colors_ = colors;
		num_colors_ = numColors;
	}

	/**
		Jumps ahead to the specified point in the cycle.

		@param index Where the cycle should start.
	*/
	void Section::setCycleIndex(unsigned int index) {
		if (index > num_colors_) {
			cycle_index_ = (index - 1) % num_colors_;
		}
		else {
			cycle_index_ = index;
		}
	}

	/**
		Overlays a Section on top of the current Section.
		The base Section automatically handles blending in the layer on output.

		@param overlay Section to overlay.
	*/
	void Section::setOverlay(Overlay overlay) {
        overlay_ = overlay;
	}

	/**
		Sets the specified Pixel to a new color.

		@param pixel The index of the Pixel to update.
		@param color New color.
	*/
	void Section::setOne(unsigned int pixel, Colors::RGB *color) {
		this->getPixel(pixel)->setNextColor(color, fade_, speed_);
	}

	/**
		Sets the specified Pixel to a new color.

		@param row The row number of the Pixel.
		@param column The column number of the Pixel.
		@param color New color.
	*/
	void Section::setOne(unsigned short row, unsigned short column, Colors::RGB *color) {
		this->getPixel(row, column)->setNextColor(color, fade_, speed_);
	}

	/**
		Displays a pattern by activating Pixels corresponding to individual bits in the pattern.
		A pattern consists of several unsigned shorts, and Pixels are activated based on which bits are active.
		A single short corresponds to a row in the Section.

		@param pattern Pointer to the pattern array.
		@param patternRows Length of the pattern array (e.g. the number of rows in the pattern).
		@param numFrames Number of frames in the pattern.
	*/
	void Section::setPattern(unsigned int *pattern, unsigned short patternRows, unsigned short numFrames) {
		pattern_ = {
			pattern,
			patternRows,
			numFrames
		};
	}

	/**
        Sets the Pixel group used in the Section.
        The Pixel collection is a standard array, while specifying the row and column count logically organizes the array into a grid.

        @param pixels Initial Pixel array.
		@param rows Number of rows of Pixels.
		@param columns Number of columns of Pixels.
	*/
	void Section::setPixels(Pixel* pixels, unsigned short rows, unsigned short columns) {
        pixels_ = pixels;
		layout_.rows = rows;
		layout_.columns = columns;
	}

	/**
		Removes the overlay from the Section.
	*/
	void Section::unsetOverlay() {
		overlay_.section = nullptr;
		overlay_.alpha = 0;
	}

	/**
		Sets the update speed in milliseconds.

		@param speed Delay between individual animation steps.
		@param delay Delay between complete animation cycles.
	*/
	void Section::setUpdateSpeed(unsigned char speed, unsigned char delay) {
		delay_ = delay;
		speed_ = speed;
	}

	/**
		Toggles fading.
	*/
	void Section::toggleFade() {
		fade_ = !fade_;
	}

	/**
		Main update routine.

		@param currentTime Program runtime.
	*/
	void Section::update(unsigned long currentTime) {
		// Update the timer
		current_time_ = &currentTime;

		/*
			Run if either of the following cases is true:
				1. The Section has never updated (last_time_ == 0).
				2. The program's runtime minus the Section's last update time exceeds the update interval (current_time_ - last_time_ >= speed).
		*/
		if ((last_time_ == 0) || (*current_time_ - last_time_ >= speed_)) {

			// Determine which animation to run, then run the associated method.
			// Defaults to off.
			switch (color_animation_) {
				case Section::ColorAnimations::SOLID:
					animation_solid();
					break;
				case Section::ColorAnimations::BLINK:
					animation_blink();
					break;
				case Section::ColorAnimations::CYCLE:
					animation_cycle();
					break;
				case Section::ColorAnimations::WAVE:
					animation_wave();
					break;
				case Section::ColorAnimations::MERGE:
					animation_merge();
					break;
				case Section::ColorAnimations::PONG:
					animation_pong();
					break;
				case Section::ColorAnimations::SPARKLE:
					animation_sparkle();
					break;
				case Section::ColorAnimations::RANDOMINDEX:
					animation_randomIndex();
					break;
				case Section::ColorAnimations::PATTERN:
					animation_pattern();
					break;
				case Section::ColorAnimations::STATIC:
					animation_static();
					break;
				default:
					setAll(&Colors::BLACK);
					break;
			}

			// Update each Pixel.
			for (unsigned int pixel = 0; pixel < this->getNumPixels(); pixel++) {
				this->getPixel(pixel)->update(fade_);
			}

			// Update the timer.
			last_time_ = *current_time_;
		}
	}

	// Private animation functions

	/**
		Creates a blinking effect.

		Modes: BLINK
	*/
	void Section::animation_blink() {
		// Alternate the Pixel between its normal color and off (Colors::BLACK).
		if (cycle_index_ == 0) {
			for (unsigned short row = 0; row < layout_.rows; row++) {
				for (unsigned short column = 0; column < layout_.columns; column++) {
					this->setOne(row, column, &colors_[animation_getColorIndex(column)]);
				}
			}
		}
		else {
			setAll(&Colors::BLACK);
		}

		// Only run for two cycles.
		animation_updateCycle(0, 2);
	}

	/**
		Cycles all Pixels through all stored colors.

		Modes: CYCLE
	*/
	void Section::animation_cycle() {
		setAll(&colors_[cycle_index_]);
		animation_updateCycle(0, num_colors_);
	}

	/**
		Calculates the index of a color.
		Used mainly to determine which color a Pixel should use during an animation based on where it is in the array.
		For example, this allows a Section of 10 Pixels to use an array of 5 colors by repeating the colors after Pixel 5.

		@param count Number to resolve to an index.
		@return Resulting index.
	*/
	unsigned int Section::animation_getColorIndex(unsigned int count) {
		if (count >= num_colors_) {
			count %= num_colors_;
		}

		return count;
	}

	/**
		Creates an effect where colors converge into the center of the array.

		Modes: MERGE
	*/
	void Section::animation_merge() {

		// Calculate the center of the array
		unsigned short midPoint;
		unsigned short count;

		for (unsigned short row = 0; row < layout_.rows; row++) {
			midPoint = (layout_.columns / 2) - 1;
			count = 0;

			// column *HAS* TO BE A SIGNED INT IN ORDER TO ACCESS INDEX 0.
			for (int column = midPoint; column >= 0; column--) {
				setOne(row, column, &colors_[animation_getColorIndex(count + cycle_index_)]);
				count++;
			}

			/*
				Check for an odd number of Pixels.
				If so, set the center one to index 0.
			*/
			if (this->getNumPixels() % 2 != 0) {
				midPoint += 1;
				setOne(row, midPoint, &colors_[cycle_index_]);
			}

			midPoint += 1;

			// Go from the center to the last
			count = 0;
			for (unsigned int column = midPoint; column < layout_.columns; column++) {
				setOne(row, column, &colors_[animation_getColorIndex(count + cycle_index_)]);
				count++;
			}
		}

		animation_updateCycle(0, num_colors_);
	}

	/**
		Scrolls through the pattern stored in pattern_.
		For now, this just sets it to whatever is stored in pattern_. Anything additional would require a 2D array.
		If there is no pattern set, blink.

		Modes: PATTERN
	*/
	void Section::animation_pattern() {
		// If the pattern has not been set, blink the array.
		if (pattern_.pattern == nullptr) {
			animation_blink();
			return;
		}

		// Increment through each bit. If the bit is set, turn on the Pixel.
		for (unsigned short row = 0; row < layout_.rows; row++) {
			for (unsigned short column = 0; column < layout_.columns; column++) {
				// If this row isn't even in the pattern, just set it to black.
				if (row >= pattern_.height) {
					setOne(row, column, &Colors::BLACK);
				}
				else {
					if (pattern_.pattern[(pattern_.height * cycle_index_) + row] & (unsigned int)pow(2, column)) {
						setOne(row, column, &colors_[animation_getColorIndex(column)]);
					}
					else {
						setOne(row, column, &Colors::BLACK);
					}
				}
			}
		}

		animation_updateCycle(0, pattern_.frames);
	}

	/**
		Cycles colors back and forth in a ping-pong pattern.

		Modes: PONG
	*/
	void Section::animation_pong() {
		for (unsigned short row = 0; row < layout_.rows; row++) {
			for (unsigned short column = 0; column < layout_.columns; column++) {
				setOne(row, column, &colors_[animation_getColorIndex(column + cycle_index_)]);
			}
		}

		if (cycle_index_ == 0) {
			reverse_animation_ = false;		// Start upcycle
		}
		if (cycle_index_ == num_colors_ - 1) {
			reverse_animation_ = true;		// Start downcycle
		}

		animation_updateCycle(0, num_colors_);
	}

	/**
		Sets each Pixel to a random color stored in colors_.

		Modes: RANDOMINDEX
	*/
	void Section::animation_randomIndex() {
		for (unsigned int pixel = 0; pixel < this->getNumPixels(); pixel++) {
			setOne(pixel, &colors_[rand() % num_colors_]);
		}
	}

	/**
		Sets each Pixel to a solid color.

		Modes: SOLID
	*/
	void Section::animation_solid() {
		for (unsigned short row = 0; row < layout_.rows; row++) {
			for (unsigned short column = 0; column < layout_.columns; column++) {
				setOne(row, column, &colors_[animation_getColorIndex(column)]);
			}
		}
	}

	/**
		Creates a shimmering effect by turning on random Pixels.
		NOTE: Requires a very fast refresh rate (1 - 2 ticks).

		Modes: SPARKLE
	*/
	void Section::animation_sparkle() {
		for (unsigned int row = 0; row < layout_.rows; row++) {
			for (unsigned short column = 0; column < layout_.columns; column++) {
				// The number of lit Pixels is inversely proportional to the threshold (e.g. the higher the threshold, the fewer the lit Pixels)
				if ((rand() % 100) > 60) {
					setOne(row, column, &colors_[animation_getColorIndex(column)]);
				}
				else {
					setOne(row, column, &Colors::BLACK);
				}
			}
		}
	}


	/**
		Creates a static effect by blending each Pixel between varying levels of gray.
		WARNING: THIS EFFECT MODIFIES THE COLOR ARRAY.

		Modes: STATIC
	*/
	void Section::animation_static() {
		for (unsigned int pixel = 0; pixel < this->getNumPixels(); pixel++) {
			colors_[pixel] = Colors::mixColors(&Colors::BLACK, &Colors::WHITE, Colors::MixMode::ALPHA_BLENDING, 0.0 + (rand() / ( RAND_MAX / (0.95) ) ));
			setOne(pixel, &colors_[pixel]);
		}
	}

	/**
		Updates the current cycle.
		If reverse_animation_ is true, this will automatically reverse the animation.
		@param min The minimum value of the cycle.
		@param max The maximum value of the cycle.
	*/
	void Section::animation_updateCycle(unsigned int min, unsigned int max) {
		// Only continue if the Pixels are finished cycling through their steps.
		if ((pixels_[0].getStepCount() == 0) && (*current_time_ - cycle_end_ >= delay_)) {
			cycle_end_ = *current_time_;
			if (reverse_animation_) {
				cycle_index_--;
				if ((unsigned int)(cycle_index_ + 1) == max) {	// I know, buffer overflows are ugly, but in this case it works.
					cycle_index_ = max - 1;
				}
			}
			else {
				cycle_index_++;
				if (cycle_index_ >= max) {
					cycle_index_ = min;
				}
			}
		}
	}

	/**
		Creates a wave effect by scrolling the color array across the Section.

		Modes: WAVE
	*/
	void Section::animation_wave() {
		for (unsigned short row = 0; row < layout_.rows; row++) {
			for (unsigned short column = 0; column < layout_.columns; column++) {
				setOne(row, column, &colors_[animation_getColorIndex(column + cycle_index_)]);
			}
		}

		animation_updateCycle(0, num_colors_);
	}
}
