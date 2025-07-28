# Jumperless Configuration Widget

This document describes the custom Jumperless configuration widget that has been added to PulseView to provide easy access to Jumperless-specific settings.

## Overview

The Jumperless configuration widget appears in the PulseView main toolbar when a Jumperless device is connected. It provides controls for:

- **Capture Mode**: Select between Digital Only, Mixed Signal, or Analog Only modes
- **Protocol Mode**: Choose between SUMP (for digital-only compatibility) or Enhanced (for mixed-signal features)
- **Analog Channels**: Configure the number of analog channels to enable (1-8)
- **Debug Mode**: Enable/disable debug output

## Features

### Automatic Device Detection

The widget automatically detects when a Jumperless device is connected by checking the device vendor and model strings. It only appears when a Jumperless device is active.

### Mode Configuration

- **Digital Only**: Enables only digital channels, uses SUMP protocol
- **Mixed Signal**: Enables both digital and analog channels, requires Enhanced protocol
- **Analog Only**: Enables only analog channels, uses Enhanced protocol

### Protocol Selection

- **SUMP**: Compatible with standard SUMP/OLS logic analyzer tools
- **Enhanced**: Provides Jumperless-specific features like analog capture and advanced triggering

### Integration with LibSigrok

The widget communicates with the Jumperless device through the libsigrok driver using:

- `SR_CONF_DEVICE_MODE` for capture mode selection
- Channel enable/disable for analog channel configuration
- Standard libsigrok configuration mechanisms

## Usage

1. Connect your Jumperless device to your computer
2. Open PulseView and scan for devices
3. Select the Jumperless device from the device dropdown
4. The Jumperless configuration widget will appear in the toolbar
5. Configure your desired settings:
   - Choose capture mode based on your needs
   - Select Enhanced protocol for mixed-signal features
   - Set the number of analog channels if using analog capture
6. Start your capture session

## Technical Details

### Files Added

- `pv/widgets/jumperlessconfig.hpp` - Widget header file
- `pv/widgets/jumperlessconfig.cpp` - Widget implementation
- Integration into `pv/toolbars/mainbar.hpp` and `pv/toolbars/mainbar.cpp`
- Updated `CMakeLists.txt` to include new files

### Driver Communication

The widget sends configuration commands to the libsigrok driver using:

```cpp
// Set capture mode
sr_dev->config_set(ConfigKey::DEVICE_MODE, mode_variant);

// Enable/disable channels
channel->set_enabled(should_enable);
```

### Mode Strings

The widget uses mode strings that match the libsigrok driver expectations:
- "digital-only"
- "mixed-signal" 
- "analog-only"

## Troubleshooting

If the widget doesn't appear:

1. Verify your Jumperless device is properly connected
2. Check that the device is recognized in the device dropdown
3. Ensure you're using the Jumperless-compatible libsigrok driver
4. Check the console output for device detection messages

If configuration changes don't take effect:

1. Verify the Enhanced protocol is selected for mixed-signal mode
2. Check that the libsigrok driver supports `SR_CONF_DEVICE_MODE`
3. Monitor debug output to see if commands are being sent
4. Ensure the Jumperless firmware is compatible with the selected mode

## Future Enhancements

Potential improvements for this widget include:

- Trigger configuration controls
- Sample buffer management
- Real-time status display
- Firmware version checking
- Advanced protocol options

The widget provides a foundation that can be extended with additional Jumperless-specific features as needed. 