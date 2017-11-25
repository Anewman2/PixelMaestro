/*
 * CueController - Class for converting PixelMaestro commands to and from serialized strings.
 */

#include "cuecontroller.h"
#include "animationcuehandler.h"
#include "canvascuehandler.h"
#include "maestrocuehandler.h"
#include "sectioncuehandler.h"
#include "showcuehandler.h"

namespace PixelMaestro {

	/**
	 * Constructor.
	 * @param maestro Maestro that processed Cues will run on.
	 * @param buffer_size The size of the Cue buffer (defaults to 256).
	 */
	CueController::CueController(Maestro* maestro, uint16_t buffer_size) {
		this->maestro_ = maestro;
		this->buffer_size_ = buffer_size;
		this->buffer_ = new uint8_t[buffer_size] {0};
	}

	/**
	 * Prepares the Cue for transport by adding header info.
	 * @param payload_size The size of the payload.
	 * @return Assembled Cue.
	 */
	uint8_t* CueController::assemble(uint16_t payload_size) {
		/*
		 * Final Cue has the following form: [ID] [Checksum] [Size] [Payload]
		 *
		 * [ID] is a set of pre-defined characters marking the start of a Cue.
		 * [Size] is the size of the payload.
		 * [Checksum] is a value generated for error detection.
		 * [Payload] contains the actual command with parameters.
		 */

		for (uint8_t i = 0; i < Byte::ChecksumByte; i++) {
			buffer_[i] = id_[i];
		}

		IntByteConvert size(payload_size);
		buffer_[Byte::SizeByte1] = size.converted_0;
		buffer_[Byte::SizeByte2] = size.converted_1;

		buffer_[Byte::ChecksumByte] = checksum(buffer_, Byte::PayloadByte + payload_size);

		return buffer_;
	}

	/**
	 * Generates a checksum for verifying the integrity of a Cue.
	 * Calculated by summing each byte in the Cue, diving by 256, and taking the remainder.
	 * @param cue The contents of the Cue.
	 * @param cue_size The size of the Cue.
	 * @return New checksum.
	 */
	uint8_t CueController::checksum(uint8_t *cue, uint16_t cue_size) {
		uint32_t sum = 0;
		for (uint16_t i = 0; i < cue_size; i++) {

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
				case ShowHandler:
					handlers_[(uint8_t)Handler::ShowHandler] = new ShowCueHandler(this);
					break;
			}
		}

		return handlers_[(uint8_t)handler];
	}

	/**
	 * Returns the currently cached Cue.
	 * @return Cue buffer.
	 */
	uint8_t* CueController::get_buffer() {
		return buffer_;
	}

	/**
	 * Returns the size of the Cue buffer.
	 * @return Buffer size.
	 */
	uint16_t CueController::get_buffer_size() {
		return buffer_size_;
	}

	/**
	 * Returns the size of the currently cached Cue.
	 * @return Cue size.
	 */
	uint16_t CueController::get_cue_size() {
		return (IntByteConvert::byte_to_int(&buffer_[CueController::Byte::SizeByte1]) + Byte::PayloadByte);
	}

	/**
	 * Returns the size of the specified Cue.
	 * @param cue Cue to measure.
	 * @return Cue size.
	 */
	uint16_t CueController::get_cue_size(uint8_t *cue) {
		return (IntByteConvert::byte_to_int(&cue[CueController::Byte::SizeByte1]) + Byte::PayloadByte);
	}

	/**
	 * Returns the instance of the specified Handler.
	 * @param handler Handler to get.
	 * @return Handler instance.
	 */
	CueHandler* CueController::get_handler(Handler handler) {
		return handlers_[(uint8_t)handler];
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
	 * @return True if the Cue was successfully executed.
	 */
	bool CueController::read(uint8_t byte) {
		buffer_[read_index_] = byte;
		read_index_++;

		/*
		 * Check the current read index.
		 * If we've reached the end of the payload, run the Cue then reset the read index.
		 *
		 * We manually set the read index in the following cases:
		 *	1) If we successfully ran the last Cue, set the index to 0 (start a new Cue)
		 *	2) If the last bytes read match the Cue ID string, move the ID and read index to the start of the buffer (reduces the chance of a buffer overflow / split Cue)
		 *	3) If we've reached the buffer size limit, set the index to 0 (error / invalid Cue)
		 */
		if (read_index_ >= IntByteConvert::byte_to_int(&buffer_[Byte::SizeByte1]) + Byte::PayloadByte) {
			run(buffer_);
			read_index_ = 0;
			return true;
		}
		else {
			if (read_index_ > ID3Byte &&
				(buffer_[read_index_ - ID3Byte] == id_[ID1Byte] &&
				 buffer_[read_index_ - ID2Byte] == id_[ID2Byte] &&
				 buffer_[read_index_] == id_[ID3Byte])) {

				buffer_[ID1Byte] = id_[ID1Byte];
				buffer_[ID2Byte] = id_[ID2Byte];
				buffer_[ID3Byte] = id_[ID3Byte];
				read_index_ = ID3Byte;
			}
			else if ((uint32_t)read_index_ + 1 > buffer_size_) {
				read_index_ = 0;
			}
		}
		return false;
	}

	/**
	 * Runs the currently loaded Cue.
	 */
	void CueController::run() {
		handlers_[buffer_[Byte::PayloadByte]]->run(buffer_);
	}

	/**
	 * Validates and runs the specified Cue.
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
		// Check the ID
		for (uint8_t i = 0; i < Byte::ChecksumByte; i++) {
			if (cue[i] != id_[i]) {
				return false;
			}
		}

		// Validate the Checksum
		uint16_t size = IntByteConvert::byte_to_int(&cue[CueController::Byte::SizeByte1]) + Byte::PayloadByte;
		if (cue[Byte::ChecksumByte] != checksum(cue, size)) {
			return false;
		}

		return true;
	}

	CueController::~CueController() {
		delete [] buffer_;
		for (uint8_t i = 0; i < 4; i++) {
			delete handlers_[i];
		}
	}
}
