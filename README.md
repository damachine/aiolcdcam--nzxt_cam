---
---
# ⚠️ STOP WORKING HERE ⚠️ 

# Please visit: [https://github.com/damachine/coolerdash](https://github.com/damachine/coolerdash) 
---
---

# LCD AIO CAM - For CoolerControl

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)](https://kernel.org/)
[![Development Status](https://img.shields.io/badge/Status-Beta-orange.svg)](https://github.com)
[![Donate BTC](https://img.shields.io/badge/Donate-Bitcoin-f7931a.svg)](bitcoin:13WjpWQMGG5sg3vTJJnCX3cXzwf2vZddKo)
[![Donate DOGE](https://img.shields.io/badge/Donate-Dogecoin-c2a633.svg)](https://dogechain.info/address/DRSY4cA8eCALn819MjWLbwaePFNti9oS3y)

## 📖 Description

**Take full control of your AIO liquid cooling system with integrated LCD display to monitor real-time sensor data in style.**

This high-performance, modular C-based daemon empowers you to harness the potential of your LCD-equipped AIO liquid coolers. Display comprehensive system monitoring data including CPU, GPU, and coolant temperatures directly on your LCD screen through seamless CoolerControl API integration.

Transform your cooling system into an intelligent monitoring hub that keeps you informed about your system's vital signs at a glance.

**🔧 Built with strict C99 compliance** for maximum portability and standards conformance.

---

## ⚠️ **BETA SOFTWARE - EARLY DEVELOPMENT STAGE**

> **🚧 This software is in very early development stage!**  
> - Features may change or break without notice
> - Bugs and issues are expected
> - **Currently tested only on developer's system with NZXT Kraken 2023**
> - **Manual configuration required** - user-friendly setup tools planned for future
> - **Command-line focused** - GUI configuration interface planned
> - **Technical knowledge needed** - simplified installation process coming
> - Use at your own risk and please report any issues
> - Contributions and feedback are highly appreciated!

### 🔮 **Planned User Experience Improvements:**
- **🎛️ Configuration GUI**: Graphical interface for UUID and settings configuration
- **🚀 One-click installer**: Automated setup without manual config editing
- **📱 System tray integration**: Easy mode switching and status monitoring
- **🎨 Theme system**: Visual customization of LCD displays
- **📊 Configuration wizard**: Step-by-step guided setup process

### 📸 **Screenshot - Current LCD Output:**
![CPU/GPU Temperature Display](images/aiolcdcam.png)
*Live temperature monitoring on NZXT AIO LCD display*

---

**👨‍💻 Author:** DAMACHINE ([christkue79@gmail.com](mailto:christkue79@gmail.com))  
**🧪 Tested with:** NZXT AIO Kraken 2023 (Z-Series) - Developer's personal system  
**🔗 Compatible:** NZXT AIO Kraken X-Series, Z-Series and other LCD-capable models *(theoretical)*

## ✨ Features

- **🏗️ Modular Architecture**: Professional separation of CPU, GPU, coolant, and display logic into separate modules
- **⚡ Performance-Optimized**: Caching, change detection, minimal I/O operations, mode-dependent resource usage
- **🎨 4 Display Modes**: From simple temperatures (def) to complex load diagrams (1-3)
- **🔧 Central Configuration**: All settings (UUID, paths, colors, layout) in `include/config.h`
- **🌐 Native CoolerControl Integration**: REST API communication without Python dependencies
- **📊 Efficient Sensor Polling**: Only necessary sensor data is queried depending on mode
- **🔄 Systemd Integration**: Professional service management with detailed logs
- **🚀 Intelligent Installation**: Automatic dependency detection and installation for all major Linux distributions

## 📦 Installation

### Prerequisites

1. **Install CoolerControl**: [Installation Guide](https://gitlab.com/coolercontrol/coolercontrol/-/blob/main/README.md)
2. **Start CoolerControl daemon**: `sudo systemctl start coolercontrold`
3. **Configure your LCD AIO** in CoolerControl GUI
4. **Set LCD to Image mode**: In CoolerControl GUI, set your AIO LCD display to "Image" mode

### Install LCD AIO CAM

#### Arch Linux (Recommended)

```bash
# STEP 1: Clone and configure UUID FIRST
git clone https://github.com/damachine/aiolcdcam.git
cd aiolcdcam

# STEP 2: Start CoolerControl and find your device UUID
# First, ensure CoolerControl daemon is running (if not already started)
sudo systemctl start coolercontrold

# Then find your device UUID
curl http://localhost:11987/devices | jq

# STEP 3: Configure UUID in config.h (REQUIRED)
nano include/config.h  # Set AIO_UUID to your device UUID

# STEP 4: Build and install (includes automatic dependency management)
makepkg -si

# Option 2: Install from AUR (when published)
# yay -S aiolcdcam
# or
# paru -S aiolcdcam
```

#### Manual Installation (All Distributions)

```bash
# STEP 1: Clone repository
git clone https://github.com/damachine/aiolcdcam.git
cd aiolcdcam

# STEP 2: Start CoolerControl and find your device UUID (REQUIRED)
# First, ensure CoolerControl daemon is running (if not already started)
sudo systemctl start coolercontrold

# Then find your device UUID
curl http://localhost:11987/devices | jq

# STEP 3: Configure UUID in config.h BEFORE building (CRITICAL!)
nano include/config.h  # Set AIO_UUID to your device UUID

# STEP 4: Build and install (auto-detects Linux distribution and installs dependencies)
sudo make install

# STEP 5: Enable autostart
sudo systemctl enable aiolcdcam.service

# STEP 6: Start AIOLCDCAM
sudo systemctl start aiolcdcam.service
```

> **⚠️ IMPORTANT**: You MUST configure the UUID in `include/config.h` BEFORE running `sudo make install`, otherwise the daemon will not work!

**Supported Distributions (Auto-Detected):**
- **Arch Linux / Manjaro**: `pacman -S cairo libcurl-gnutls coolercontrol gcc make pkg-config`
- **Ubuntu / Debian**: `apt install libcairo2-dev libcurl4-openssl-dev gcc make pkg-config`
- **Fedora**: `dnf install cairo-devel libcurl-devel gcc make pkg-config`
- **RHEL / CentOS**: `yum install cairo-devel libcurl-devel gcc make pkg-config`
- **openSUSE**: `zypper install cairo-devel libcurl-devel gcc make pkg-config`

## ⚙️ Configuration

### Device UUID Configuration (REQUIRED)

> **⚠️ CRITICAL**: You **MUST** configure your device UUID before first use!

1. **Start CoolerControl (if not running)**: `sudo systemctl start coolercontrold`
2. **Find your device UUID**: `curl http://localhost:11987/devices | jq`
3. **Copy the UUID** from the JSON output (long hexadecimal string)
4. **Edit** `include/config.h` and replace `AIO_UUID` with your device's UUID
5. **Rebuild**: `make clean && sudo make install`

**Example CoolerControl API output:**
```json
{
  "8d4becb03bca2a8e8d4213ac376a1094f39d2786f688549ad3b6a591c3affdf9": {
    "name": "NZXT Kraken",
    "device_type": "Liquidctl",
    "type_index": 0
  }
}
```
> **💡 Tip**: The long string is your device UUID that you need to copy into `include/config.h`

### Display Modes

| Mode | Description | Sensor Data |
|------|-------------|-------------|
| `def` | Temperatures only (CPU, GPU, coolant) | Minimal I/O - temperatures only |
| `1`   | Temperatures + vertical load bars | All sensors + CPU/RAM/GPU load |
| `2`   | Temperatures + circular diagrams | All sensors + CPU/RAM/GPU load |
| `3`   | Temperatures + horizontal load bars | All sensors + CPU/RAM/GPU load |

#### How to Change Display Mode

**Via systemd service (recommended):**
```bash
# Edit systemd service file
sudo systemctl edit aiolcdcam.service

# Add this content (example for mode 2):
[Service]
ExecStart=
ExecStart=/opt/aiolcdcam/bin/aiolcdcam 2

# Apply changes
sudo systemctl daemon-reload
sudo systemctl restart aiolcdcam.service
```

**Manual execution:**
```bash
# Stop service first
sudo systemctl stop aiolcdcam.service

# Run manually with desired mode
./aiolcdcam def      # Temperatures only
./aiolcdcam 1        # Vertical bars
./aiolcdcam 2        # Circular diagrams
./aiolcdcam 3        # Horizontal bars

# Or with --mode syntax
./aiolcdcam --mode 2
```

### Advanced Configuration

Edit `include/config.h` for customization:

```c
// Device settings (MUST BE CONFIGURED!)
#define AIO_UUID "your-device-uid"        // ⚠️ Replace with YOUR device UUID!
#define DAEMON_ADDRESS "http://localhost:11987"

// Display settings
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_REFRESH_INTERVAL_SEC 2

// Temperature thresholds (color gradient)
#define TEMP_THRESHOLD_GREEN 55.0f
#define TEMP_THRESHOLD_ORANGE 65.0f
#define TEMP_THRESHOLD_RED 75.0f
```

## 🔧 Usage & Tips

### Service Management

```bash
# Service control
sudo systemctl start aiolcdcam.service     # Start
sudo systemctl stop aiolcdcam.service      # Stop (displays face.png automatically)
sudo systemctl restart aiolcdcam.service   # Restart
sudo systemctl status aiolcdcam.service    # Status + recent logs

# Live logs
sudo journalctl -u aiolcdcam.service -f

# Makefile shortcuts
make start      # systemctl start aiolcdcam
make stop       # systemctl stop aiolcdcam
make status     # systemctl status aiolcdcam
make logs       # journalctl -u aiolcdcam -f
```

### Manual Usage

```bash
# Run manually (different modes) - both work after installation
aiolcdcam def      # System-wide command (via symlink)
aiolcdcam 1        # Vertical bars
aiolcdcam 2        # Circular diagrams  
aiolcdcam 3        # Horizontal bars

# Or use full path
/opt/aiolcdcam/bin/aiolcdcam def

# Alternative syntax
aiolcdcam --mode def
aiolcdcam --mode 2

# From project directory (before installation)
./aiolcdcam def
```

### Build Commands

```bash
make          # Standard C99 build
make clean    # Clean up
make install  # System installation with dependency auto-detection
make debug    # Debug build with AddressSanitizer
make help     # Show all options
```

**C99 Compliance:**
- Compiled with `-std=c99` flag for strict standards conformance
- Uses `_POSIX_C_SOURCE 200112L` for POSIX compliance
- No GNU extensions or non-standard features

### Performance Notes

- **Mode "def"**: Only temperature sensors, minimal I/O (~3.4MB RAM, <1% CPU)
- **Modes 1-3**: Additional load data with intelligent caching (~3.5MB RAM, <2% CPU)
- **Sensor caching**: hwmon paths cached at startup, GPU data cached for 2 seconds
- **Change detection**: PNG only updated when significant changes occur

### System Requirements

- **OS**: Linux (hwmon support required)
- **CoolerControl**: Version 1.0+ (REQUIRED - must be installed and running)
- **CPU**: x86-64-v3 compatible (Intel Haswell+ 2013+ / AMD Excavator+ 2015+)
- **LCD**: LCD AIO displays supported by CoolerControl (NZXT AIO, etc.)
- **Resources**: < 5 MB RAM, < 1-2% CPU load

**For older CPUs**: Use `CFLAGS=-march=x86-64 make` for compatibility

## 🔍 Troubleshooting

### Common Issues

- **"Connection refused"**: CoolerControl daemon not running → `sudo systemctl start coolercontrold`
- **"Device not found"**: LCD AIO not configured in CoolerControl → Use CoolerControl GUI  
- **"Permission denied"**: Run with appropriate permissions → `sudo aiolcdcam def`
- **"Empty JSON response"**: No devices found → Check CoolerControl configuration and LCD AIO connection
- **"UUID not working"**: Wrong device UUID → Verify with `curl http://localhost:11987/devices | jq` and copy exact UUID

### Debugging Steps

```bash
# 1. Check CoolerControl status
sudo systemctl status coolercontrold
curl http://localhost:11987/devices

# 2. Test LCD AIO CAM manually
aiolcdcam def

# 3. Debug build for detailed information
make debug && aiolcdcam def

# 4. Check service logs
sudo journalctl -u aiolcdcam.service -f
```

### Finding Device UUID

```bash
# If curl command fails, ensure CoolerControl is running
sudo systemctl status coolercontrold
# If not running, start it:
sudo systemctl start coolercontrold

# Then try again
curl http://localhost:11987/devices | jq

# If no devices shown, check CoolerControl GUI configuration
# Your LCD AIO must be detected and configured in CoolerControl first

# Example of expected output:
curl http://localhost:11987/devices | jq
# Should show:
# {
#   "your-device-uuid-here": {
#     "name": "NZXT Kraken",
#     "device_type": "Liquidctl"
#   }
# }
```

## 📄 License

MIT License - See LICENSE file for details.

## 💝 Support the Project

If you find LCD AIO CAM useful and want to support its development:

### 🪙 Cryptocurrency Donations:
- **Bitcoin (BTC)**: `13WjpWQMGG5sg3vTJJnCX3cXzwf2vZddKo`
- **Dogecoin (DOGE)**: `DRSY4cA8eCALn819MjWLbwaePFNti9oS3y`

### 🤝 Other Ways to Support:
- ⭐ **Star this repository** on GitHub
- 🐛 **Report bugs** and suggest improvements  
- 🔄 **Share** the project with others
- 📝 **Contribute** code or documentation

> *All donations help maintain and improve this project. Thank you for your support!*

---

**👨‍💻 Developed by DAMACHINE for maximum efficiency, stability and professional code structure.**  
**📧 Contact:** [christkue79@gmail.com](mailto:christkue79@gmail.com)  
**📖 Manual:** `man aiolcdcam`  
**📍 Binary:** `/opt/aiolcdcam/bin/aiolcdcam` (also available as `aiolcdcam`)  
**💝 Donate:** BTC: `13WjpWQMGG5sg3vTJJnCX3cXzwf2vZddKo` | DOGE: `DRSY4cA8eCALn819MjWLbwaePFNti9oS3y`
