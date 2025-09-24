# NZXT Kraken LCD Temperature Monitor

Ein C-basierter Daemon, der CPU- und GPU-Temperaturen überwacht und diese grafisch auf dem LCD-Display eines NZXT Kraken Wasserkühlers anzeigt.

## 📁 Projektstruktur

```
nzxt_cam/
├── nzxt.c              # Hauptprogramm (C-Code)
├── Makefile            # Build- und Installations-Automatisierung
├── docs/               # Dokumentation
│   └── nzxt.1         # Manual Page
├── systemd/           # Systemd Service-Dateien
│   └── nzxt-cam.service
├── image/             # Generierte Bilder (lokal)
└── README.md          # Diese Datei
```

## 🚀 Installation

```bash
# Kompilieren
make

# Systemweit installieren (erstellt /opt/nzxt_cam/)
sudo make install

# Service aktivieren und starten
make enable
make start
```

## 📖 Verwendung

### Kommandozeile
```bash
# Standard-Modus (nur Temperaturen)
/opt/nzxt_cam/nzxt --mode def

# Mit Auslastungsbalken
/opt/nzxt_cam/nzxt --mode 1

# Mit Kreisdiagrammen  
/opt/nzxt_cam/nzxt --mode 2

# Mit horizontalen Balken
/opt/nzxt_cam/nzxt --mode 3
```

### Service Management
```bash
make start      # Service starten
make stop       # Service stoppen
make restart    # Service neustarten
make status     # Status anzeigen
make logs       # Live-Logs anzeigen
make enable     # Autostart aktivieren
make disable    # Autostart deaktivieren
```

## 📋 Abhängigkeiten

- **CoolerControl** - Muss installiert und gestartet sein
- **Cairo-Bibliothek** - Für Bildgenerierung
- **cURL** - Für REST-API-Kommunikation
- **NVIDIA-SMI** - Für GPU-Daten (bei NVIDIA-Karten)

### Ubuntu/Debian Installation:
```bash
sudo apt update
sudo apt install libcairo2-dev libcurl4-openssl-dev build-essential pkg-config
```

## 🔧 Deinstallation

```bash
make uninstall
```

## 📚 Dokumentation

```bash
# Manual Page anzeigen
man nzxt

# Makefile-Hilfe
make help
```

## 📊 Anzeige-Modi

| Modus | Beschreibung |
|-------|-------------|
| `def` | Nur Temperaturen (Standard) |
| `1`   | Temperaturen + vertikale Auslastungsbalken |
| `2`   | Temperaturen + Kreisdiagramme |
| `3`   | Temperaturen + horizontale Auslastungsbalken |

## 📁 Installierte Dateien

- `/opt/nzxt_cam/nzxt` - Hauptprogramm
- `/opt/nzxt_cam/README.md` - Diese Dokumentation
- `/opt/nzxt_cam/image/` - Generierte Bilder
- `/etc/systemd/system/nzxt-cam.service` - Systemd Service
- `/usr/share/man/man1/nzxt.1` - Manual Page

## 🐛 Debugging

```bash
# Debug-Build kompilieren
make debug

# Service-Logs anzeigen
make logs

# Status prüfen
make status
```

## 👨‍💻 Entwicklung

```bash
# Projekt klonen/bearbeiten
git clone <repository>
cd nzxt_cam

# Entwicklungs-Zyklus
make clean
make
./nzxt --mode def

# Installieren für Tests
sudo make install
```

---
**Autor:** damachine  
**Version:** 1.0  
**Lizenz:** Open Source
