# Makefile für NZXT CAM (Standard C-Projektstruktur)
# NZXT Kraken LCD Temperature Monitor

# Version
VERSION = v0.1.1

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -march=x86-64-v3 -Iinclude $(shell pkg-config --cflags cairo)
LIBS = $(shell pkg-config --libs cairo) -lcurl -lm
TARGET = nzxt

# Verzeichnisse
SRCDIR = src
INCDIR = include
OBJDIR = build

# Quellcode-Dateien
MAIN_SOURCE = $(SRCDIR)/main.c
MODULES = $(SRCDIR)/cpu_monitor.c $(SRCDIR)/gpu_monitor.c $(SRCDIR)/coolant_monitor.c $(SRCDIR)/display.c $(SRCDIR)/coolercontrol.c
HEADERS = $(INCDIR)/config.h $(INCDIR)/cpu_monitor.h $(INCDIR)/gpu_monitor.h $(INCDIR)/coolant_monitor.h $(INCDIR)/display.h $(INCDIR)/coolercontrol.h
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(MODULES))
ALL_SOURCES = $(MAIN_SOURCE) $(MODULES)

SERVICE = systemd/nzxt-cam.service
MANPAGE = docs/nzxt.1
README = docs/README.md
README_DE = docs/README_DE.md
README_ZH = docs/README_ZH.md

# Farben für Terminal-Ausgabe
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
WHITE = \033[1;37m
RESET = \033[0m

# Icons (Unicode)
ICON_BUILD = 🔨
ICON_INSTALL = 📦
ICON_SERVICE = ⚙️
ICON_SUCCESS = ✅
ICON_WARNING = ⚠️
ICON_INFO = ℹ️
ICON_CLEAN = 🧹
ICON_UNINSTALL = 🗑️

# Standard Build Target - Standard C project structure (modern + legacy)
$(TARGET): $(OBJDIR) $(OBJECTS) $(MAIN_SOURCE)
	@printf "$(ICON_BUILD) $(CYAN)Compiling $(TARGET) (Standard C structure)...$(RESET)\n"
	@printf "$(BLUE)Structure:$(RESET) src/ include/ build/\n"
	@printf "$(BLUE)CFLAGS:$(RESET) $(CFLAGS)\n"
	@printf "$(BLUE)LIBS:$(RESET) $(LIBS)\n"
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN_SOURCE) $(OBJECTS) $(LIBS)
	@printf "$(ICON_SUCCESS) $(GREEN)Standard C build successful: $(TARGET)$(RESET)\n"
	@printf "$(ICON_WARNING) $(YELLOW)Building legacy version (legacy/nzxt.c)...$(RESET)\n"
	$(CC) $(CFLAGS) -o nzxt_legacy legacy/nzxt.c $(LIBS)
	@printf "$(ICON_SUCCESS) $(GREEN)Legacy build: nzxt_legacy$(RESET)\n"

# Build-Verzeichnis erstellen
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Objektdateien kompilieren (mit korrekten Pfaden)
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h $(INCDIR)/config.h | $(OBJDIR)
	@printf "$(ICON_BUILD) $(YELLOW)Compiling module: $<$(RESET)\n"
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies für Header-Änderungen
$(OBJECTS): $(HEADERS)

# Clean Target
clean:
	@printf "$(ICON_CLEAN) $(YELLOW)Cleaning up...$(RESET)\n"
	rm -f $(TARGET) nzxt_legacy $(OBJECTS) *.o
	rm -rf $(OBJDIR)
	@printf "$(ICON_SUCCESS) $(GREEN)Cleanup completed$(RESET)\n"

# Detect Linux Distribution (internal function)
detect-distro:
	@if [ -f /etc/arch-release ]; then \
		echo "arch"; \
	elif [ -f /etc/debian_version ]; then \
		echo "debian"; \
	elif [ -f /etc/fedora-release ]; then \
		echo "fedora"; \
	elif [ -f /etc/redhat-release ]; then \
		echo "rhel"; \
	elif [ -f /etc/opensuse-release ]; then \
		echo "opensuse"; \
	else \
		echo "unknown"; \
	fi

# Install Dependencies (internal function)
install-deps:
	@DISTRO=$$($(MAKE) detect-distro); \
	case $$DISTRO in \
		arch) \
			printf "$(ICON_INSTALL) $(GREEN)Installing dependencies for Arch Linux/Manjaro...$(RESET)\n"; \
			sudo pacman -S --needed cairo libcurl-gnutls gcc make pkg-config || { \
				printf "$(ICON_WARNING) $(RED)Error installing dependencies!$(RESET)\n"; \
				printf "$(YELLOW)Please run manually:$(RESET) sudo pacman -S cairo libcurl-gnutls gcc make pkg-config\n"; \
				exit 1; \
			}; \
			;; \
		debian) \
			printf "$(ICON_INSTALL) $(GREEN)Installing dependencies for Ubuntu/Debian...$(RESET)\n"; \
			sudo apt update && sudo apt install -y libcairo2-dev libcurl4-openssl-dev gcc make pkg-config || { \
				printf "$(ICON_WARNING) $(RED)Error installing dependencies!$(RESET)\n"; \
				printf "$(YELLOW)Please run manually:$(RESET) sudo apt install libcairo2-dev libcurl4-openssl-dev gcc make pkg-config\n"; \
				exit 1; \
			}; \
			;; \
		fedora) \
			printf "$(ICON_INSTALL) $(GREEN)Installing dependencies for Fedora...$(RESET)\n"; \
			sudo dnf install -y cairo-devel libcurl-devel gcc make pkg-config || { \
				printf "$(ICON_WARNING) $(RED)Error installing dependencies!$(RESET)\n"; \
				printf "$(YELLOW)Please run manually:$(RESET) sudo dnf install cairo-devel libcurl-devel gcc make pkg-config\n"; \
				exit 1; \
			}; \
			;; \
		rhel) \
			printf "$(ICON_INSTALL) $(GREEN)Installing dependencies for RHEL/CentOS...$(RESET)\n"; \
			sudo yum install -y cairo-devel libcurl-devel gcc make pkg-config || { \
				printf "$(ICON_WARNING) $(RED)Error installing dependencies!$(RESET)\n"; \
				printf "$(YELLOW)Please run manually:$(RESET) sudo yum install cairo-devel libcurl-devel gcc make pkg-config\n"; \
				exit 1; \
			}; \
			;; \
		opensuse) \
			printf "$(ICON_INSTALL) $(GREEN)Installing dependencies for openSUSE...$(RESET)\n"; \
			sudo zypper install -y cairo-devel libcurl-devel gcc make pkg-config || { \
				printf "$(ICON_WARNING) $(RED)Error installing dependencies!$(RESET)\n"; \
				printf "$(YELLOW)Please run manually:$(RESET) sudo zypper install cairo-devel libcurl-devel gcc make pkg-config\n"; \
				exit 1; \
			}; \
			;; \
		*) \
			printf "$(ICON_WARNING) $(RED)Unknown distribution detected!$(RESET)\n"; \
			printf "\n"; \
			printf "$(YELLOW)Please install the following dependencies manually:$(RESET)\n"; \
			printf "\n"; \
			printf "$(WHITE)Arch Linux / Manjaro:$(RESET)\n"; \
			printf "  sudo pacman -S cairo libcurl-gnutls gcc make pkg-config\n"; \
			printf "\n"; \
			printf "$(WHITE)Ubuntu / Debian:$(RESET)\n"; \
			printf "  sudo apt install libcairo2-dev libcurl4-openssl-dev gcc make pkg-config\n"; \
			printf "\n"; \
			printf "$(WHITE)Fedora:$(RESET)\n"; \
			printf "  sudo dnf install cairo-devel libcurl-devel gcc make pkg-config\n"; \
			printf "\n"; \
			printf "$(WHITE)RHEL / CentOS:$(RESET)\n"; \
			printf "  sudo yum install cairo-devel libcurl-devel gcc make pkg-config\n"; \
			printf "\n"; \
			printf "$(WHITE)openSUSE:$(RESET)\n"; \
			printf "  sudo zypper install cairo-devel libcurl-devel gcc make pkg-config\n"; \
			printf "\n"; \
			exit 1; \
			;; \
	esac

# Check Dependencies for Installation (internal function called by install)
check-deps-for-install:
	@MISSING=""; \
	if ! pkg-config --exists cairo >/dev/null 2>&1; then \
		MISSING="$$MISSING cairo"; \
	fi; \
	if ! pkg-config --exists libcurl >/dev/null 2>&1; then \
		MISSING="$$MISSING libcurl"; \
	fi; \
	if ! command -v gcc >/dev/null 2>&1; then \
		MISSING="$$MISSING gcc"; \
	fi; \
	if ! command -v make >/dev/null 2>&1; then \
		MISSING="$$MISSING make"; \
	fi; \
	if [ -n "$$MISSING" ]; then \
		printf "$(ICON_WARNING) $(YELLOW)Missing dependencies detected:$$MISSING$(RESET)\n"; \
		printf "$(ICON_INSTALL) $(CYAN)Auto-installing dependencies...$(RESET)\n"; \
		$(MAKE) install-deps || { \
			printf "$(ICON_WARNING) $(RED)Auto-installation failed!$(RESET)\n"; \
			printf "$(YELLOW)Please install dependencies manually before running 'make install'$(RESET)\n"; \
			exit 1; \
		}; \
	fi

# Install Target - Installs to /opt/nzxt_cam/ (with automatic dependency check and service management)
install: check-deps-for-install $(TARGET)
	@printf "\n"
	@printf "$(ICON_INSTALL) $(WHITE)═══ NZXT CAM INSTALLATION ═══$(RESET)\n"
	@printf "\n"
	@printf "$(ICON_SERVICE) $(CYAN)Checking running service and processes...$(RESET)\n"
	@if sudo systemctl is-active --quiet nzxt-cam.service; then \
		printf "  $(YELLOW)→$(RESET) Service running, stopping for update...\n"; \
		sudo systemctl stop nzxt-cam.service; \
		printf "  $(GREEN)→$(RESET) Service stopped\n"; \
	else \
		printf "  $(BLUE)→$(RESET) Service not running\n"; \
	fi
	@# Check for manual nzxt processes and terminate them
	@NZXT_COUNT=$$(pgrep -x nzxt 2>/dev/null | wc -l); \
	if [ "$$NZXT_COUNT" -gt 0 ]; then \
		printf "  $(YELLOW)→$(RESET) Found $$NZXT_COUNT manual nzxt process(es), terminating...\n"; \
		sudo killall -TERM nzxt 2>/dev/null || true; \
		sleep 2; \
		REMAINING_COUNT=$$(pgrep -x nzxt 2>/dev/null | wc -l); \
		if [ "$$REMAINING_COUNT" -gt 0 ]; then \
			printf "  $(RED)→$(RESET) Force killing $$REMAINING_COUNT remaining process(es)...\n"; \
			sudo killall -KILL nzxt 2>/dev/null || true; \
		fi; \
		printf "  $(GREEN)→$(RESET) Manual processes terminated\n"; \
	else \
		printf "  $(BLUE)→$(RESET) No manual nzxt processes found\n"; \
	fi
	@printf "\n"
	@printf "$(ICON_INFO) $(CYAN)Creating directories...$(RESET)\n"
	sudo mkdir -p /opt/nzxt_cam
	sudo mkdir -p /opt/nzxt_cam/docs
	sudo mkdir -p /opt/nzxt_cam/image
	sudo mkdir -p /opt/nzxt_cam/legacy
	@printf "$(ICON_SUCCESS) $(GREEN)Directories created$(RESET)\n"
	@printf "\n"
	@printf "$(ICON_INFO) $(CYAN)Copying files...$(RESET)\n"
	sudo cp $(TARGET) /opt/nzxt_cam/
	sudo cp nzxt_legacy /opt/nzxt_cam/legacy/
	sudo chmod +x /opt/nzxt_cam/$(TARGET)
	sudo chmod +x /opt/nzxt_cam/legacy/nzxt_legacy
	sudo cp image/face.png /opt/nzxt_cam/image/ 2>/dev/null || true
	sudo cp $(README) /opt/nzxt_cam/docs/
	sudo cp $(README_DE) /opt/nzxt_cam/docs/
	sudo cp $(README_ZH) /opt/nzxt_cam/docs/
	@printf "  $(GREEN)→$(RESET) Program (modern): /opt/nzxt_cam/$(TARGET)\n"
	@printf "  $(GREEN)→$(RESET) Program (legacy): /opt/nzxt_cam/legacy/nzxt_legacy\n"
	@printf "  $(GREEN)→$(RESET) Shutdown image: /opt/nzxt_cam/image/face.png\n"
	@printf "  $(GREEN)→$(RESET) README (EN): /opt/nzxt_cam/docs/README.md\n"
	@printf "  $(GREEN)→$(RESET) README (DE): /opt/nzxt_cam/docs/README_DE.md\n"
	@printf "  $(GREEN)→$(RESET) README (ZH): /opt/nzxt_cam/docs/README_ZH.md\n"
	@printf "\n"
	@printf "$(ICON_SERVICE) $(CYAN)Installing service & documentation...$(RESET)\n"
	sudo cp $(SERVICE) /etc/systemd/system/
	sudo cp $(MANPAGE) /usr/share/man/man1/
	sudo mandb -q
	sudo systemctl daemon-reload
	@printf "  $(GREEN)→$(RESET) Service: /etc/systemd/system/nzxt-cam.service\n"
	@printf "  $(GREEN)→$(RESET) Manual: /usr/share/man/man1/nzxt.1\n"
	@printf "\n"
	@printf "$(ICON_SERVICE) $(CYAN)Restarting service...$(RESET)\n"
	@if sudo systemctl is-enabled --quiet nzxt-cam.service; then \
		sudo systemctl start nzxt-cam.service; \
		printf "  $(GREEN)→$(RESET) Service started\n"; \
		printf "  $(GREEN)→$(RESET) Status: $$(sudo systemctl is-active nzxt-cam.service)\n"; \
	else \
		printf "  $(YELLOW)→$(RESET) Service not enabled\n"; \
		printf "  $(YELLOW)→$(RESET) Enable with: sudo systemctl enable nzxt-cam.service\n"; \
	fi
	@printf "\n"
	@printf "$(ICON_SUCCESS) $(WHITE)═══ INSTALLATION SUCCESSFUL ═══$(RESET)\n"
	@printf "\n"
	@printf "$(YELLOW)📋 Next steps:$(RESET)\n"
	@if sudo systemctl is-enabled --quiet nzxt-cam.service; then \
		printf "  $(GREEN)✓$(RESET) Service enabled and started\n"; \
		printf "  $(PURPLE)Check status:$(RESET)        sudo systemctl status nzxt-cam.service\n"; \
	else \
		printf "  $(PURPLE)Enable service:$(RESET)      sudo systemctl enable nzxt-cam.service\n"; \
		printf "  $(PURPLE)Start service:$(RESET)       sudo systemctl start nzxt-cam.service\n"; \
	fi
	@printf "  $(PURPLE)Show manual:$(RESET)         man nzxt\n"
	@printf "\n"
	@printf "$(YELLOW)🔄 Available versions:$(RESET)\n"
	@printf "  $(GREEN)Modern (default):$(RESET) /opt/nzxt_cam/nzxt [mode]\n"
	@printf "  $(GREEN)Legacy:$(RESET)  /opt/nzxt_cam/legacy/nzxt_legacy [mode]\n"
	@printf "  $(BLUE)Note:$(RESET) Service always uses modern version\n"
	@printf "\n"

# Uninstall Target
uninstall:
	@printf "\n"
	@printf "$(ICON_UNINSTALL) $(WHITE)═══ NZXT CAM UNINSTALLATION ═══$(RESET)\n"
	@printf "\n"
	@printf "$(ICON_WARNING) $(YELLOW)Stopping and disabling service...$(RESET)\n"
	sudo systemctl stop nzxt-cam.service || true
	sudo systemctl disable nzxt-cam.service || true
	@printf "$(ICON_SUCCESS) $(GREEN)Service stopped$(RESET)\n"
	@printf "\n"
	@printf "$(ICON_INFO) $(CYAN)Removing files...$(RESET)\n"
	sudo rm -f /etc/systemd/system/nzxt-cam.service
	sudo rm -f /usr/share/man/man1/nzxt.1
	sudo rm -rf /opt/nzxt_cam/docs/
	sudo rm -rf /opt/nzxt_cam/scripts/
	sudo rm -f /opt/nzxt_cam/$(TARGET)
	sudo rm -rf /opt/nzxt_cam/legacy/
	@printf "  $(RED)✗$(RESET) Service: /etc/systemd/system/nzxt-cam.service\n"
	@printf "  $(RED)✗$(RESET) Manual: /usr/share/man/man1/nzxt.1\n"
	@printf "  $(RED)✗$(RESET) Program: /opt/nzxt_cam/$(TARGET)\n"
	@printf "  $(RED)✗$(RESET) Documentation: /opt/nzxt_cam/docs/\n"
	@printf "  $(RED)✗$(RESET) Legacy: /opt/nzxt_cam/legacy/\n"
	@printf "\n"
	@printf "$(ICON_INFO) $(CYAN)Updating system...$(RESET)\n"
	sudo mandb -q
	sudo systemctl daemon-reload
	@printf "\n"
	@printf "$(ICON_SUCCESS) $(WHITE)═══ UNINSTALLATION COMPLETE ═══$(RESET)\n"
	@printf "\n"
	@printf "$(ICON_INFO) $(BLUE)Note:$(RESET) /opt/nzxt_cam/image/ remains (may contain images)\n"
	@printf "\n"

# Debug Build
debug: CFLAGS += -g -DDEBUG -fsanitize=address
debug: LIBS += -fsanitize=address
debug: $(TARGET)
	@printf "$(ICON_SUCCESS) $(GREEN)Debug build created with AddressSanitizer$(RESET)\n"

# Service Management Targets
start:
	@printf "$(ICON_SERVICE) $(GREEN)Starting nzxt-cam service...$(RESET)\n"
	sudo systemctl start nzxt-cam.service
	@printf "$(ICON_SUCCESS) $(GREEN)Service started$(RESET)\n"

stop:
	@printf "$(ICON_SERVICE) $(YELLOW)Stopping nzxt-cam service...$(RESET)\n"
	sudo systemctl stop nzxt-cam.service
	@printf "$(ICON_SUCCESS) $(GREEN)Service stopped$(RESET)\n"

restart:
	@printf "$(ICON_SERVICE) $(CYAN)Restarting nzxt-cam service...$(RESET)\n"
	sudo systemctl restart nzxt-cam.service
	@printf "$(ICON_SUCCESS) $(GREEN)Service restarted$(RESET)\n"

status:
	@printf "$(ICON_INFO) $(CYAN)Service Status:$(RESET)\n"
	sudo systemctl status nzxt-cam.service

enable:
	@printf "$(ICON_SERVICE) $(GREEN)Enabling autostart...$(RESET)\n"
	sudo systemctl enable nzxt-cam.service
	@printf "$(ICON_SUCCESS) $(GREEN)Service will start automatically at boot$(RESET)\n"

disable:
	@printf "$(ICON_SERVICE) $(YELLOW)Disabling autostart...$(RESET)\n"
	sudo systemctl disable nzxt-cam.service
	@printf "$(ICON_SUCCESS) $(GREEN)Boot autostart disabled$(RESET)\n"

logs:
	@printf "$(ICON_INFO) $(CYAN)Live logs (Ctrl+C to exit):$(RESET)\n"
	sudo journalctl -u nzxt-cam.service -f

# Help
help:
	@printf "\n"
	@printf "$(WHITE)════════════════════════════════════════$(RESET)\n"
	@printf "$(WHITE)         NZXT CAM BUILD SYSTEM          $(RESET)\n"
	@printf "$(WHITE)════════════════════════════════════════$(RESET)\n"
	@printf "\n"
	@printf "$(YELLOW)🔨 Build Targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)          - Compiles the program (modern + legacy)\n"
	@printf "  $(GREEN)make clean$(RESET)    - Removes compiled files\n"
	@printf "  $(GREEN)make debug$(RESET)    - Debug build with AddressSanitizer\n"
	@printf "\n"
	@printf "$(YELLOW)📦 Installation:$(RESET)\n"
	@printf "  $(GREEN)make install$(RESET)  - Installs to /opt/nzxt_cam/ (auto-installs dependencies)\n"
	@printf "  $(GREEN)make uninstall$(RESET)- Uninstalls the program\n"
	@printf "\n"
	@printf "$(YELLOW)⚙️  Service Management:$(RESET)\n"
	@printf "  $(GREEN)sudo systemctl start nzxt-cam.service$(RESET)    - Starts the service\n"
	@printf "  $(GREEN)sudo systemctl stop nzxt-cam.service$(RESET)     - Stops the service (sends face.png to LCD automatically)\n"
	@printf "  $(GREEN)sudo systemctl restart nzxt-cam.service$(RESET)  - Restarts the service\n"
	@printf "  $(GREEN)sudo systemctl status nzxt-cam.service$(RESET)   - Shows service status\n"
	@printf "  $(GREEN)sudo systemctl enable nzxt-cam.service$(RESET)   - Enables autostart\n"
	@printf "  $(GREEN)sudo systemctl disable nzxt-cam.service$(RESET)  - Disables autostart\n"
	@printf "  $(GREEN)sudo journalctl -u nzxt-cam.service -f$(RESET)   - Shows live logs\n"
	@printf "  $(BLUE)Note:$(RESET) Shortcuts available: make start/stop/restart/status/enable/disable/logs\n"
	@printf "  $(BLUE)Shutdown:$(RESET) Service automatically displays face.png when stopped (integrated in C code)\n"
	@printf "\n"
	@printf "$(YELLOW)📚 Documentation:$(RESET)\n"
	@printf "  $(GREEN)man nzxt$(RESET)      - Shows manual page\n"
	@printf "  $(GREEN)make help$(RESET)     - Shows this help\n"
	@printf "\n"
	@printf "$(YELLOW)🌍 README Languages:$(RESET)\n"
	@printf "  $(GREEN)docs/README.md$(RESET)    - 🇺🇸 English (main documentation)\n"
	@printf "  $(GREEN)docs/README_DE.md$(RESET) - 🇩🇪 Deutsch\n"
	@printf "  $(GREEN)docs/README_ZH.md$(RESET) - 🇨🇳 中文 (Chinese)\n"
	@printf "\n"
	@printf "$(YELLOW)🔄 Version Usage:$(RESET)\n"
	@printf "  $(GREEN)Modern (default):$(RESET) /opt/nzxt_cam/nzxt [mode]\n"
	@printf "  $(GREEN)Legacy:$(RESET)  /opt/nzxt_cam/legacy/nzxt_legacy [mode]\n"
	@printf "  $(BLUE)Note:$(RESET) Service always uses modern version\n"
	@printf "\n"
	@printf "$(BLUE)Compiler Flags:$(RESET) $(CFLAGS)\n"
	@printf "$(BLUE)Libraries:$(RESET) $(LIBS)\n"
	@printf "\n"

.PHONY: clean install uninstall debug start stop restart status enable disable logs help detect-distro install-deps check-deps-for-install