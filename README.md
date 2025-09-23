---
---
# ⚠️ STOP WORKING HERE ⚠️ 

# Please visit: [https://github.com/damachine/coolerdash](https://github.com/damachine/coolerdash) 
---
---

# LCD AIO CAM

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)](https://kernel.org/)
[![Development Status](https://img.shields.io/badge/Status-Beta-orange.svg)](https://github.com)

> **⚠️ Beta Notice:** This project is in early development stage. Features may change and bugs are expected. Please report issues and contribute to improve the software.

**👨‍💻 Author:** DAMACHINE ([christkue79@gmail.com](mailto:christkue79@gmail.com))

## ✨ Features

- **✅ Modular Architecture**: Professional separation of CPU, GPU, coolant, and display logic into separate modules
- **✅ Efficient Sensor Polling**: Only necessary sensor data is queried depending on mode (def = temperatures only, 1-3 = additional load data)
- **✅ Central Configuration**: All settings (UUID, paths, colors, layout) in `include/config.h`
- **✅ 4 Optimized Display Modes**: From simple temperatures to complex load diagrams
- **✅ Performance-Optimized**: Caching, change detection, minimal I/O operations, mode-dependent resource usage
- **✅ Native CoolerControl Integration**: REST API communication without Python dependencies
- **✅ Systemd Logs**: Detailed initialization and status messages for professional service management
- **✅ Intelligent Installation**: Automatic service stop/start during updates via `make install`
- **🚧 Beta Features**: Active development with regular improvements and bug fixes

## 🚀 Quick Start

### Prerequisites

1. **Install CoolerControl first**: [Installation Guide](https://gitlab.com/coolercontrol/coolercontrol/-/blob/main/README.md)
2. **Start CoolerControl daemon**: `sudo systemctl start coolercontrold`
3. **Configure your LCD AIO** in CoolerControl GUI

### Install LCD AIO CAM

```bash
# One-command installation (auto-detects Linux distribution and installs dependencies)
git clone https://github.com/damachine/aiolcdcam.git
cd aiolcdcam
sudo make install

# Enable autostart and start service
sudo systemctl enable aiolcdcam.service
sudo systemctl start aiolcdcam.service

# Check status
systemctl status aiolcdcam
```

> **ℹ️ Note:** The compiled binary is named `aiolcdcam`, and the systemd service is now also `aiolcdcam.service` for consistency.

### Verify Setup

```bash
# Check CoolerControl API
curl http://localhost:11987/devices

# Check LCD AIO CAM logs
sudo journalctl -u aiolcdcam.service -f
```

## 📋 System Requirements

### Essential Requirements

- **OS**: Linux (hwmon support required)
- **🌐 CoolerControl**: **REQUIRED** - [CoolerControl](https://gitlab.com/coolercontrol/coolercontrol) daemon must be installed and running
  - Provides REST API for LCD AIO communication
  - Version 1.0+ recommended
  - Must be configured with your LCD AIO device
- **CPU**: x86-64-v3 compatible (Intel Haswell+ / AMD Excavator+, 2013+)
- **LCD**: LCD AIO displays supported by CoolerControl (NZXT Kraken, etc.)

### Hardware & Performance

- **Sensors**: hwmon temperature sensors for CPU
- **GPU**: NVIDIA (optional, for GPU data via nvidia-smi)
- **RAM**: < 5 MB (very efficient)
- **CPU Load**: < 1% (def mode), < 2% (modes 1-3)

### Software Dependencies

- **cairo**: Graphics rendering library
- **libcurl**: HTTP client for CoolerControl API
- **systemd**: Service management (for daemon integration)
- **pkg-config**: Build dependency detection
- **gcc**: C compiler

### CoolerControl Setup

1. **Install CoolerControl**: Follow [installation guide](https://gitlab.com/coolercontrol/coolercontrol/-/blob/main/README.md)
2. **Start CoolerControl daemon**: `sudo systemctl start coolercontrold`
3. **Configure your LCD AIO**: Use CoolerControl GUI to detect and configure your device
4. **Verify API access**: `curl http://localhost:11987/devices` should return your devices

## 📦 Installation & Dependencies

```bash
# STEP 1:
sudo make install
```

### Supported Distributions (Auto-Detected)

- **Arch Linux / Manjaro**: `pacman -S cairo libcurl-gnutls gcc make pkg-config`
- **Ubuntu / Debian**: `apt install libcairo2-dev libcurl4-openssl-dev gcc make pkg-config`
- **Fedora**: `dnf install cairo-devel libcurl-devel gcc make pkg-config`
- **RHEL / CentOS**: `yum install cairo-devel libcurl-devel gcc make pkg-config`
- **openSUSE**: `zypper install cairo-devel libcurl-devel gcc make pkg-config`

### Manual Build (Optional)

```bash
# Build only (without installation)
make

# Clean rebuild
make clean && make

# Debug build
make debug
```

## ⚙️ Service Management

```bash
# Enable autostart at boot
sudo systemctl enable aiolcdcam.service

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

## � Display Modes & Usage

| Mode | Description | I/O Optimization | Sensor Data |
|------|-------------|------------------|-------------|
| `def` | Temperatures only (CPU, GPU, coolant) | ✅ **Minimal** - No load data | CPU temp, GPU temp, coolant temp |
| `1`   | Temperatures + vertical load bars | CPU/GPU/RAM load | All sensors + CPU/RAM/GPU load |
| `2`   | Temperatures + circular diagrams | CPU/GPU/RAM load | All sensors + CPU/RAM/GPU load |
| `3`   | Temperatures + horizontal load bars | CPU/GPU/RAM load | All sensors + CPU/RAM/GPU load |

### Usage Examples

```bash
# Modern version (modular, recommended) - Binary: aiolcdcam
./aiolcdcam def      # Temperatures only (default, minimal I/O)
./aiolcdcam 1        # Vertical bars
./aiolcdcam 2        # Circular diagrams  
./aiolcdcam 3        # Horizontal bars

# Alternative builds
make debug    # Debug version with AddressSanitizer

# Alternative --mode syntax (modern version)
./aiolcdcam --mode def
./aiolcdcam --mode 2

# As systemd service (uses modern aiolcdcam binary)
sudo systemctl start aiolcdcam.service
sudo systemctl status aiolcdcam.service  # Shows detailed initialization logs

# Installed versions (after make install)
/opt/aiolcdcam/bin/aiolcdcam def           # Standard version
```

**Resource efficiency by mode:**
- **Mode "def"**: ~3.4MB RAM, minimal CPU load, temperature sensors only
- **Modes 1-3**: ~3.5MB RAM, additional CPU/GPU load queries

## 🔧 Configuration

All important settings are located in **`include/config.h`**:

```c
// Device settings
#define KRAKEN_UID "your-device-uid"
#define DAEMON_ADDRESS "http://localhost:11987"
#define DAEMON_PASSWORD "coolAdmin"

// Display settings
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_REFRESH_INTERVAL_SEC 2

// Temperature thresholds (color gradient)
#define TEMP_THRESHOLD_GREEN 55.0f
#define TEMP_THRESHOLD_ORANGE 65.0f
#define TEMP_THRESHOLD_RED 75.0f

// Caching intervals
#define GPU_CACHE_INTERVAL 2
#define CHANGE_TOLERANCE_TEMP 0.1f
#define CHANGE_TOLERANCE_USAGE 0.5f
```

### Adding New Modules

1. Create header: `include/new_module.h`
2. Implementation: `src/new_module.c`
3. Include in `src/main.c`: `#include "new_module.h"`
4. Extend Makefile `MODULES`: `$(SRCDIR)/new_module.c`

**Example:**
```bash
# Create header file
echo '#ifndef NEW_MODULE_H\n#define NEW_MODULE_H\nvoid new_function(void);\n#endif' > include/new_module.h

# Create source file
echo '#include "new_module.h"\nvoid new_function(void) { /* Implementation */ }' > src/new_module.c
```

## 🏗️ Project Structure

```
aiolcdcam/
├── src/                    # 📁 Source code files (.c)
│   ├── main.c              # 🎯 Main program (daemon management, sensor coordination)
│   ├── cpu_monitor.c       # 🔥 CPU temperature, CPU load, RAM monitoring
│   ├── gpu_monitor.c       # 🎮 GPU temperature, GPU load (NVIDIA)
│   ├── coolant_monitor.c   # 💧 Coolant temperature monitoring
│   ├── display.c           # 🖼️ Rendering engine, mode logic, Cairo graphics
│   └── coolercontrol.c     # 🌐 REST API communication, session management
├── include/                # 📁 Header files (.h)
│   ├── config.h            # ⚙️ Central configuration (UUID, paths, colors, layout)
│   ├── cpu_monitor.h       # 🔥 CPU monitor interface
│   ├── gpu_monitor.h       # 🎮 GPU monitor interface
│   ├── coolant_monitor.h   # 💧 Coolant monitor interface
│   ├── display.h           # 🖼️ Display engine interface
│   └── coolercontrol.h     # 🌐 CoolerControl API interface
├── build/                  # 📁 Compiled object files (.o)
├── man/                    # 📁 Documentation
│   └── aiolcdcam.1         # � Manual page
├── docs/                   # 📁 Documentation
│   ├── README.md           # 📖 English documentation (main)
│   ├── README_DE.md        # 📖 German documentation
│   ├── README_ZH.md        # 📖 Chinese documentation
│   └── aiolcdcam.1         # 📖 Manual page
├── systemd/                # 📁 systemd service integration
│   └── aiolcdcam.service   # 🔧 Service file
└── Makefile                # 🔨 Build system with auto-dependency installation
```

### ✅ Mode-dependent I/O optimization

**Implemented in `src/display.c:draw_combined_image()`:**

- **Mode "def"**: 
  ```c
  // Temperatures only (minimal I/O)
  sensor_data.cpu_temp = read_cpu_temp();
  sensor_data.gpu_temp = read_gpu_temp(); 
  sensor_data.coolant_temp = read_coolant_temp();
  // No load data → 0% CPU time for /proc/stat, nvidia-smi
  ```

- **Modes 1-3**: 
  ```c
  // Additional load data
  if (mode != DISPLAY_MODE_DEF) {
      sensor_data.cpu_usage = calculate_cpu_usage(...);
      sensor_data.ram_usage = get_ram_usage();
      get_gpu_usage_data(&gpu_usage, &gpu_mem_usage);
  }
  ```

### ✅ Sensor data caching & path optimization

- **CPU temperature**: hwmon paths are cached once at startup (`init_cpu_sensor_path()`)
- **GPU data**: 2-second cache with nvidia-smi (`GPU_CACHE_INTERVAL`)
- **Coolant temperature**: hwmon cache with one-time path detection
- **CPU load**: Stateful between measurements (no duplicate /proc/stat calls)

### ✅ Change detection & display updates

```c
// Tolerances for change detection (config.h)
#define CHANGE_TOLERANCE_TEMP 0.1f    // 0.1°C temperature difference
#define CHANGE_TOLERANCE_USAGE 0.5f   // 0.5% load difference

// PNG is only written and transmitted when significant changes occur
const int needs_update = (
    fabsf(cpu_temp - last_cpu_temp) > CHANGE_TOLERANCE_TEMP ||
    fabsf(gpu_usage - last_gpu_usage) > CHANGE_TOLERANCE_USAGE ||
    ...
);
```

## 🎨 Customization

### Adjusting Colors

In `include/config.h`:

```c
// Temperature color gradient
#define COLOR_GREEN_R 0
#define COLOR_GREEN_G 255  
#define COLOR_GREEN_B 0

// Load bar colors
#define COLOR_CPU_USAGE_R 0.3
#define COLOR_CPU_USAGE_G 0.7
#define COLOR_CPU_USAGE_B 1.0
```

### Adjusting Layout

```c
// Display size
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

// Bar dimensions
#define BAR_WIDTH 220
#define BAR_HEIGHT 30

// Font sizes
#define FONT_SIZE_LARGE 90.0
#define FONT_SIZE_LABELS 22.0
```

## 🔍 Debugging

```bash
# Check CoolerControl status first
systemctl status coolercontrold
curl http://localhost:11987/devices

# Start LCD AIO CAM manually (foreground)
./aiolcdcam def

# With debug information
make debug && ./aiolcdcam def

# Check CoolerControl API response
curl http://localhost:11987/devices | jq
```

### Common Issues

- **"Connection refused"**: CoolerControl daemon not running → `sudo systemctl start coolercontrold`
- **"Device not found"**: LCD AIO not configured in CoolerControl → Use CoolerControl GUI
- **"Permission denied"**: Run with appropriate permissions → `sudo ./aiolcdcam def`

## 📄 License

MIT License - See LICENSE file for details.

---

**👨‍💻 Developed by DAMACHINE for maximum efficiency, stability and professional code structure.**  
**📧 Contact:** [christkue79@gmail.com](mailto:christkue79@gmail.com)
