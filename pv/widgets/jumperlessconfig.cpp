/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2024 Kevin Santo Cappuccio <kevin@jumperless.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "jumperlessconfig.hpp"

#include <QDebug>
#include <QString>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>

#include <pv/devices/device.hpp>

#include <libsigrokcxx/libsigrokcxx.hpp>

using std::shared_ptr;
using sigrok::ConfigKey;
using sigrok::Error;

namespace pv {
namespace widgets {

JumperlessConfig::JumperlessConfig(QWidget *parent) :
	QWidget(parent),
	device_(),
	layout_(nullptr),
	channels_label_(nullptr),
	channels_status_(nullptr),
	trigger_label_(nullptr),
	trigger_type_combo_(nullptr),
	trigger_channel_spin_(nullptr),
	trigger_voltage_spin_(nullptr),
	debug_checkbox_(nullptr),
	update_timer_(nullptr),
	updating_(false),
	last_digital_enabled_(-1),
	last_analog_enabled_(-1)
{
	// Create widgets with proper parent
	layout_ = new QHBoxLayout();
	// Remove mode dropdown - firmware determines mode automatically from enabled channels
	// mode_label_ = new QLabel(tr("Mode:"));
	// capture_mode_combo_ = new QComboBox();
	
	channels_label_ = new QLabel(tr("Channels:"));
	channels_status_ = new QLabel(tr("None"));
	trigger_label_ = new QLabel(tr("Trigger:"));
	trigger_type_combo_ = new QComboBox();
	trigger_channel_spin_ = new QSpinBox();
	trigger_voltage_spin_ = new QDoubleSpinBox();
	debug_checkbox_ = new QCheckBox(tr("Dbg"));
	
	// Remove capture mode combo configuration since we're not using it
	// capture_mode_combo_->addItem(tr("Digital"), 0);
	// capture_mode_combo_->addItem(tr("Mixed Signal"), 1);
	// capture_mode_combo_->addItem(tr("Analog"), 2);
	// capture_mode_combo_->setToolTip(tr("Set Jumperless capture mode"));
	// capture_mode_combo_->setMinimumWidth(120);
	
	// Configure channels status label
	channels_status_->setToolTip(tr("Currently enabled channels"));
	channels_status_->setMinimumWidth(80);
	channels_status_->setStyleSheet("QLabel { color:rgb(242, 0, 255); font-weight: bold; }");
	
	// Configure trigger widgets
	trigger_type_combo_->addItem(tr("None"), 0);
	trigger_type_combo_->addItem(tr("Python"), 1);
	trigger_type_combo_->addItem(tr("GPIO"), 2);
	trigger_type_combo_->addItem(tr("Threshold↑"), 4);
	trigger_type_combo_->addItem(tr("Threshold↓"), 5);
	trigger_type_combo_->setToolTip(tr("Select trigger type"));
	trigger_type_combo_->setMinimumWidth(90);
	
	trigger_channel_spin_->setRange(0, 7);
	trigger_channel_spin_->setValue(0);
	trigger_channel_spin_->setToolTip(tr("ADC channel for threshold trigger"));
	trigger_channel_spin_->setMinimumWidth(40);
	trigger_channel_spin_->setVisible(false);  // Hidden by default
	
	trigger_voltage_spin_->setRange(-10.0, 10.0);
	trigger_voltage_spin_->setValue(2.5);
	trigger_voltage_spin_->setDecimals(2);
	trigger_voltage_spin_->setSuffix("V");
	trigger_voltage_spin_->setToolTip(tr("Threshold voltage"));
	trigger_voltage_spin_->setMinimumWidth(70);
	trigger_voltage_spin_->setVisible(false);  // Hidden by default
	
	// Configure debug checkbox
	debug_checkbox_->setToolTip(tr("Enable debug output"));
	
	// Set layout
	setLayout(layout_);
	
	// Add widgets to layout with spacing
	layout_->addWidget(channels_label_);
	layout_->addWidget(channels_status_);
	layout_->addSpacing(10);
	layout_->addWidget(trigger_label_);
	layout_->addWidget(trigger_type_combo_);
	layout_->addWidget(trigger_channel_spin_);
	layout_->addWidget(trigger_voltage_spin_);
	layout_->addSpacing(10);
	layout_->addWidget(debug_checkbox_);
	
	// Set reasonable margins
	layout_->setContentsMargins(5, 2, 5, 2);
	layout_->setSpacing(5);
	
	// Connect signals
	connect(trigger_type_combo_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_trigger_type_changed(int)));
	connect(trigger_channel_spin_, SIGNAL(valueChanged(int)),
		this, SLOT(on_trigger_channel_changed(int)));
	connect(trigger_voltage_spin_, SIGNAL(valueChanged(double)),
		this, SLOT(on_trigger_voltage_changed(double)));
	connect(debug_checkbox_, SIGNAL(toggled(bool)),
		this, SLOT(on_debug_enabled_changed(bool)));
	
	// Set up periodic update timer for channel status
	update_timer_ = new QTimer(this);
	connect(update_timer_, &QTimer::timeout, this, &JumperlessConfig::update_channel_status);
	update_timer_->start(500); // Update every 500ms
	
	// Always visible for testing - TODO: Change back to device detection later
	setVisible(true);
	
	// Make widget more visible for testing
	//setStyleSheet("QWidget { background-color:#4f293e; border: 2px solidrgb(151, 0, 159); margin: 2px; }");
	
	qDebug() << "JumperlessConfig widget created and set to visible";
}

void JumperlessConfig::set_device(shared_ptr<pv::devices::Device> device)
{
	qDebug() << "JumperlessConfig::set_device called with device:" << (device ? "valid device" : "null device");
	
	device_ = device;
	update_visibility();
}

void JumperlessConfig::update_visibility()
{
	// Always show for testing - TODO: Change back to device detection later
	// bool should_show = is_jumperless_device();
	bool should_show = true;
	setVisible(should_show);
	
	if (should_show) {
		if (is_jumperless_device()) {
			qDebug() << "Jumperless device detected - showing configuration controls";
		} else {
			qDebug() << "Non-Jumperless device - showing widget for testing purposes";
		}
		// Set default values for new device
		updating_ = true;
		// capture_mode_combo_->setCurrentIndex(1); // Mixed Signal default - REMOVED
		// protocol_mode_combo_->setCurrentIndex(1); // Enhanced default - COMMENTED OUT
		debug_checkbox_->setChecked(false);
		updating_ = false;
		
		// Update channel status display
		update_channel_status();
	}
}

// Remove the on_capture_mode_changed function since we don't have mode selection anymore
// void JumperlessConfig::on_capture_mode_changed(int index)

void JumperlessConfig::on_debug_enabled_changed(bool enabled)
{
	if (updating_ || !device_)
		return;
		
	qDebug() << "Jumperless debug mode changed to:" << enabled;
	
	// This could be used to send a debug enable/disable command
	// For now, just emit the signal
	config_changed();
}

void JumperlessConfig::on_device_config_changed()
{
	qDebug() << "JumperlessConfig: Device configuration changed - updating channel status";
	update_channel_status();
}

void JumperlessConfig::on_trigger_type_changed(int index)
{
	if (updating_ || !device_)
		return;
		
	qDebug() << "Jumperless trigger type changed to:" << index;
	
	int trigger_type = trigger_type_combo_->itemData(index).toInt();
	
	// Show/hide trigger parameter widgets based on type
	bool is_threshold = (trigger_type == 4 || trigger_type == 5);
	trigger_channel_spin_->setVisible(is_threshold);
	trigger_voltage_spin_->setVisible(is_threshold);
	
	// Send trigger command to device
	send_trigger_command();
	
	config_changed();
}

void JumperlessConfig::on_trigger_channel_changed(int value)
{
	if (updating_ || !device_)
		return;
		
	qDebug() << "Jumperless trigger channel changed to:" << value;
	
	// Send updated trigger command
	send_trigger_command();
	
	config_changed();
}

void JumperlessConfig::on_trigger_voltage_changed(double value)
{
	if (updating_ || !device_)
		return;
		
	qDebug() << "Jumperless trigger voltage changed to:" << value;
	
	// Send updated trigger command
	send_trigger_command();
	
	config_changed();
}

void JumperlessConfig::send_trigger_command()
{
	if (!device_) {
		qWarning() << "Cannot send trigger command - no device";
		return;
	}
	
	const shared_ptr<sigrok::Device> sr_dev = device_->device();
	if (!sr_dev) {
		qWarning() << "Cannot send trigger command - no sigrok device";
		return;
	}
	
	int trigger_type = trigger_type_combo_->currentData().toInt();
	uint32_t mask = 0;
	uint32_t pattern = 0;
	
	if (trigger_type == 4 || trigger_type == 5) {  // Threshold triggers
		// For threshold triggers, use mask as channel and pattern as voltage
		mask = static_cast<uint32_t>(trigger_channel_spin_->value());
		
		// Convert voltage to uint32_t for transmission
		float voltage = static_cast<float>(trigger_voltage_spin_->value());
		union { float f; uint32_t i; } voltage_union;
		voltage_union.f = voltage;
		pattern = voltage_union.i;
	}
	
	qDebug() << "Sending trigger command: type=" << trigger_type 
	         << "mask=0x" << QString::number(mask, 16)
	         << "pattern=0x" << QString::number(pattern, 16);
	
	// Note: This would need to be implemented as a custom device command
	// For now, just log the command that would be sent
	qDebug() << "Trigger command would be sent to firmware via custom protocol";
}

void JumperlessConfig::send_mode_command(uint8_t mode)
{
	if (!device_) {
		qWarning() << "Cannot send mode command - no device";
		return;
	}
	
	const shared_ptr<sigrok::Device> sr_dev = device_->device();
	if (!sr_dev) {
		qWarning() << "Cannot send mode command - no sigrok device";
		return;
	}
	
	try {
		// Use SR_CONF_DEVICE_MODE if the driver supports it
		if (sr_dev->config_check(ConfigKey::DEVICE_MODE, sigrok::Capability::SET)) {
			// Use strings that match the libsigrok driver expectation
			const char* mode_strings[] = {"digital-only", "mixed-signal", "analog-only"};
			if (mode < 3) {
				// Create string variant explicitly to ensure correct type
				std::string mode_str(mode_strings[mode]);
				auto mode_variant = Glib::Variant<Glib::ustring>::create(mode_str);
				sr_dev->config_set(ConfigKey::DEVICE_MODE, mode_variant);
				qDebug() << "Sent mode command via SR_CONF_DEVICE_MODE:" << mode_strings[mode] << "(variant type: string)";
			}
		} else {
			qDebug() << "Device doesn't support SR_CONF_DEVICE_MODE, mode may need to be set differently";
		}
	} catch (const Error& e) {
		qWarning() << "Failed to send mode command:" << e.what();
	}
}

void JumperlessConfig::update_channel_status()
{
	if (!device_) {
		channels_status_->setText(tr("No Device"));
		return;
	}
	
	const shared_ptr<sigrok::Device> sr_dev = device_->device();
	if (!sr_dev) {
		channels_status_->setText(tr("No Device"));
		return;
	}
	
	try {
		auto channels = sr_dev->channels();
		int digital_enabled = 0;
		int analog_enabled = 0;
		QStringList analog_names;
		
		for (auto channel : channels) {
			if (channel->enabled()) {
				if (channel->type()->id() == SR_CHANNEL_LOGIC) {
					digital_enabled++;
				} else if (channel->type()->id() == SR_CHANNEL_ANALOG) {
					analog_enabled++;
					analog_names << QString::fromStdString(channel->name()).split(' ').first();
				}
			}
		}
		
		// Check if channel configuration has changed
		bool channels_changed = false;
		if (last_digital_enabled_ != digital_enabled || last_analog_enabled_ != analog_enabled) {
			channels_changed = true;
			qDebug() << "Channel configuration changed: Digital" << last_digital_enabled_ << "->" << digital_enabled
			         << "Analog" << last_analog_enabled_ << "->" << analog_enabled;
			
			// Update stored values
			last_digital_enabled_ = digital_enabled;
			last_analog_enabled_ = analog_enabled;
		}
		
		QString status;
		if (digital_enabled > 0 && analog_enabled > 0) {
			status = tr("D:%1 A:%2").arg(digital_enabled).arg(analog_enabled);
			if (analog_enabled <= 3) {
				status += QString(" (%1)").arg(analog_names.join(","));
			}
		} else if (digital_enabled > 0) {
			status = tr("D:%1").arg(digital_enabled);
		} else if (analog_enabled > 0) {
			status = tr("A:%1").arg(analog_enabled);
			if (analog_enabled <= 3) {
				status += QString(" (%1)").arg(analog_names.join(","));
			}
		} else {
			status = tr("None");
		}
		
		channels_status_->setText(status);
		
		// If channels changed, request updated header from device
		if (channels_changed && is_jumperless_device()) {
			qDebug() << "Requesting updated header due to channel configuration change";
			request_device_header_update();
		}
		
	} catch (const Error& e) {
		qWarning() << "Failed to read channel status:" << e.what();
		channels_status_->setText(tr("Error"));
	}
}

void JumperlessConfig::send_enhanced_config()
{
	if (!device_) {
		qWarning() << "Cannot send enhanced config - no device";
		return;
	}
	
	qDebug() << "Enabling enhanced protocol mode";
	
	// The enhanced protocol is typically enabled by sending specific commands
	// This might be handled automatically by the driver when mixed-signal mode is selected
	// For now, we'll just log that enhanced mode is requested
}

void JumperlessConfig::request_device_header_update()
{
	if (!device_) {
		qDebug() << "Cannot update device configuration - no device";
		return;
	}
	
	try {
		// Get current device mode to trigger reconfiguration
		auto sr_dev = device_->device();
		if (!sr_dev) {
			qWarning() << "Cannot access sigrok device for reconfiguration";
			return;
		}
		
		// Get current device mode
		string current_mode = "mixed-signal";  // Default to mixed-signal
		try {
			auto mode_var = sr_dev->config_get(ConfigKey::DEVICE_MODE);
			current_mode = Glib::VariantBase::cast_dynamic<Glib::Variant<string>>(mode_var).get();
		} catch (...) {
			// If we can't get current mode, use mixed-signal as default
			qDebug() << "Using default mixed-signal mode for reconfiguration";
		}
		
		qDebug() << "Triggering device reconfiguration with mode:" << QString::fromStdString(current_mode);
		
		// Trigger reconfiguration by setting the device mode (this forces SET_CHANNELS command)
		auto mode_variant = Glib::Variant<Glib::ustring>::create(current_mode);
		sr_dev->config_set(ConfigKey::DEVICE_MODE, mode_variant);
		
		qDebug() << "Device reconfiguration triggered successfully";
		
	} catch (const Error& e) {
		qWarning() << "Failed to trigger device reconfiguration:" << e.what();
	}
}



bool JumperlessConfig::is_jumperless_device() const
{
	if (!device_) {
		return false;
	}
	
	const shared_ptr<sigrok::Device> sr_dev = device_->device();
	if (!sr_dev) {
		return false;
	}
	
	// Check if this is a Jumperless device by examining the vendor/model
	QString vendor = QString::fromStdString(sr_dev->vendor());
	QString model = QString::fromStdString(sr_dev->model());
	
	// Look for Jumperless-specific identifiers
	bool is_jumperless = vendor.contains("Jumperless", Qt::CaseInsensitive) ||
	                    model.contains("Jumperless", Qt::CaseInsensitive) ||
	                    model.contains("Mixed-Signal", Qt::CaseInsensitive);
	
	if (is_jumperless) {
		qDebug() << "Detected Jumperless device:" << vendor << model;
	}
	
	return is_jumperless;
}

}  // namespace widgets
}  // namespace pv 