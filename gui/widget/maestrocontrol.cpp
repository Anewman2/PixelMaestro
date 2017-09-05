#include "maestrocontrol.h"
#include "ui_maestrocontrol.h"
#include "controller/maestrocontroller.h"
#include "controller/sectioncontroller.h"
#include "drawingarea/simpledrawingarea.h"
#include <QPalette>
#include <QString>
#include "section.h"

/**
 * Constructor.
 * @param parent The QWidget containing this controller.
 * @param maestro_controller The MaestroController being controlled.
 */
MaestroControl::MaestroControl(QWidget* parent, MaestroController* maestro_controller) : QWidget(parent), ui(new Ui::MaestroControl) {

	// Assign easy reference variables for the Maestro
	this->maestro_controller_ = maestro_controller;

	// Initialize UI
	ui->setupUi(this);
	this->initialize();
}

/**
 * Applies the active Section settings to the UI.
 */
void MaestroControl::get_section_settings() {
	ui->animationComboBox->setCurrentIndex(active_section_controller_->get_section()->get_color_animation() - 1);
	ui->reverse_animationCheckBox->setChecked(active_section_controller_->get_section()->get_reverse());
	ui->fadeCheckBox->setChecked(active_section_controller_->get_section()->get_fade());
	ui->num_colorsSpinBox->setValue(active_section_controller_->get_section()->get_num_colors());

	ui->cycleSlider->setValue(this->active_section_controller_->get_section()->get_cycle_interval());
	ui->rowsSpinBox->setValue(this->active_section_controller_->get_section()->get_dimensions()->y);
	ui->columnsSpinBox->setValue(this->active_section_controller_->get_section()->get_dimensions()->x);

	QStringList section_type = ui->sectionComboBox->currentText().split(" ");
	if (QString::compare(section_type[0], "overlay", Qt::CaseInsensitive) == 0) {
		ui->mix_modeComboBox->setCurrentIndex(this->maestro_controller_->get_section_controller(section_type[1].toInt() - 1)->get_overlay()->mix_mode);
		ui->alphaSpinBox->setValue(this->maestro_controller_->get_section_controller(section_type[1].toInt() - 1)->get_overlay()->alpha);
	}
}

/**
 * Build the initial UI.
 */
void MaestroControl::initialize() {
	active_section_controller_ = maestro_controller_->get_section_controller(0);

	// Populate Animation combo box
	ui->animationComboBox->addItems({"Solid", "Blink", "Cycle", "Wave", "Pong", "Merge", "Random", "Sparkle"});

	// Populate color combo box
	ui->colorComboBox->addItems({"Custom", "Fire", "Deep Sea", "Color Wheel"});
	ui->colorComboBox->setCurrentIndex(2);
	this->set_custom_color_controls_visible(false);

	// Set default values
	ui->sectionComboBox->addItem("Section 1");


	// Add an Overlay
	active_section_controller_->add_overlay(Colors::MixMode::NONE);
	ui->sectionComboBox->addItem(QString("Overlay 1"));

	// Initialize Overlay controls
	ui->mix_modeComboBox->addItems({"None", "Normal", "Alpha Blending", "Multiply"});

	get_section_settings();
}

/**
 * Updates the color array based on changes to the color scheme and settings.
 * @param color Base color to use when generating the array.
 */
void MaestroControl::change_scaling_color_array(Colors::RGB color) {
	unsigned int num_colors = (unsigned int)ui->num_colorsSpinBox->value();

	std::vector<Colors::RGB> tmp_colors;
	tmp_colors.resize(num_colors);

	unsigned char threshold = 255 - (unsigned char)ui->thresholdSpinBox->value();
	Colors::generate_scaling_color_array(&tmp_colors[0], &color, num_colors, threshold, true);
	active_section_controller_->set_controller_colors(&tmp_colors[0], num_colors);

	// Release tmp_colors
	std::vector<Colors::RGB>().swap(tmp_colors);
}

/**
 * Sets the Overlay's transparency level.
 * @param arg1 Transparency level from 0 - 1.
 */
void MaestroControl::on_alphaSpinBox_valueChanged(double arg1) {
	maestro_controller_->get_section_controller(0)->get_overlay()->alpha = arg1;
}

/**
 * Changes the current animation.
 * @param index Index of the new animation.
 */
void MaestroControl::on_animationComboBox_currentIndexChanged(int index) {
	active_section_controller_->get_section()->set_color_animation((Section::ColorAnimations)(index + 1), ui->reverse_animationCheckBox->isChecked());
}

/**
 * Changes the color scheme.
 * If 'Custom' is selected, this also displays controls for adjusting the custom color scheme.
 * @param index Index of the new color scheme.
 */
void MaestroControl::on_colorComboBox_currentIndexChanged(int index) {
	switch (index) {
		case 0:	// Custom
			on_custom_color_changed();
			set_custom_color_controls_visible(true);
			break;
		case 1:	// Fire
			{
				unsigned char num_colors = 14;
				Colors::RGB fire[num_colors];
				Colors::generate_scaling_color_array(fire, &Colors::RED, &Colors::ORANGE, num_colors, true);
				active_section_controller_->set_controller_colors(fire, num_colors);
				set_custom_color_controls_visible(false);
				break;
			}
		case 2:	// Deep Sea
			{
				unsigned char num_colors = 14;
				Colors::RGB deep_sea[num_colors];
				Colors::generate_scaling_color_array(deep_sea, &Colors::BLUE, &Colors::GREEN, num_colors, true);
				active_section_controller_->set_controller_colors(deep_sea, num_colors);
				set_custom_color_controls_visible(false);
				break;
			}
		default:// Color Wheel
			active_section_controller_->get_section()->set_colors(Colors::COLORWHEEL, 12);
			set_custom_color_controls_visible(false);
	}
}

/**
 * Changes the number of columns in the display grid.
 */
void MaestroControl::on_columnsSpinBox_valueChanged(int arg1) {
	this->active_section_controller_->set_dimensions(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());

	// Set Overlay if applicable
	if (active_section_controller_->get_overlay_controller() != nullptr) {
		active_section_controller_->get_overlay_controller()->set_dimensions(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
	}
}


/**
 * Changes the custom color scheme.
 */
void MaestroControl::on_custom_color_changed() {
	if (ui->colorComboBox->currentIndex() != 0) {
		return;
	}

	unsigned char r = ui->redSlider->value();
	unsigned char g = ui->greenSlider->value();
	unsigned char b = ui->blueSlider->value();

	change_scaling_color_array(Colors::RGB {r, g, b});

	ui->baseColorPreviewLabel->setText(QString("{%1, %2, %3}").arg(r).arg(g).arg(b));
	ui->baseColorPreviewLabel->setStyleSheet(QString("QLabel { color: rgb(%1, %2, %3); font-weight: bold; }").arg(r).arg(g).arg(b));
}

/**
 * Changes the cycle speed.
 * @param value New cycle speed.
 */
void MaestroControl::on_cycleSlider_valueChanged(int value) {
	value = ui->cycleSlider->maximum() - value;
	this->active_section_controller_->get_section()->set_cycle_interval((unsigned short)value);
	ui->cycleSlider->setToolTip(QString::number(value));
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
	active_section_controller_->get_section()->toggle_fade();
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
	if (maestro_controller_->get_section_controller(0)->get_overlay_controller()) {
		maestro_controller_->get_section_controller(0)->get_overlay()->mix_mode = (Colors::MixMode)index;

		// Show/hide spin box for alpha only
		if (index == 2) {
			ui->alphaSpinBox->setVisible(true);
		}
		else {
			ui->alphaSpinBox->setVisible(false);
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
	on_animationComboBox_currentIndexChanged(ui->animationComboBox->currentIndex());
}

/**
 * Changes the number of rows in the display grid.
 * @param arg1 New number of rows.
 */
void MaestroControl::on_rowsSpinBox_valueChanged(int arg1) {
	on_columnsSpinBox_valueChanged(arg1);
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
		active_section_controller_ = active_section_controller_->get_overlay_controller().get();

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
 * Destructor.
 */
MaestroControl::~MaestroControl() {
	delete ui;
}
