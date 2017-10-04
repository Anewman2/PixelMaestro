#include "animation/blinkanimation.h"
#include "animation/cycleanimation.h"
#include "animation/lightninganimation.h"
#include "animation/lightninganimationcontrol.h"
#include "animation/mergeanimation.h"
#include "animation/mandelbrotanimation.h"
#include "animation/plasmaanimation.h"
#include "animation/plasmaanimationcontrol.h"
#include "animation/radialanimation.h"
#include "animation/randomanimation.h"
#include "animation/solidanimation.h"
#include "animation/sparkleanimation.h"
#include "animation/sparkleanimationcontrol.h"
#include "animation/waveanimation.h"
#include "canvas/animationcanvas.h"
#include "canvas/colorcanvas.h"
#include "canvas/canvascontrol.h"
#include "controller/maestrocontroller.h"
#include "controller/sectioncontroller.h"
#include "core/section.h"
#include "drawingarea/simpledrawingarea.h"
#include "maestrocontrol.h"
#include <QPalette>
#include <QString>
#include "ui_maestrocontrol.h"

/**
 * Constructor.
 * @param parent The QWidget containing this controller.
 * @param maestro_controller The MaestroController being controlled.
 */
MaestroControl::MaestroControl(QWidget* parent, MaestroController* maestro_controller) : QWidget(parent), ui(new Ui::MaestroControl), io_service_(), serial_port_(io_service_) {

	// Assign easy reference variables for the Maestro
	this->maestro_controller_ = maestro_controller;

	// Open connection to Arduino
	if (serial_enabled_) {

		controller_ = maestro_controller_->get_maestro()->add_cue_controller();
		animation_handler = static_cast<AnimationCueHandler*>(controller_->enable_handler(CueController::Handler::AnimationHandler));
		canvas_handler = static_cast<CanvasCueHandler*>(controller_->enable_handler(CueController::Handler::CanvasHandler));
		maestro_handler = static_cast<MaestroCueHandler*>(controller_->enable_handler(CueController::Handler::MaestroHandler));
		section_handler = static_cast<SectionCueHandler*>(controller_->enable_handler(CueController::Handler::SectionHandler));

		struct stat buffer;
		if (stat (port_num_, &buffer) == 0) {
			serial_port_.open(port_num_);
			serial_port_.set_option(boost::asio::serial_port_base::baud_rate(9600));
		}
	}

	// Initialize UI
	ui->setupUi(this);
	this->initialize();
}

/**
 * Applies the active Section settings to the UI.
 */
void MaestroControl::get_section_settings() {
	// Apply animation options and speed
	Animation* animation = active_section_controller_->get_section()->get_animation();
	ui->orientationComboBox->setCurrentIndex(animation->get_orientation());
	ui->reverse_animationCheckBox->setChecked(animation->get_reverse());
	ui->fadeCheckBox->setChecked(animation->get_fade());
	ui->num_colorsSpinBox->setValue(animation->get_num_colors());
	ui->cycleSlider->setValue(ui->cycleSlider->maximum() - active_section_controller_->get_section()->get_animation()->get_speed());

	// Get the animation type, then change to the animation without firing the signal
	ui->animationComboBox->blockSignals(true);
	ui->animationComboBox->setCurrentIndex(active_section_controller_->get_section()->get_animation()->get_type());
	ui->animationComboBox->blockSignals(false);
	show_extra_controls(active_section_controller_->get_section()->get_animation());

	// Get Overlay MixMode and alpha from the Overlay's parent section
	QStringList section_type = ui->sectionComboBox->currentText().split(" ");
	if (QString::compare(section_type[0], "overlay", Qt::CaseInsensitive) == 0) {
		ui->mix_modeComboBox->setCurrentIndex(maestro_controller_->get_section_controller(section_type[1].toInt() - 1)->get_overlay()->mix_mode);
		ui->alphaSpinBox->setValue(maestro_controller_->get_section_controller(section_type[1].toInt() - 1)->get_overlay()->alpha);
	}

	// Get the current color scheme
	if (active_section_controller_->mc_color_scheme_ != 0) {
		ui->colorComboBox->setCurrentIndex(active_section_controller_->mc_color_scheme_);
	}
	else {
		// Custom scheme
		Animation* animation = active_section_controller_->get_section()->get_animation();
		Colors::RGB first_color = Colors::BLACK;

		if (animation != nullptr && animation->get_colors() != nullptr) {
			first_color = active_section_controller_->get_section()->get_animation()->get_colors()[0];
		}

		ui->colorComboBox->setCurrentIndex(0);
		ui->redSlider->setValue(first_color.r);
		ui->greenSlider->setValue(first_color.g);
		ui->blueSlider->setValue(first_color.b);
		on_custom_color_changed();
	}
}

/**
 * Build the initial UI.
 */
void MaestroControl::initialize() {
	active_section_controller_ = maestro_controller_->get_section_controller(0);
	active_section_controller_->get_section()->set_animation(new SolidAnimation());

	// Populate Animation combo box
	ui->animationComboBox->addItems({"Blink", "Cycle", "Lightning", "Mandelbrot", "Merge", "Plasma", "Radial", "Random", "Solid", "Sparkle", "Wave"});
	ui->orientationComboBox->addItems({"Horizontal", "Vertical"});

	// Populate color combo box
	ui->colorComboBox->addItems({"Custom", "Fire", "Deep Sea", "Color Wheel"});
	ui->colorComboBox->setCurrentIndex(2);
	set_custom_color_controls_visible(false);

	// Set default values
	ui->sectionComboBox->addItem("Section 1");

	// Add an Overlay
	active_section_controller_->add_overlay(Colors::MixMode::None);
	active_section_controller_->get_overlay()->section->set_animation(new SolidAnimation(Colors::COLORWHEEL, 12));
	ui->sectionComboBox->addItem(QString("Overlay 1"));

	// Initialize Overlay controls
	ui->mix_modeComboBox->addItems({"None", "Alpha", "Multiply", "Overlay"});
	ui->alphaSpinBox->setVisible(false);

	// Initialize Canvas controls
	ui->canvasComboBox->addItems({"No Canvas", "Animation Canvas", "Color Canvas"});

	get_section_settings();
}

/**
 * Updates the color array based on changes to the color scheme and settings.
 * @param color Base color to use when generating the array.
 */
void MaestroControl::change_scaling_color_array(Colors::RGB color) {
	uint32_t num_colors = (uint32_t)ui->num_colorsSpinBox->value();

	std::vector<Colors::RGB> tmp_colors;
	tmp_colors.resize(num_colors);

	uint8_t threshold = 255 - (uint8_t)ui->thresholdSpinBox->value();
	Colors::RGB tmp[num_colors];
	Colors::generate_scaling_color_array(tmp, &color, num_colors, threshold, true);
	active_section_controller_->get_section()->get_animation()->set_colors(tmp, num_colors);

	if (serial_enabled_) {
		animation_handler->set_colors(0, active_section_controller_->is_overlay_, tmp, num_colors);
		send_to_device(controller_->get_cue(), controller_->get_cue_size());
	}

	// Release tmp_colors
	std::vector<Colors::RGB>().swap(tmp_colors);
}

/**
 * Sets the Overlay's transparency level.
 * @param arg1 Transparency level from 0 - 255.
 */
void MaestroControl::on_alphaSpinBox_valueChanged(int arg1) {
	maestro_controller_->get_section_controller(0)->get_overlay()->alpha = arg1;

	if (serial_enabled_) {
		section_handler->add_overlay(0, active_section_controller_->is_overlay_, maestro_controller_->get_section_controller(0)->get_overlay()->mix_mode, arg1);
		send_to_device(controller_->get_cue(), controller_->get_cue_size());
	}
}

/**
 * Changes the current animation.
 * @param index Index of the new animation.
 */
void MaestroControl::on_animationComboBox_currentIndexChanged(int index) {

	Animation* animation = nullptr;	// Stores the new Animation
	if (active_section_controller_->get_section()->get_animation() != nullptr) {
		// Only change if the animation is different
		if (active_section_controller_->get_section()->get_animation()->get_type() == index) {
			return;
		}
	}

	// Preserve the animation cycle between changes
	bool preserve_cycle_index = true;

	switch((Animation::Type)index) {
		case Animation::Type::Solid:
			animation = active_section_controller_->get_section()->set_animation(new SolidAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Blink:
			animation = active_section_controller_->get_section()->set_animation(new BlinkAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Cycle:
			animation = active_section_controller_->get_section()->set_animation(new CycleAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Wave:
			animation = active_section_controller_->get_section()->set_animation(new WaveAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Merge:
			animation = active_section_controller_->get_section()->set_animation(new MergeAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Random:
			animation = active_section_controller_->get_section()->set_animation(new RandomAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Sparkle:
			animation = active_section_controller_->get_section()->set_animation(new SparkleAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Radial:
			animation = active_section_controller_->get_section()->set_animation(new RadialAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Mandelbrot:
			animation = active_section_controller_->get_section()->set_animation(new MandelbrotAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Plasma:
			animation = active_section_controller_->get_section()->set_animation(new PlasmaAnimation(), preserve_cycle_index);
			break;
		case Animation::Type::Lightning:
			animation = active_section_controller_->get_section()->set_animation(new LightningAnimation(), preserve_cycle_index);
			break;
		default:
			return;
	}

	show_extra_controls(animation);

	if (serial_enabled_) {
		section_handler->set_animation(0, active_section_controller_->is_overlay_, (Animation::Type)index, preserve_cycle_index, nullptr, 0);
		send_to_device(controller_->get_cue(), controller_->get_cue_size());
	}

	// Reapply animation settings
	on_colorComboBox_currentIndexChanged(ui->colorComboBox->currentIndex());
	animation->set_orientation((Animation::Orientation)ui->orientationComboBox->currentIndex());
	animation->set_fade(ui->fadeCheckBox->isChecked());
	animation->set_reverse(ui->reverse_animationCheckBox->isChecked());
	animation->set_speed(ui->cycleSlider->maximum() - ui->cycleSlider->value());
}

/**
 * Changes the current Canvas.
 * @param index Index of the new Canvas type.
 */
void MaestroControl::on_canvasComboBox_currentIndexChanged(int index) {
	Canvas* canvas = nullptr;

	// Remove the existing Canvas.
	active_section_controller_->get_section()->remove_canvas();

	// Add the new Canvas
	switch (index) {
		case 1:	// Animation Canvas
			canvas = active_section_controller_->get_section()->add_canvas(CanvasType::Type::AnimationCanvas);
			break;
		case 2: // Color Canvas
			canvas = active_section_controller_->get_section()->add_canvas(CanvasType::Type::ColorCanvas);
			break;
	}

	if (serial_enabled_) {
		section_handler->add_canvas(0, active_section_controller_->is_overlay_, (CanvasType::Type)index);
		send_to_device(controller_->get_cue(), controller_->get_cue_size());
	}

	show_canvas_controls(canvas);
}

/**
 * Changes the color scheme.
 * If 'Custom' is selected, this also displays controls for adjusting the custom color scheme.
 * @param index Index of the new color scheme.
 */
void MaestroControl::on_colorComboBox_currentIndexChanged(int index) {
	active_section_controller_->mc_color_scheme_ = index;
	switch (index) {
		case 0:	// Custom
			on_custom_color_changed();
			set_custom_color_controls_visible(true);
			break;
		case 1:	// Fire
			{
				uint8_t num_colors = 14;
				Colors::RGB tmp[num_colors];
				Colors::generate_scaling_color_array(tmp, &Colors::RED, &Colors::YELLOW, num_colors, true);
				active_section_controller_->get_section()->get_animation()->set_colors(tmp, num_colors);
				set_custom_color_controls_visible(false);

				if (serial_enabled_) {
					animation_handler->set_colors(0, active_section_controller_->is_overlay_, tmp, num_colors);
					send_to_device(controller_->get_cue(), controller_->get_cue_size());
				}
				break;
			}
		case 2:	// Deep Sea
			{
				uint8_t num_colors = 14;
				Colors::RGB tmp[num_colors];
				Colors::generate_scaling_color_array(tmp, &Colors::BLUE, &Colors::GREEN, num_colors, true);
				active_section_controller_->get_section()->get_animation()->set_colors(tmp, num_colors);
				set_custom_color_controls_visible(false);

				if (serial_enabled_) {
					animation_handler->set_colors(0, active_section_controller_->is_overlay_, tmp, num_colors);
					send_to_device(controller_->get_cue(), controller_->get_cue_size());
				}
				break;
			}
		default:// Color Wheel
			active_section_controller_->get_section()->get_animation()->set_colors(Colors::COLORWHEEL, 12);
			set_custom_color_controls_visible(false);

			if (serial_enabled_) {
				animation_handler->set_colors(0, active_section_controller_->is_overlay_, Colors::COLORWHEEL, 12);
				send_to_device(controller_->get_cue(), controller_->get_cue_size());
			}
	}
}

/**
 * Changes the number of columns in the display grid.
 */
void MaestroControl::on_columnsSpinBox_editingFinished() {
	on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
}

/**
 * Changes the custom color scheme.
 */
void MaestroControl::on_custom_color_changed() {
	// Verify that the custom color scheme option is selected, and that the color is different from the one used in the Section.
	if (ui->colorComboBox->currentIndex() != 0) {
		return;
	}

	Colors::RGB new_color = {
		(uint8_t)ui->redSlider->value(),
		(uint8_t)ui->greenSlider->value(),
		(uint8_t)ui->blueSlider->value()
	};

	if (!active_section_controller_->get_section()->get_animation() || !active_section_controller_->get_section()->get_animation()->get_colors()) {
		return;
	}

	if (active_section_controller_->get_section()->get_animation()->get_num_colors() == 0 || (new_color != *active_section_controller_->get_section()->get_animation()->get_color_at_index(0))) {
		change_scaling_color_array(new_color);

		ui->baseColorPreviewLabel->setText(QString("{%1, %2, %3}").arg(new_color.r).arg(new_color.g).arg(new_color.b));
		ui->baseColorPreviewLabel->setStyleSheet(QString("QLabel { color: rgb(%1, %2, %3); font-weight: bold; }").arg(new_color.r).arg(new_color.g).arg(new_color.b));
	}
}

/**
 * Changes the cycle speed.
 * @param value New cycle speed.
 */
void MaestroControl::on_cycleSlider_valueChanged(int value) {
	if (value != active_section_controller_->get_section()->get_animation()->get_speed()) {
		value = ui->cycleSlider->maximum() - value;
		active_section_controller_->get_section()->get_animation()->set_speed(value);
		ui->cycleSlider->setToolTip(QString::number(value));

		if (serial_enabled_) {
			animation_handler->set_speed(0, active_section_controller_->is_overlay_, value, 0);
			send_to_device(controller_->get_cue(), controller_->get_cue_size());
		}
	}
}

/**
 * Handles changes to the blue custom color slider.
 * @param value New value of the blue slider.
 */
void MaestroControl::on_blueSlider_valueChanged(int value) {
	on_custom_color_changed();
}

/**
 * Toggles fading.
 * @param checked If true, fading is enabled.
 */
void MaestroControl::on_fadeCheckBox_toggled(bool checked) {
	active_section_controller_->get_section()->get_animation()->set_fade(checked);

	if (serial_enabled_) {
		animation_handler->set_fade(0, active_section_controller_->is_overlay_, checked);
		send_to_device(controller_->get_cue(), controller_->get_cue_size());
	}
}

/**
 * Handles changes to the green custom color slider.
 * @param value New value of the green slider.
 */
void MaestroControl::on_greenSlider_valueChanged(int value) {
	on_custom_color_changed();
}

/**
 * Changes the Overlay's mix mode.
 * @param index
 */
void MaestroControl::on_mix_modeComboBox_currentIndexChanged(int index) {
	if ((Colors::MixMode)index != maestro_controller_->get_section_controller(0)->get_overlay()->mix_mode) {
		if (maestro_controller_->get_section_controller(0)->get_overlay_controller()) {
			maestro_controller_->get_section_controller(0)->get_overlay()->mix_mode = (Colors::MixMode)index;

			// Show/hide spin box for alpha only
			if (ui->mix_modeComboBox->currentText().contains("Alpha")) {
				ui->alphaSpinBox->setVisible(true);
			}
			else {
				ui->alphaSpinBox->setVisible(false);
			}

			if (serial_enabled_) {
				section_handler->add_overlay(0, active_section_controller_->is_overlay_, (Colors::MixMode)index, ui->alphaSpinBox->value());
				send_to_device(controller_->get_cue(), controller_->get_cue_size());
			}
		}
	}
}

/**
 * Sets the number of colors in the color scheme.
 * @param arg1 New color count.
 */
void MaestroControl::on_num_colorsSpinBox_valueChanged(int arg1) {
	on_custom_color_changed();
}

/**
 * Sets the animation's orientation
 * @param index New orientation.
 */
void MaestroControl::on_orientationComboBox_currentIndexChanged(int index) {
	if ((Animation::Orientation)index != active_section_controller_->get_section()->get_animation()->get_orientation()) {
		if (active_section_controller_->get_section()->get_animation()) {
			active_section_controller_->get_section()->get_animation()->set_orientation((Animation::Orientation)index);

			if (serial_enabled_) {
				animation_handler->set_orientation(0, active_section_controller_->is_overlay_, (Animation::Orientation)index);
				send_to_device(controller_->get_cue(), controller_->get_cue_size());
			}
		}
	}
}

/**
 * Handles changes to the red custom color slider.
 * @param value New value of the red slider.
 */
void MaestroControl::on_redSlider_valueChanged(int value) {
	on_custom_color_changed();
}

/**
 * Toggles whether the color animation is shown in reverse.
 * @param checked If true, reverse the animation.
 */
void MaestroControl::on_reverse_animationCheckBox_toggled(bool checked) {
	active_section_controller_->get_section()->get_animation()->set_reverse(checked);

	if (serial_enabled_) {
		animation_handler->set_reverse(0, active_section_controller_->is_overlay_, checked);
		send_to_device(controller_->get_cue(), controller_->get_cue_size());
	}
}

/**
 * Changes the number of rows in the displayed grid.
 */
void MaestroControl::on_rowsSpinBox_editingFinished() {
	on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
}

void MaestroControl::on_sectionComboBox_currentIndexChanged(const QString &arg1) {
	QString type = arg1.split(" ")[0];

	if(QString::compare(type, "section", Qt::CaseInsensitive) == 0) {
		// Set active controller
		active_section_controller_ = maestro_controller_->get_section_controller(0);

		// Hide Overlay controls
		this->set_overlay_controls_visible(false);
	}
	else {	// Overlay
		// Set active controller to OverlayController
		active_section_controller_ = active_section_controller_->get_overlay_controller();

		// Show Overlay controls
		set_overlay_controls_visible(true);
	}

	get_section_settings();
}

/**
 * Sets the variance of the colors in the color scheme.
 * @param arg1 New variance between colors (0-255).
 */
void MaestroControl::on_thresholdSpinBox_valueChanged(int arg1) {
	on_custom_color_changed();
}

/**
 * Sets the size of the active SectionController.
 * @param x Number of rows.
 * @param y Number of columns.
 */
void MaestroControl::on_section_resize(uint16_t x, uint16_t y) {
	// Check the Canvas
	if (canvas_control_widget_ != nullptr) {
		CanvasControl* widget = qobject_cast<CanvasControl*>(canvas_control_widget_.get());
		if (!widget->confirm_clear()) {
			return;
		}
	}

	if ((x != active_section_controller_->get_section()->get_dimensions()->x) || (y != active_section_controller_->get_section()->get_dimensions()->y)) {
		active_section_controller_->get_section()->set_dimensions(x, y);

		if (serial_enabled_) {
			section_handler->set_dimensions(0, active_section_controller_->is_overlay_, ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
			send_to_device(controller_->get_cue(), controller_->get_cue_size());
		}
	}
}

void MaestroControl::send_to_device(uint8_t* out, uint8_t size) {
	serial_port_.write_some(boost::asio::buffer((void*)out, size));
}

/**
 * Toggles the visibility of the custom color scheme controls.
 * @param visible If true, display custom controls.
 */
void MaestroControl::set_custom_color_controls_visible(bool visible) {
	ui->baseColorLabel->setVisible(visible);
	ui->baseColorPreviewLabel->setVisible(visible);
	ui->redSlider->setVisible(visible);
	ui->greenSlider->setVisible(visible);
	ui->blueSlider->setVisible(visible);
	ui->num_colorsSpinBox->setVisible(visible);
	ui->num_colorsLabel->setVisible(visible);
	ui->thresholdSpinBox->setVisible(visible);
	ui->thresholdLabel->setVisible(visible);
}

/**
 * Sets the visibility of Overlay-related controls.
 * @param visible True if you want to show the controls.
 */
void MaestroControl::set_overlay_controls_visible(bool visible) {
	// If visible, show Overlay controls
	ui->mixModeLabel->setVisible(visible);
	ui->mix_modeComboBox->setVisible(visible);
	ui->alphaSpinBox->setVisible(visible);

	// Invert layout controls
	ui->gridSizeLabel->setVisible(!visible);
	ui->columnsSpinBox->setVisible(!visible);
	ui->rowsSpinBox->setVisible(!visible);
}

/**
 * Displays extra controls for animations that take custom parameters.
 * @param index Index of the animation in the animations list.
 * @param animation Pointer to the animation.
 */
void MaestroControl::show_extra_controls(Animation* animation) {
	// First, remove any existing extra control widgets
	if (extra_control_widget_ != nullptr) {
		this->findChild<QLayout*>("extraControlsLayout")->removeWidget(extra_control_widget_.get());
		extra_control_widget_.reset();
	}

	QLayout* layout = this->findChild<QLayout*>("extraControlsLayout");

	switch(animation->get_type()) {
		case Animation::Type::Sparkle:
			extra_control_widget_ = std::unique_ptr<QWidget>(new SparkleAnimationControl((SparkleAnimation*)animation, layout->widget()));
			break;
		case Animation::Type::Plasma:
			extra_control_widget_ = std::unique_ptr<QWidget>(new PlasmaAnimationControl((PlasmaAnimation*)animation, layout->widget()));
			break;
		case Animation::Type::Lightning:
			extra_control_widget_ = std::unique_ptr<QWidget>(new LightningAnimationControl((LightningAnimation*)animation, layout->widget()));
			break;
		default:
			break;
	}

	if (extra_control_widget_) {
		layout->addWidget(extra_control_widget_.get());
	}
}

void MaestroControl::show_canvas_controls(Canvas *canvas) {
	QLayout* layout = this->findChild<QLayout*>("canvasControlsLayout");

	// Remove the Canvas controls.
	// If a Canvas is set, re-initialize and re-add the controls.
	layout->removeWidget(canvas_control_widget_.get());
	canvas_control_widget_.reset();

	if (canvas != nullptr) {
		canvas_control_widget_ = std::unique_ptr<QWidget>(new CanvasControl(canvas));
		layout->addWidget(canvas_control_widget_.get());
	}
}

/**
 * Destructor.
 */
MaestroControl::~MaestroControl() {
	if (serial_enabled_) {
		serial_port_.close();
	}
	delete ui;
}
