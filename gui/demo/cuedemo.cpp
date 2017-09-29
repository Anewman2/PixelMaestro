#include "cuedemo.h"
#include "cue/animationcuehandler.h"
#include "cue/cuecontroller.h"
#include "cue/canvascuehandler.h"
#include "cue/sectioncuehandler.h"

CueDemo::CueDemo(QWidget* parent, MaestroController* maestro_controller) : SimpleDrawingArea(parent, maestro_controller) {
	maestro_controller_->add_section_controller(new Point(10, 10));

	Maestro* maestro = maestro_controller_->get_maestro();

	CueController* controller = maestro->add_cue_controller();

	SectionCueHandler *section_handler = static_cast<SectionCueHandler*>(controller->enable_handler(CueController::Handler::SectionHandler));
	section_handler->add_overlay(0, Colors::MixMode::Overlay, 0);
	controller->run();

	section_handler->set_dimensions(0, 61, 9);
	controller->run();

	Colors::RGB colors[] = {Colors::SPRING, Colors::ROSE};
	section_handler->set_animation(0, Animation::Type::Cycle, false, colors, 2);
	controller->run();

	AnimationCueHandler* animation_handler = static_cast<AnimationCueHandler*>(controller->enable_handler(CueController::Handler::AnimationHandler));
	animation_handler->set_speed(0, 1000, 750);
	controller->run();

	section_handler->add_canvas(0, CanvasType::AnimationCanvas);
	controller->run();

	CanvasCueHandler* canvas_handler = static_cast<CanvasCueHandler*>(controller->enable_handler(CueController::Handler::CanvasHandler));
	canvas_handler->draw_text(0, 1, 1, Font::Type::Font5x8, "Hello world!", 12);
	controller->run();
}

CueDemo::~CueDemo() {}
