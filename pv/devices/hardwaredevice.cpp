/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <boost/algorithm/string/join.hpp>

#include <QString>
#include <string>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <pv/devicemanager.hpp>

#include "hardwaredevice.hpp"

using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;

using boost::algorithm::join;

using sigrok::HardwareDevice;

namespace pv {
namespace devices {

HardwareDevice::HardwareDevice(const shared_ptr<sigrok::Context> &context,
	shared_ptr<sigrok::HardwareDevice> device) :
	context_(context),
	device_open_(false)
{
	device_ = device;
}

HardwareDevice::~HardwareDevice()
{
	close();
}

string HardwareDevice::full_name() const
{
	vector<string> parts = {};
	
	// Check if device is valid
	if (!device_) {
		return "(Invalid Device)";
	}
	
	try {
		if (device_->vendor().length() > 0)
			parts.push_back(device_->vendor());
	} catch (const sigrok::Error &e) {
		// Ignore vendor errors
	} catch (...) {
		// Ignore any other vendor errors
	}
	
	try {
		if (device_->model().length() > 0)
			parts.push_back(device_->model());
	} catch (const sigrok::Error &e) {
		// Ignore model errors
	} catch (...) {
		// Ignore any other model errors
	}
	
	try {
		if (device_->version().length() > 0)
			parts.push_back(device_->version());
	} catch (const sigrok::Error &e) {
		// Ignore version errors
	} catch (...) {
		// Ignore any other version errors
	}
	
	try {
		if (device_->serial_number().length() > 0)
			parts.push_back("[S/N: " + device_->serial_number() + "]");
	} catch (const sigrok::Error &e) {
		// Ignore serial number errors
	} catch (...) {
		// Ignore any other serial number errors
	}
	
	try {
		if (device_->connection_id().length() > 0)
			parts.push_back("(" + device_->connection_id() + ")");
	} catch (const sigrok::Error &e) {
		// Ignore connection ID errors
	} catch (...) {
		// Ignore any other connection ID errors
	}
	
	if (parts.empty()) {
		return "(Disconnected Device)";
	}
	
	return join(parts, " ");
}

shared_ptr<sigrok::HardwareDevice> HardwareDevice::hardware_device() const
{
	return static_pointer_cast<sigrok::HardwareDevice>(device_);
}

string HardwareDevice::display_name(
	const DeviceManager &device_manager) const
{
	// Check if device is valid
	if (!device_) {
		return "(Invalid Device)";
	}
	
	const auto hw_dev = hardware_device();

	// If we can find another device with the same model/vendor then
	// we have at least two such devices and need to distinguish them.
	const auto &devices = device_manager.devices();
	const bool multiple_dev = hw_dev && any_of(
		devices.begin(), devices.end(),
		[&](shared_ptr<devices::HardwareDevice> dev) {
			try {
				return dev->hardware_device()->vendor() ==
						hw_dev->vendor() &&
					dev->hardware_device()->model() ==
						hw_dev->model() &&
					dev->device_ != device_;
			} catch (...) {
				return false; // Skip devices that can't be accessed
			}
		});

	vector<string> parts = {};
	
	try {
		if (device_->vendor().length() > 0)
			parts.push_back(device_->vendor());
	} catch (...) {
		// Ignore vendor errors
	}
	
	try {
		if (device_->model().length() > 0)
			parts.push_back(device_->model());
	} catch (...) {
		// Ignore model errors
	}

	if (multiple_dev) {
		try {
			if (device_->version().length() > 0)
				parts.push_back(device_->version());
		} catch (...) {
			// Ignore version errors
		}
		
		try {
			if (device_->serial_number().length() > 0)
				parts.push_back("[S/N: " + device_->serial_number() + "]");
		} catch (...) {
			// Ignore serial number errors
		}

		try {
			if ((device_->serial_number().length() == 0) &&
				(device_->connection_id().length() > 0))
				parts.push_back("(" + device_->connection_id() + ")");
		} catch (...) {
			// Ignore connection ID errors
		}
	}

	if (parts.empty()) {
		return "(Disconnected Device)";
	}

	return join(parts, " ");
}

void HardwareDevice::open()
{
	if (device_open_)
		close();

	try {
		device_->open();
	} catch (const sigrok::Error &e) {
		throw QString(e.what());
	}

	device_open_ = true;

	// Set up the session
	session_ = context_->create_session();
	session_->add_device(device_);
}

void HardwareDevice::close()
{
	if (device_open_)
		device_->close();

	if (session_)
		session_->remove_devices();

	device_open_ = false;
}

} // namespace devices
} // namespace pv
