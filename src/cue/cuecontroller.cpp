#include "animationcuehandler.h"
#include "canvascuehandler.h"
#include "cuecontroller.h"
#include "maestrocuehandler.h"
#include "sectioncuehandler.h"

namespace PixelMaestro {

	/**
	 * Constructor.
	 * @param maestro Maestro that processed Cues will run on.
	 */
	CueController::CueController(Maestro* maestro) {
		maestro_ = maestro;
	}

	/**
	 * Prepares the Cue for transport by adding header info.
	 * @param payload_size The size of the payload.
	 */
	void CueController::assemble(unsigned char payload_size) {
		/*
		 * Final Cue has the following form: [Header] [Checksum] [Size] [Payload]
		 *
		 * [Header] is a unique identifier confirming that this is a PixelMaestro Cue (hence PMC).
		 * [Size] is the size of the payload.
		 * [Checksum] is a value generated for error detection.
		 * [Payload] contains the actual command with parameters.
		 */

		for (unsigned char i = 0; i < Bit::ChecksumBit; i++) {
			cue_[i] = header_[i];
		}

		cue_[Bit::SizeBit] = payload_size;

		cue_[Bit::ChecksumBit] = checksum(cue_, Bit::PayloadBit + payload_size);
	}

	/**
	 * Generates a checksum for verifying the integrity of a Cue.
	 * @param cue The contents of the Cue.
	 * @param cue_size The size of the Cue.
	 * @return New checksum.
	 */
	unsigned char CueController::checksum(unsigned char *cue, unsigned char cue_size) {
		unsigned int sum = 0;
		for (unsigned char i = 0; i < cue_size; i++) {

			// Make sure we don't include the checksum in its own calculation
			if (i != Bit::ChecksumBit) {
				sum += cue[i];
			}
		}

		return (sum % 256);
	}

	CueHandler* CueController::enable_handler(Handler handler) {
		switch(handler) {
			case AnimationHandler:
				handlers_[(int)Handler::AnimationHandler] = new AnimationCueHandler(this);
				break;
			case CanvasHandler:
				handlers_[(int)Handler::CanvasHandler] = new CanvasCueHandler(this);
				break;
			case MaestroHandler:
				handlers_[(int)Handler::MaestroHandler] = new MaestroCueHandler(this);
				break;
			case SectionHandler:
				handlers_[(int)Handler::SectionHandler] = new SectionCueHandler(this);
				break;
		}

		return handlers_[(int)handler];
	}

	/**
	 * Returns the current Cue.
	 * @return Cue buffer.
	 */
	unsigned char* CueController::get_cue() {
		return cue_;
	}

	/**
	 * Returns the instance of the specified Handler.
	 * @param handler Handler to get.
	 * @return Handler instance.
	 */
	CueHandler* CueController::get_handler(Handler handler) {
		if (handlers_[(int)handler] != nullptr) {
			return handlers_[(int)handler];
		}
		return nullptr;
	}

	/**
	 * Returns the Maestro managed by this controller.
	 * @return Maestro.
	 */
	Maestro* CueController::get_maestro() {
		return maestro_;
	}

	/**
	 * Loads and runs a raw Cue.
	 * @param cue Cue to load.
	 */
	void CueController::load(unsigned char *cue) {
		// First, verify the header.
		for (unsigned char i = 0; i < Bit::ChecksumBit; i++) {
			if (cue[i] != header_[i]) {
				return;
			}
		}

		unsigned char size = cue[Bit::SizeBit] + Bit::PayloadBit;

		// Second, generate and compare the checksum.
		if (cue[Bit::ChecksumBit] != checksum(cue, size)) {
			return;
		}

		// Next, load the Cue into the local buffer.
		for (unsigned char i = 0; i < size; i++) {
			cue_[i] = cue[i];
		}

		// Finally, run the Cue.
		run();
	}

	/**
	 * Loads and executes multiple Cues sequentially.
	 * @param cues Cues to load.
	 * @param num_cues Number of Cues to load.
	 */
	void CueController::load(unsigned char *cues, unsigned char num_cues) {
		for (unsigned char i = 0; i < num_cues; i++) {
			load(&cues[i]);
		}
	}

	/**
	 * Runs the currently loaded Cue.
	 * @param maestro The Maestro to run the command against.
	 * @param cue The Cue to run.
	 */
	void CueController::run() {
		handlers_[cue_[Bit::PayloadBit]]->run(cue_);
	}

	/**
	 * Initializes the required Cue handlers.
	 * @param handlers List of Handlers to load.
	 * @param num_handlers Number of Handlers.
	 */
	void CueController::set_handlers(Handler *components, unsigned char num_handlers) {
		for (unsigned char i = 0; i < num_handlers; i++) {
			switch(components[i]) {
				case AnimationHandler:
					handlers_[(int)Handler::AnimationHandler] = new AnimationCueHandler(this);
					break;
				case CanvasHandler:
					handlers_[(int)Handler::CanvasHandler] = new CanvasCueHandler(this);
					break;
				case MaestroHandler:
					handlers_[(int)Handler::MaestroHandler] = new MaestroCueHandler(this);
					break;
				case SectionHandler:
					handlers_[(int)Handler::SectionHandler] = new SectionCueHandler(this);
					break;
			}
		}
	}
}
