# Makefile für nzxt.c
# NZXT Kraken LCD Temperature Monitor

CC = gcc
CFLAGS = -Wall -O2 $(shell pkg-config --cflags cairo)
LIBS = $(shell pkg-config --libs cairo) -lcurl -lm
TARGET = nzxt
SOURCE = nzxt.c
SERVICE = systemd/nzxt-cam.service
MANPAGE = docs/nzxt.1
README = README.md

# Standard Build Target
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

# Clean Target
clean:
	rm -f $(TARGET)

# Install Target - Installiert nach /opt/nzxt_cam/
install: $(TARGET)
	sudo mkdir -p /opt/nzxt_cam
	sudo mkdir -p /opt/nzxt_cam/image
	sudo cp $(TARGET) /opt/nzxt_cam/
	sudo chmod +x /opt/nzxt_cam/$(TARGET)
	sudo cp $(README) /opt/nzxt_cam/
	sudo cp $(SERVICE) /etc/systemd/system/
	sudo cp $(MANPAGE) /usr/share/man/man1/
	sudo mandb -q
	sudo systemctl daemon-reload
	@echo "Installation abgeschlossen:"
	@echo "  Programm: /opt/nzxt_cam/$(TARGET)"
	@echo "  README: /opt/nzxt_cam/$(README)"
	@echo "  Bildordner: /opt/nzxt_cam/image"
	@echo "  Service: /etc/systemd/system/$(SERVICE)"
	@echo "  Manual: /usr/share/man/man1/$(MANPAGE)"
	@echo ""
	@echo "Service starten mit:"
	@echo "  sudo systemctl enable $(SERVICE)"
	@echo "  sudo systemctl start $(SERVICE)"
	@echo ""
	@echo "Manual anzeigen mit:"
	@echo "  man nzxt"

# Uninstall Target
uninstall:
	sudo systemctl stop $(SERVICE) || true
	sudo systemctl disable $(SERVICE) || true
	sudo rm -f /etc/systemd/system/$(SERVICE)
	sudo rm -f /usr/share/man/man1/$(MANPAGE)
	sudo rm -f /opt/nzxt_cam/$(README)
	sudo mandb -q
	sudo systemctl daemon-reload
	sudo rm -f /opt/nzxt_cam/$(TARGET)
	@echo "Deinstallation abgeschlossen"
	@echo "Hinweis: /opt/nzxt_cam/image bleibt bestehen (enthält ggf. Bilder)"

# Debug Build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Service Management Targets
start:
	sudo systemctl start $(SERVICE)

stop:
	sudo systemctl stop $(SERVICE)

restart:
	sudo systemctl restart $(SERVICE)

status:
	sudo systemctl status $(SERVICE)

enable:
	sudo systemctl enable $(SERVICE)
	@echo "Service wird beim Boot automatisch gestartet"

disable:
	sudo systemctl disable $(SERVICE)
	@echo "Autostart beim Boot deaktiviert"

logs:
	sudo journalctl -u $(SERVICE) -f

# Help
help:
	@echo "Verfügbare Targets:"
	@echo "  make          - Kompiliert das Programm"
	@echo "  make clean    - Löscht kompilierte Dateien"
	@echo "  make install  - Installiert nach /opt/nzxt_cam/ (erstellt Ordner & Service)"
	@echo "  make uninstall- Deinstalliert das Programm"
	@echo "  make debug    - Kompiliert mit Debug-Informationen"
	@echo ""
	@echo "Service Management:"
	@echo "  make start    - Startet den Service"
	@echo "  make stop     - Stoppt den Service"
	@echo "  make restart  - Startet den Service neu"
	@echo "  make status   - Zeigt Service-Status"
	@echo "  make enable   - Aktiviert Autostart"
	@echo "  make disable  - Deaktiviert Autostart"
	@echo "  make logs     - Zeigt Live-Logs"
	@echo ""
	@echo "  make help     - Zeigt diese Hilfe"

.PHONY: clean install uninstall debug start stop restart status enable disable logs help