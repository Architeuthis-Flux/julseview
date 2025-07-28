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

#ifndef PULSEVIEW_PV_WIDGETS_JUMPERLESSCONFIG_HPP
#define PULSEVIEW_PV_WIDGETS_JUMPERLESSCONFIG_HPP

#include <memory>

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimer>

using std::shared_ptr;

namespace sigrok {
class Device;
}

namespace pv {
namespace devices {
class Device;
}

namespace widgets {

class JumperlessConfig : public QWidget
{
	Q_OBJECT

public:
	JumperlessConfig(QWidget *parent = nullptr);

	void set_device(shared_ptr<pv::devices::Device> device);
	void update_visibility();

Q_SIGNALS:
	void config_changed();

private Q_SLOTS:
	// Remove mode-related slot - firmware determines mode from enabled channels
	// void on_capture_mode_changed(int index);
	void on_trigger_type_changed(int index);
	void on_trigger_channel_changed(int value);
	void on_trigger_voltage_changed(double value);
	void on_debug_enabled_changed(bool enabled);
	void on_device_config_changed();

private:
	void send_mode_command(uint8_t mode);
	void update_channel_status();
	void send_enhanced_config();
	void send_trigger_command();
	void request_device_header_update();
	bool is_jumperless_device() const;

	shared_ptr<pv::devices::Device> device_;
	
	QHBoxLayout *layout_;
	// Remove mode-related widgets - firmware determines mode from enabled channels
	// QLabel *mode_label_;
	// QComboBox *capture_mode_combo_;
	QLabel *channels_label_;
	QLabel *channels_status_;
	QLabel *trigger_label_;
	QComboBox *trigger_type_combo_;
	QSpinBox *trigger_channel_spin_;
	QDoubleSpinBox *trigger_voltage_spin_;
	QCheckBox *debug_checkbox_;
	QTimer *update_timer_;
	
	bool updating_;
	
	// Track previous channel configuration to detect changes
	int last_digital_enabled_;
	int last_analog_enabled_;
};

}  // namespace widgets
}  // namespace pv

#endif // PULSEVIEW_PV_WIDGETS_JUMPERLESSCONFIG_HPP 