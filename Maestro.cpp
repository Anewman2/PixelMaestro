/*
	Maestro.cpp - Library for controlling a collection of Pixels and Sections.
*/

#include "Maestro.h"
#include "Section.h"

namespace PixelMaestro {
	/**
		Constructor. Initializes the Pixel array and divides it into Sections.

		@param sections Initial Section array.
		@param numSections Number of Sections.
	*/
	Maestro::Maestro(Section *sections, unsigned short numSections) {
		this->setSections(sections, numSections);
	}

	/**
		Returns the number of Sections.

		@return Number of Sections.
	*/
	unsigned short Maestro::getNumSections() {
		return num_sections_;
	}

	/**
		Returns whether the Maestro is running.

		@return Whether the Maestro is running.
	*/
	bool Maestro::getRunning() {
		return running_;
	}

	/**
		Returns the Section at the specified index.

		@param section Index of the Section to return.
		@return Section at the specified index.
	*/
	Section *Maestro::getSection(unsigned short section) {
		return &sections_[section];
	}

	/**
		Returns the update speed based on the fastest running Section.

		@return Update speed of the fastest Section.
	*/
	unsigned char Maestro::getUpdateSpeed() {
		// Start off at the slowest possible speed
		unsigned char minSpeed = 255;

		// The Maestro must be at least as fast as the fastest animation.
		for (unsigned short section = 0; section < num_sections_; section++) {
			if (sections_[section].getUpdateSpeed() < minSpeed) {
				minSpeed = sections_[section].getUpdateSpeed();
			}
		}

		return minSpeed;
	}

	/**
		Sets the Grids used in the Maestro.

		@param grids Array of Grids.
		@param numGrids Number of Grids in the array.
	*/
	void Maestro::setSections(Section *sections, unsigned short numSections) {
		sections_ = sections;
		num_sections_ = numSections;
	}

	/**
		Toggles whether the Maestro is conducting (running).
	*/
	void Maestro::toggleRunning() {
		running_ = !running_;
	}

	/**
		Main update routine.

		@param currentTime Program runtime.
	*/
	void Maestro::update(unsigned long currentTime) {
		// Call each Grid and Line's update method.
		if (running_) {
			for (unsigned short section = 0; section < num_sections_; section++) {
				sections_[section].update(currentTime);
			}
		}
	}
}
