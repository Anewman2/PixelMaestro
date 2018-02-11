#include "../catch/single_include/catch.hpp"
#include "animation/animation.h"
#include "animation/solidanimation.h"
#include "canvas/animationcanvas.h"
#include "canvas/colorcanvas.h"
#include "colorpresets.h"
#include "core/colors.h"
#include "core/maestro.h"
#include "core/point.h"
#include "core/section.h"
#include "sectiontest.h"

using namespace PixelMaestro;

TEST_CASE("Create and manipulate a section.", "[Section]") {
	Point dimensions(1, 10);
	Section sections[] = {
		Section(dimensions)
	};
	Maestro maestro(sections, 1);

	Section* section = &sections[0];

	int test_pixel = 0;

	SECTION("Verify that Section dimensions are set correctly.") {
		REQUIRE(*section->get_dimensions() == dimensions);
	}

	SECTION("Verify that Animations and Colors are set correctly.") {
		Animation* animation = section->set_animation(AnimationType::Solid, nullptr, 0);
		animation->set_colors(ColorPresets::Colorwheel, 12);
		animation->set_timer(100);
		animation->set_fade(false);

		maestro.update(101);

		REQUIRE(*section->get_pixel(test_pixel, 0)->get_color() == ColorPresets::Colorwheel[test_pixel]);
	}

	SECTION("Verify that different Canvas types can be added.") {
		Animation* animation = section->set_animation(AnimationType::Solid, ColorPresets::Colorwheel, 12);
		animation->set_fade(false);

		// Draw a filled in animation rectangle
		AnimationCanvas* animation_canvas = static_cast<AnimationCanvas*>(section->set_canvas(CanvasType::Type::AnimationCanvas));
		animation_canvas->draw_rect(0, 0, section->get_dimensions()->x, section->get_dimensions()->y, true);

		maestro.update(101);
		REQUIRE(section->get_pixel_color(test_pixel, 0) == ColorPresets::Colorwheel[test_pixel]);

		// Delete the Canvas
		section->remove_canvas();
		REQUIRE(section->get_canvas() == nullptr);

		// Draw a color rectangle
		ColorCanvas* color_canvas = static_cast<ColorCanvas*>(section->set_canvas(CanvasType::Type::ColorCanvas));
		color_canvas->draw_rect(ColorPresets::Chartreuse, 0, 0, section->get_dimensions()->x, section->get_dimensions()->y, true);

		REQUIRE(section->get_pixel_color(test_pixel, 0) == ColorPresets::Chartreuse);
	}

	SECTION("Verify that Layers work.") {
		Colors::RGB section_colors[] = {ColorPresets::White};
		Colors::RGB layer_colors[] = {ColorPresets::Black, ColorPresets::Blue};

		Section::Layer* layer = section->set_layer(Colors::MixMode::None);
		Animation* section_animation = section->set_animation(AnimationType::Solid, section_colors, 1);
		Animation* layer_animation = layer->section->set_animation(AnimationType::Solid, section_colors, 1);
		section_animation->set_timer(100);
		layer_animation->set_timer(100);
		section_animation->set_fade(false);
		layer_animation->set_fade(false);

		maestro.update(101);

		// Test no mix mode
		REQUIRE(section->get_pixel_color(0, 0) == section_colors[0]);

		// Test Alpha mix mode
		layer->mix_mode = Colors::MixMode::Alpha;
		layer->alpha = 127;
		REQUIRE(section->get_pixel_color(0, 0) == (section_colors[0] * 0.5));

		// Test Multiply mix mode
		layer->mix_mode = Colors::MixMode::Multiply;
		REQUIRE(section->get_pixel_color(0, 0) == layer_colors[1]);

		// Test Layer mix mode
		layer->mix_mode = Colors::MixMode::Overlay;
		REQUIRE(section->get_pixel_color(0, 0) == layer_colors[0]);
		maestro.update(201);
		REQUIRE(section->get_pixel_color(0, 0) == layer_colors[1]);
	}
}
