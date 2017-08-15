/*
	Show.cpp - Library for scheduling PixelMaestro animations.
*/

#ifndef SHOW_H
#define SHOW_H

#include "show/Transition.h"
#include "Maestro.h"

using namespace PixelMaestro;

namespace PixelMaestro {
	class Show {
		public:
			/// The method used to measure time between Transitions.
			enum TimingModes {

				/// Counts time from when the Show starts.
				ABSOLUTE,

				/// Counts time since the last Transition.
				RELATIVE
			};

			Show();
			Show (Maestro *maestro);
			unsigned short getCurrentIndex();
			bool getLooping();
			Maestro *getMaestro();
			void setMaestro(Maestro *maestro);
			void setTiming(TimingModes timing);
			void setTransitions(Transition **transitions, unsigned char numTransitions);
			void toggleLooping();
			void update(const unsigned long &currentTime);

		private:
			/// The index of the current Transition.
			unsigned short current_index_ = 0;

			/// The index of the last run Transition.
			unsigned short last_index_ = 0;

			/// The time that the last Transition ran.
			unsigned long last_time_ = 0;

			/// Whether to loop over the Transition.
			bool loop_ = false;

			/// The Maestro that the Transitions apply to.
			Maestro *maestro_ = nullptr;

			/// The number of Transitions in the array.
			unsigned char num_transitions_;

			/// Method for measuring a Transition's start time. Defaults to Absolute.
			TimingModes timing_ = TimingModes::ABSOLUTE;

			/// Transitions used in the Show.
			Transition **transitions_;

			unsigned short getNextIndex();
	};
}

#endif // SHOW_H
