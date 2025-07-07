# NZXT CAM - Professional Modular C Daemon

High-performance, modular C-based daemon for NZXT Kraken LCD Temperature Monitor with professional systemd integration.

**📖 Languages / Sprachen / 语言:**
- **🇺🇸 English**: README.md (this file)
- **🇩🇪 Deutsch**: [README_DE.md](README_DE.md)
- **🇨🇳 中文**: [README_ZH.md](README_ZH.md)

## ✨ Features

- **✅ Modular Architecture**: Professional separation of CPU, GPU, coolant, and display logic into separate modules
- **✅ Efficient Sensor Polling**: Only necessary sensor data is queried depending on mode (def = temperatures only, 1-3 = additional load data)
- **✅ Central Configuration**: All settings (UUID, paths, colors, layout) in `include/config.h`
- **✅ 4 Optimized Display Modes**: From simple temperatures to complex load diagrams
- **✅ Performance-Optimized**: Caching, change detection, minimal I/O operations, mode-dependent resource usage
- **✅ Native CoolerControl Integration**: REST API communication without Python dependencies
- **✅ Systemd Logs**: Detailed initialization and status messages for professional service management
- **✅ Intelligent Installation**: Automatic service stop/start during updates via `make install`

## 🏗️ Standard C Project Structure

```
nzxt_cam/
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
├── legacy/                 # 📁 Legacy version
│   └── nzxt.c              # 🔄 Original single-file implementation
├── docs/                   # 📁 Documentation
│   ├── README.md           # 📖 English documentation (main)
│   ├── README_DE.md        # 📖 German documentation
│   ├── README_ZH.md        # 📖 Chinese documentation
│   └── nzxt.1              # 📖 Manual page
├── systemd/                # 📁 systemd service integration
│   └── nzxt-cam.service    # 🔧 Service file
└── Makefile                # 🔨 Build system with auto-dependency installation
```

## 🚀 Installation & Build

### One-Command Installation

**🆕 NEW**: `make install` now automatically installs all dependencies for any Linux distribution!

```bash
# Complete installation in one command (auto-detects Linux distribution)
sudo make install

# This automatically:
# 1. Detects your Linux distribution (Arch, Ubuntu, Debian, Fedora, RHEL, openSUSE)
# 2. Checks for missing dependencies (cairo, libcurl, gcc, make, pkg-config)
# 3. Auto-installs missing dependencies using your distribution's package manager
# 4. Builds the program (modern + legacy versions)
# 5. Installs to /opt/nzxt_cam/
# 6. Configures systemd service
# 7. Starts/restarts the service
```

### Dependencies (Auto-Installed)

**Supported distributions** (auto-detected and installed by `make install`):
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

# Check dependencies before building
make check-deps
```

**Note**: The `make` command now automatically builds both the modern modular version (`nzxt`) and the legacy single-file version (`nzxt_legacy`) from `legacy/nzxt.c`.

**Build output with performance optimization:**
```
🔨 Compiling module: src/cpu_monitor.c
🔨 Compiling module: src/gpu_monitor.c
🔨 Compiling nzxt (Standard C structure)...
CFLAGS: -Wall -Wextra -O2 -std=c99 -march=x86-64-v3 -Iinclude
LIBS: -lcairo -lcurl -lm
✅ Standard C build successful: nzxt
⚠️ Building legacy version (legacy/nzxt.c)...
✅ Legacy build: nzxt_legacy
```

**Performance benefits of x86-64-v3:**
- Extended CPU instructions (BMI1, BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE)
- Optimized floating-point operations for Cairo graphics
- Better vectorization for sensor data processing
- ~10-15% performance improvement in graphical rendering

### Installation

```bash
# Complete system installation (auto-installs dependencies + configures service)
sudo make install

# Output example:
#   📦 NZXT CAM INSTALLATION
#   ⚠️ Missing dependencies detected: cairo libcurl
#   � Installing dependencies for Arch Linux/Manjaro...
#   ✅ Dependencies installed successfully!
#   ⚙️ Checking running service and processes...
#   → Service stopped
#   ℹ️ Creating directories...
#   ℹ️ Copying files...
#   ⚙️ Installing service & documentation...
#   ⚙️ Restarting service...
#   ✅ INSTALLATION SUCCESSFUL

# Enable autostart (optional)
sudo systemctl enable nzxt-cam

# Check status
systemctl status nzxt-cam
```

**Systemd service logs show:**
```
Jul 06 03:06:43 computer nzxt-cam[72998]: Selected mode: def (temperatures only, resource-efficient)
Jul 06 03:06:43 computer nzxt-cam[72998]: Initializing modules...
Jul 06 03:06:43 computer nzxt-cam[72998]: ✓ CPU monitor initialized
Jul 06 03:06:43 computer nzxt-cam[72998]: ✓ GPU monitor initialized
Jul 06 03:06:43 computer nzxt-cam[72998]: ✓ Coolant monitor initialized
Jul 06 03:06:43 computer nzxt-cam[72998]: ✓ CoolerControl session initialized
Jul 06 03:06:43 computer nzxt-cam[72998]: CoolerControl: Connected to Kraken LCD
Jul 06 03:06:43 computer nzxt-cam[72998]: All modules successfully initialized!
Jul 06 03:06:43 computer nzxt-cam[72998]: NZXT CAM daemon started (Mode: 0)
Jul 06 03:06:43 computer nzxt-cam[72998]: Sensor data updated every 2.5 seconds
Jul 06 03:06:43 computer nzxt-cam[72998]: Daemon now running silently in background...
```

## 📱 Modes

| Mode | Description | I/O Optimization | Sensor Data |
|------|-------------|------------------|-------------|
| `def` | Temperatures only (CPU, GPU, coolant) | ✅ **Minimal** - No load data | CPU temp, GPU temp, coolant temp |
| `1`   | Temperatures + vertical load bars | CPU/GPU/RAM load | All sensors + CPU/RAM/GPU load |
| `2`   | Temperatures + circular diagrams | CPU/GPU/RAM load | All sensors + CPU/RAM/GPU load |
| `3`   | Temperatures + horizontal load bars | CPU/GPU/RAM load | All sensors + CPU/RAM/GPU load |

### Usage

```bash
# Modern version (modular, recommended)
./nzxt def      # Temperatures only (default, minimal I/O)
./nzxt 1        # Vertical bars
./nzxt 2        # Circular diagrams  
./nzxt 3        # Horizontal bars

# Legacy version (single-file implementation)
./nzxt_legacy def
./nzxt_legacy 1

# Alternative --mode syntax (modern version)
./nzxt --mode def
./nzxt --mode 2

# As systemd service (uses modern version)
sudo systemctl start nzxt-cam
sudo systemctl status nzxt-cam  # Shows detailed initialization logs

# Show live logs
sudo journalctl -u nzxt-cam.service -f

# Installed versions (after make install)
/opt/nzxt_cam/nzxt def                    # Modern version
/opt/nzxt_cam/legacy/nzxt_legacy def      # Legacy version
```

**Resource efficiency by mode:**
- **Mode "def"**: ~3.4MB RAM, minimal CPU load, temperature sensors only
- **Modes 1-3**: ~3.5MB RAM, additional CPU/GPU load queries

## ⚙️ Configuration

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

## 🎯 Performance Optimizations

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

### ✅ Modular architecture benefits

- **Separate compilation**: Each module is compiled individually → faster development
- **Clear responsibilities**: CPU, GPU, coolant, display logic isolated
- **Central configuration**: All constants in `include/config.h`
- **Header dependencies**: Clean include structure without circular dependencies

## 🛠️ Development

### Adding modules

1. Create header: `include/new_module.h`
2. Implementation: `src/new_module.c`
3. Include in `src/main.c`: `#include "new_module.h"`
4. Extend Makefile `MODULES`: `$(SRCDIR)/new_module.c`

**Example for new module:**
```bash
# Create header file
echo '#ifndef NEW_MODULE_H\n#define NEW_MODULE_H\nvoid new_function(void);\n#endif' > include/new_module.h

# Create source file
echo '#include "new_module.h"\nvoid new_function(void) { /* Implementation */ }' > src/new_module.c

# Adjust Makefile
# MODULES = $(SRCDIR)/cpu_monitor.c ... $(SRCDIR)/new_module.c
```

### Build targets

```bash
make          # Standard C build (modern + legacy versions)
make clean    # Clean up (deletes build/ directory)
make install  # System installation
make start    # Start service
make help     # Show all options
```

**Directory structure after build:**
```
nzxt_cam/
├── src/           # Source code (.c)
├── include/       # Headers (.h)  
├── build/         # Object files (.o)
├── nzxt           # Executable file
└── ...
```

## 📊 Professional Systemd Integration

```bash
# Service management with detailed logs
sudo systemctl enable nzxt-cam    # Auto-start at boot
sudo systemctl start nzxt-cam     # Start immediately
sudo systemctl stop nzxt-cam      # Stop
sudo systemctl restart nzxt-cam   # Restart
sudo systemctl status nzxt-cam    # Status + recent logs

# Detailed live logs (shows initialization and operation)
journalctl -u nzxt-cam -f

# Logs from last 10 minutes
journalctl -u nzxt-cam --since "10 minutes ago"
```

**What the logs show:**
- ✅ Selected mode and optimization strategy
- ✅ Module-by-module initialization
- ✅ CoolerControl session status
- ✅ Refresh interval and operating mode
- ✅ Error diagnosis for problems

**Makefile service integration:**
```bash
# All service commands available as Make targets
make start      # systemctl start nzxt-cam
make stop       # systemctl stop nzxt-cam  
make restart    # systemctl restart nzxt-cam
make status     # systemctl status nzxt-cam
make logs       # journalctl -u nzxt-cam -f
```

## 🔍 Debugging

```bash
# Start manually (foreground)
./nzxt def

# With debug information
make debug && ./nzxt def

# Session check
systemctl status coolercontrold
curl http://localhost:11987/devices
```

## 🎨 Customization

### Adjusting colors

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

### Adjusting layout

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

## 📋 System Requirements

- **OS**: Linux (hwmon support required)
- **CPU**: x86-64-v3 compatible (Intel Haswell+ / AMD Excavator+, 2013+)
- **Sensors**: hwmon temperature sensors for CPU
- **GPU**: NVIDIA (optional, for GPU data)
- **LCD**: NZXT Kraken with CoolerControl support
- **RAM**: < 5 MB (very efficient)
- **CPU Load**: < 1% (def mode), < 2% (modes 1-3)
- **Dependencies**: cairo, libcurl, systemd (for service integration)

**x86-64-v3 compatibility:**
- **Intel**: Haswell (2013) and newer
- **AMD**: Excavator (2015) and newer  
- Older CPUs: Use `CFLAGS=-march=x86-64` for compatibility

## 📄 License

MIT License - See LICENSE file for details.

---

**Developed for maximum efficiency, stability and professional code structure.**
