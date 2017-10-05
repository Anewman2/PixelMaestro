#include "cuecontroller.h"
#include "animationcuehandler.h"
#include "canvascuehandler.h"
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
	void CueController::assemble(uint8_t payload_size) {
		/*
		 * Final Cue has the following form: [ID] [Checksum] [Size] [Payload]
		 *
		 * [ID] is a unique identifier confirming that this is a PixelMaestro Cue (hence PMC).
		 * [Size] is the size of the payload.
		 * [Checksum] is a value generated for error detection.
		 * [Payload] contains the actual command with parameters.
		 */

		for (uint8_t i = 0; i < Byte::ChecksumByte; i++) {
			cue_[i] = header_[i];
		}

		cue_[Byte::SizeByte] = payload_size;

		cue_[Byte::ChecksumByte] = checksum(cue_, Byte::PayloadByte + payload_size);
	}

	/**
	 * Generates a checksum for verifying the integrity of a Cue.
	 * @param cue The contents of the Cue.
	 * @param cue_size The size of the Cue.
	 * @return New checksum.
	 */
	uint8_t CueController::checksum(uint8_t *cue, uint8_t cue_size) {
		uint32_t sum = 0;
		for (uint8_t i = 0; i < cue_size; i++) {

			// Make sure we don't include the checksum in its own calculation
			if (i != Byte::ChecksumByte) {
				sum += cue[i];
			}
		}

		return (sum % 256);
	}

	/**
	 * Enables the specified CueHandler.
	 * @param handler CueHandler to enable.
	 * @return New CueHandler.
	 */
	CueHandler* CueController::enable_handler(Handler handler) {
		if (handlers_[(uint8_t)handler] == nullptr) {
			switch(handler) {
				case AnimationHandler:
					handlers_[(uint8_t)Handler::AnimationHandler] = new AnimationCueHandler(this);
					break;
				case CanvasHandler:
					handlers_[(uint8_t)Handler::CanvasHandler] = new CanvasCueHandler(this);
					break;
				case MaestroHandler:
					handlers_[(uint8_t)Handler::MaestroHandler] = new MaestroCueHandler(this);
					break;
				case SectionHandler:
					handlers_[(uint8_t)Handler::SectionHandler] = new SectionCueHandler(this);
					break;
			}
		}

		return handlers_[(uint8_t)handler];
	}

	/**
	 * Returns the current Cue.
	 * @return Cue buffer.
	 */
	uint8_t* CueController::get_cue() {
		return cue_;
	}

	/**
	 * Returns the size of the currently loaded Cue.
	 * @return Cue size.
	 */
	uint8_t CueController::get_cue_size() {
		return (cue_[Byte::SizeByte] + Byte::PayloadByte);
	}

	/**
	 * Returns the instance of the specified Handler.
	 * @param handler Handler to get.
	 * @return Handler instance.
	 */
	CueHandler* CueController::get_handler(Handler handler) {
		if (handlers_[(uint8_t)handler] != nullptr) {
			return handlers_[(uint8_t)handler];
		}
		return nullptr;
	}

	/**
	 * Returns the controller's Maestro.
	 * @return Maestro controlling this CueController.
	 */
	Maestro* CueController::get_maestro() {
		return maestro_;
	}

	/**
	 * Reads a Cue byte-by-byte into the buffer.
	 * Once the end of the Cue has been reached, the Cue runs and the reader resets for the next Cue.
	 * @param byte Byte to read into the buffer.
	 */
	void CueController::read(uint8_t byte) {
		cue_[read_index_] = byte;
		read_index_++;

		/*
		 * Once we've read past the payload, run the Cue then reset the read index.
		 *
		 * We manually set the read index in the following cases:
		 *	1) If we successfully ran the Cue, set the index to 0 (start new Cue)
		 *	2) If the last bytes read match the header, set the index to the header size
		 *	3) If we've reached the buffer size, set the index to 0 (error / invalid Cue)
		 */
		if (read_index_ >= cue_[Byte::SizeByte] + Byte::PayloadByte) {
			run();
			read_index_ = 0;
		}
		else {
			if (read_index_ >= Header3Byte &&
				(cue_[read_index_ - Header3Byte] == header_[Header1Byte] &&
				 cue_[read_index_ - Header2Byte] == header_[Header2Byte] &&
				 cue_[read_index_] == header_[Header3Byte])) {

				cue_[Header1Byte] = header_[Header1Byte];
				cue_[Header2Byte] = header_[Header2Byte];
				cue_[Header3Byte] = header_[Header3Byte];
				read_index_ = Header3Byte;
			}
			else if ((int16_t)read_index_ + 1 >= 256) {
				read_index_ = 0;
			}
		}
	}

	/**
	 * Runs the currently loaded Cue.
	 */
	void CueController::run() {
		handlers_[cue_[Byte::PayloadByte]]->run(cue_);
	}

	/**
	 * Validates and runs the provided Cue.
	 * @param cue Cue to load.
	 */
	void CueController::run(uint8_t *cue) {
		if (validate_header(cue)) {
			handlers_[cue[Byte::PayloadByte]]->run(cue);
		}
	}

	/**
	 * Validates and runs multiple Cues sequentially.
	 * @param cues Cues to load.
	 * @param num_cues Number of Cues to load.
	 */
	void CueController::run(uint8_t *cues, uint8_t num_cues) {
		for (uint8_t i = 0; i < num_cues; i++) {
			run(&cues[i]);
		}
	}

	/**
	 * Validates the header and checksum of the provided Cue.
	 * @param cue Cue to validate.
	 * @return True if valid, false if not.
	 */
	bool CueController::validate_header(uint8_t *cue) {
		for (uint8_t i = 0; i < Byte::ChecksumByte; i++) {
			if (cue[i] != header_[i]) {
				return false;
			}
		}

		uint8_t size = cue[Byte::SizeByte] + Byte::PayloadByte;

		// Second, generate and compare the checksum.
		if (cue[Byte::ChecksumByte] != checksum(cue, size)) {
			return false;
		}

		return true;
	}

	CueController::~CueController() {
		for (uint8_t i = 0; i < 4; i++) {
			delete handlers_[i];
		}
	}
}