#define _XOPEN_SOURCE 700
#define _GNU_SOURCE  // Für strcasestr
#include <unistd.h>
// nzxt.c
// NZXT Kraken LCD Temperature Monitor - Hochperformanter C-basierter Daemon
// Kompilieren mit: gcc -Wall -Wextra -O2 -std=c99 nzxt.c -o nzxt -lcairo -lcurl -lm
// Oder einfach: make

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cairo/cairo.h>
#include <dirent.h>
#include <time.h>
#include <math.h>
#include <curl/curl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define GPU_CACHE_INTERVAL 2
#define HWMON_PATH "/sys/class/hwmon"
#define IMAGE_DIR "/opt/nzxt_cam/image"
#define IMAGE_PATH "/opt/nzxt_cam/image/cpu_gpu_temp.png"

#define DAEMON_ADDRESS "http://localhost:11987"
#define DAEMON_PASSWORD "coolAdmin"

// Globale Variablen für die HTTP-Session
static CURL *curl_handle = NULL;
static char cookie_jar[256] = {0};

// Globale Variablen für gecachte hwmon-Pfade (einmalig beim Start ermittelt)
static char cpu_temp_path[512] = {0};
static char coolant_temp_path[512] = {0};

// Globale Variablen für Session-Management
static int session_initialized = 0;
static char kraken_uid[128] = {0};

// Funktionsdeklarationen
// Sensor-Funktionen
void init_sensor_paths(void);
float read_cpu_temp(void);
float read_coolant_temp(void);
void get_gpu_data(float *temp, float *usage, float *mem_usage);
float get_gpu_temp_only(void);

// CPU/RAM-Monitoring-Funktionen
static int get_cpu_stat(long *total, long *idle);
static float get_cpu_usage(long last_total, long last_idle, long curr_total, long curr_idle);
static float get_ram_usage(void);

// Grafik-Funktionen
static void lerp_temp_color(float val, int* r, int* g, int* b);
void draw_combined_image(const char *mode);

// CoolerControl-API-Funktionen
int init_coolercontrol_session(void);
int send_image_to_lcd(const char* image_path, const char* device_uid);
void cleanup_coolercontrol_session(void);

// Utility-Funktionen
int check_and_terminate_existing_instance(const char *pid_file);
void write_pid_file(const char *pid_file);
void cleanup_and_exit(int sig);

volatile sig_atomic_t running = 1;
const char *pid_file = "/tmp/nzxt_cam.pid";

void cleanup_and_exit(int sig) {
    (void)sig; // Parameter wird nicht verwendet
    unlink(pid_file);
    running = 0;
}

// Farbverlauf für Temperaturbalken (grün → orange → rot) - inline für Performance
static void lerp_temp_color(float val, int* restrict r, int* restrict g, int* restrict b) {
    if (val <= 55.0f) { 
        *r = 0; *g = 255; *b = 0; 
    } else if (val <= 65.0f) { 
        *r = 255; *g = 140; *b = 0; 
    } else if (val <= 75.0f) { 
        *r = 255; *g = 70; *b = 0; 
    } else { 
        *r = 255; *g = 0; *b = 0; 
    }
}

/**
 * Liest die CPU-Temperatur aus dem gecachten hwmon-Pfad.
 *
 * @return Temperatur in Grad Celsius (float), 0.0f bei Fehler
 */
float read_cpu_temp() {
    if (strlen(cpu_temp_path) == 0) return 0.0f;
    
    FILE *finput = fopen(cpu_temp_path, "r");
    if (!finput) return 0.0f;
    
    int t = 0;
    float temp = 0.0f;
    if (fscanf(finput, "%d", &t) == 1) {
        temp = t > 200 ? t / 1000.0f : (float)t;
    }
    fclose(finput);
    return temp;
}

/**
 * Liest die Coolant-Temperatur aus dem gecachten hwmon-Pfad.
 *
 * @return Temperatur in Grad Celsius (float), 0.0f bei Fehler
 */
float read_coolant_temp() {
    if (strlen(coolant_temp_path) == 0) return 0.0f;
    
    FILE *finput = fopen(coolant_temp_path, "r");
    if (!finput) return 0.0f;
    
    int t = 0;
    float temp = 0.0f;
    if (fscanf(finput, "%d", &t) == 1) {
        temp = t > 200 ? t / 1000.0f : (float)t;
    }
    fclose(finput);
    return temp;
}

/**
 * Liest aktuelle GPU-Daten (Temperatur, Auslastung, RAM-Auslastung) mit Caching.
 *
 * @param[out] temp      Zeiger für GPU-Temperatur (float)
 * @param[out] usage     Zeiger für GPU-Auslastung (float, Prozent)
 * @param[out] mem_usage Zeiger für GPU-RAM-Auslastung (float, Prozent)
 */
void get_gpu_data(float *temp, float *usage, float *mem_usage) {
    static time_t last_update = 0;
    static float cached_temp = 0, cached_usage = 0, cached_mem_usage = 0;
    time_t now = time(NULL);
    if (now - last_update >= GPU_CACHE_INTERVAL) {
        FILE *fp = popen("nvidia-smi --query-gpu=temperature.gpu,utilization.gpu,memory.used,memory.total --format=csv,noheader,nounits 2>/dev/null", "r");
        if (fp) {
            float mem_total = 0, mem_used = 0;
            if (fscanf(fp, "%f, %f, %f, %f", &cached_temp, &cached_usage, &mem_used, &mem_total) == 4) {
                cached_mem_usage = (mem_total > 0) ? (100.0f * mem_used / mem_total) : 0.0f;
            }
            pclose(fp);
            last_update = now;
        }
    }
    *temp = cached_temp;
    *usage = cached_usage;
    *mem_usage = cached_mem_usage;
}

/**
 * Liest nur die GPU-Temperatur für mode "def".
 *
 * @return GPU-Temperatur in Grad Celsius (float), 0.0f bei Fehler
 */
float get_gpu_temp_only() {
    static time_t last_update = 0;
    static float cached_temp = 0;
    time_t now = time(NULL);
    if (now - last_update >= GPU_CACHE_INTERVAL) {
        FILE *fp = popen("nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader,nounits 2>/dev/null", "r");
        if (fp) {
            if (fscanf(fp, "%f", &cached_temp) != 1) {
                cached_temp = 0.0f;
            }
            pclose(fp);
            last_update = now;
        }
    }
    return cached_temp;
}

/**
 * Liest CPU-Statistiken für die Auslastungsberechnung.
 *
 * @param[out] total Gesamtticks (long)
 * @param[out] idle  Idle-Ticks (long)
 * @return 1 bei Erfolg, 0 bei Fehler
 */
static int get_cpu_stat(long *restrict total, long *restrict idle) {
    FILE *fstat = fopen("/proc/stat", "r");
    if (!fstat) { 
        *total = 0; 
        *idle = 0; 
        return 0; 
    }
    
    long user, nice, system, idle_val, iowait, irq, softirq, steal;
    const int result = fscanf(fstat, "cpu %ld %ld %ld %ld %ld %ld %ld %ld", 
                       &user, &nice, &system, &idle_val, &iowait, &irq, &softirq, &steal);
    fclose(fstat);
    
    if (result == 8) {
        *idle = idle_val + iowait;
        *total = *idle + user + nice + system + irq + softirq + steal;
        return 1;
    }
    
    *total = 0;
    *idle = 0;
    return 0;
}

/**
 * Berechnet die CPU-Auslastung zwischen zwei Zeitpunkten.
 *
 * @param last_total Vorherige Gesamtticks
 * @param last_idle  Vorherige Idle-Ticks
 * @param curr_total Aktuelle Gesamtticks
 * @param curr_idle  Aktuelle Idle-Ticks
 * @return CPU-Auslastung in Prozent (float), -1.0f bei Fehler
 */
static float get_cpu_usage(long last_total, long last_idle, long curr_total, long curr_idle) {
    const long totald = curr_total - last_total;
    const long idled = curr_idle - last_idle;
    
    if (totald <= 0) return -1.0f;  // Fehler-Indikator
    
    const float usage = 100.0f * (float)(totald - idled) / (float)totald;
    return (usage >= 0.0f && usage <= 100.0f) ? usage : 0.0f;
}

/**
 * Liest die RAM-Auslastung aus /proc/meminfo.
 *
 * @return RAM-Auslastung in Prozent (float), -1.0f bei Fehler
 */
static float get_ram_usage(void) {
    FILE *fmem = fopen("/proc/meminfo", "r");
    if (!fmem) return -1.0f;
    
    long mem_total = 0, mem_free = 0, buffers = 0, cached = 0;
    char line[128];
    int found = 0;
    
    while (fgets(line, sizeof(line), fmem) && found < 4) {
        if (sscanf(line, "MemTotal: %ld", &mem_total) == 1) found++;
        else if (sscanf(line, "MemFree: %ld", &mem_free) == 1) found++;
        else if (sscanf(line, "Buffers: %ld", &buffers) == 1) found++;
        else if (sscanf(line, "Cached: %ld", &cached) == 1) found++;
    }
    fclose(fmem);
    
    if (found == 4 && mem_total > 0) {
        const long mem_used = mem_total - (mem_free + buffers + cached);
        if (mem_used >= 0) {
            return 100.0f * (float)mem_used / (float)mem_total;
        }
    }
    return -1.0f;
}

/**
 * Zeichnet das kombinierte Temperatur- und Auslastungsbild und speichert es als PNG.
 *
 * @param mode Anzeige-Modus ("def", "1", "2", "3")
 */
void draw_combined_image(const char *mode) {
    // CPU- und RAM-Auslastung nur für Modi berechnen, die sie auch anzeigen
    float cpu_usage = 0.0f, ram_usage = 0.0f;
    static long last_total = 0, last_idle = 0;
    
    if (strcmp(mode, "def") != 0) {
        // CPU-Auslastung berechnen (nur für Modi 1, 2, 3)
        long curr_total = 0, curr_idle = 0;
        if (get_cpu_stat(&curr_total, &curr_idle)) {
            if (last_total != 0 && last_idle != 0) {
                const float usage = get_cpu_usage(last_total, last_idle, curr_total, curr_idle);
                cpu_usage = (usage >= 0.0f) ? usage : 0.0f;
            }
            last_total = curr_total;
            last_idle = curr_idle;
        }

        // RAM-Auslastung berechnen
        const float usage = get_ram_usage();
        ram_usage = (usage >= 0.0f) ? usage : 0.0f;
    }

    // Sensorwerte effizient auslesen
    float cpu_temp = read_cpu_temp();
    float gpu_temp, gpu_usage, gpu_ram_usage;
    
    // GPU-Daten je nach Modus abrufen
    if (strcmp(mode, "def") == 0) {
        // Nur GPU-Temperatur für mode "def" (weniger I/O)
        gpu_temp = get_gpu_temp_only();
        gpu_usage = 0.0f;
        gpu_ram_usage = 0.0f;
    } else {
        // Vollständige GPU-Daten für Modi 1, 2, 3
        get_gpu_data(&gpu_temp, &gpu_usage, &gpu_ram_usage);
    }
    
    float coolant_temp = read_coolant_temp();

    // Layout-Konstanten
    const int W = 240, H = 240;
    const int box_w = 70, box_h = 60, box_gap = 10;
    const int bar_w = 220, bar_h = 30, bar_gap = 6;
    const int bar_x = (W - bar_w) / 2;
    const int cpu_bar_y = (H - (2 * bar_h + bar_gap)) / 2;
    const int gpu_bar_y = cpu_bar_y + bar_h + bar_gap;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, W, H);
    if (!surface || cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        if (surface) cairo_surface_destroy(surface);
        return; // Fehler beim Surface erstellen
    }
    
    cairo_t *cr = cairo_create(surface);
    if (!cr || cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        if (cr) cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return; // Fehler beim Cairo-Context erstellen
    }

    // Hintergrund komplett schwarz
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    // Layout-Koordinaten einmalig berechnen
    const int cpu_box_x = 2 + box_w + box_gap;                  // X-Position CPU-Box (mittig oben)
    const int cpu_box_y = 2;                                     // Y-Position CPU-Box (oberer Rand)
    const int coolant_box_x = 2 + 2 * (box_w + box_gap);        // X-Position Coolant-Box (rechts oben)
    const int coolant_box_y = 2;                                 // Y-Position Coolant-Box (oberer Rand)
    const int gpu_box_x = 2 + box_w + box_gap;                  // X-Position GPU-Box (mittig unten)
    const int gpu_box_y = H - box_h - 2;                        // Y-Position GPU-Box (unterer Rand)

    // --- CPU-Balken (mittig, ohne Text, mit Rand) ---
    int r, g, b;
    lerp_temp_color(cpu_temp, &r, &g, &b);
    const int cpu_val_w = (cpu_temp > 0.0f) ? 
        (int)((cpu_temp / 100.0f) * bar_w) : 0;
    const int safe_cpu_val_w = (cpu_val_w < 0) ? 0 : 
        (cpu_val_w > bar_w) ? bar_w : cpu_val_w;
    
    cairo_set_source_rgb(cr, 0.16, 0.16, 0.16);
    cairo_rectangle(cr, bar_x, cpu_bar_y, bar_w, bar_h);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, r/255.0, g/255.0, b/255.0);
    cairo_rectangle(cr, bar_x, cpu_bar_y, safe_cpu_val_w, bar_h);
    cairo_fill(cr);
    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgb(cr, 0.08, 0.08, 0.08);
    cairo_rectangle(cr, bar_x, cpu_bar_y, bar_w, bar_h);
    cairo_stroke(cr);

    // --- GPU-Balken (darunter, ohne Text, mit Rand) ---
    lerp_temp_color(gpu_temp, &r, &g, &b);
    const int gpu_val_w = (gpu_temp > 0.0f) ? 
        (int)((gpu_temp / 100.0f) * bar_w) : 0;
    const int safe_gpu_val_w = (gpu_val_w < 0) ? 0 : 
        (gpu_val_w > bar_w) ? bar_w : gpu_val_w;
    
    cairo_set_source_rgb(cr, 0.16, 0.16, 0.16);
    cairo_rectangle(cr, bar_x, gpu_bar_y, bar_w, bar_h);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, r/255.0, g/255.0, b/255.0);
    cairo_rectangle(cr, bar_x, gpu_bar_y, safe_gpu_val_w, bar_h);
    cairo_fill(cr);
    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgb(cr, 0.08, 0.08, 0.08);
    cairo_rectangle(cr, bar_x, gpu_bar_y, bar_w, bar_h);
    cairo_stroke(cr);

    // --- CPU/GPU Labels für alle Modi außer "def" (aligned mit Balken) ---
    if (strcmp(mode, "def") != 0) {
        cairo_select_font_face(cr, "DejaVuSans-Bold", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 22);
        cairo_set_source_rgb(cr, 0, 0, 0);  // Schwarz für bessere Sichtbarkeit
        
        cairo_text_extents_t text_ext;
        
        // CPU-Label oben links (aligned mit CPU-Balken)
        cairo_text_extents(cr, "CPU", &text_ext);
        cairo_move_to(cr, 15 + (40 - text_ext.width) / 2, 
                      cpu_bar_y + (bar_h + text_ext.height) / 2);
        cairo_show_text(cr, "CPU");
        
        // GPU-Label unten links (aligned mit GPU-Balken)
        cairo_text_extents(cr, "GPU", &text_ext);
        cairo_move_to(cr, 15 + (40 - text_ext.width) / 2, 
                      gpu_bar_y + (bar_h + text_ext.height) / 2);
        cairo_show_text(cr, "GPU");
    }

    // --- Spezielle Text-Labels nur für Mode "def" ---
    if (strcmp(mode, "def") == 0) {
        cairo_select_font_face(cr, "DejaVuSans-Bold", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 26);
        cairo_set_source_rgb(cr, 1, 1, 1);
        
        cairo_text_extents_t text_ext;
        
        // CPU-Label oben links (wie vorher)
        cairo_text_extents(cr, "CPU", &text_ext);
        cairo_move_to(cr, 1 + (50 - text_ext.width) / 2, 70);
        cairo_show_text(cr, "CPU");
        
        // GPU-Label unten links (wie vorher)
        cairo_text_extents(cr, "GPU", &text_ext);
        cairo_move_to(cr, 1 + (50 - text_ext.width) / 2, H - 50);
        cairo_show_text(cr, "GPU");
    }

    // --- Vertikale Balken für Mode 1 ---
    if (strcmp(mode, "1") == 0) {
        // Konstanten für bessere Performance
        const int bar_base_x = 10;
        const int bar_base_y = 10;
        const int bar_width = 6;
        const int bar_height = box_h - 1;
        const int bar_spacing = 6;
        
        // CPU-Auslastungsbalken (blau)
        const int cpu_fill = (int)(bar_height * cpu_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.3, 0.7, 1.0);
        cairo_rectangle(cr, bar_base_x, bar_base_y + bar_height - cpu_fill, 
                       bar_width, cpu_fill);
        cairo_fill(cr);
        
        // RAM-Auslastungsbalken (lila)
        const int ram_fill = (int)(bar_height * ram_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.8, 0.5, 1.0);
        cairo_rectangle(cr, bar_base_x + bar_width + bar_spacing, 
                       bar_base_y + bar_height - ram_fill, bar_width, ram_fill);
        cairo_fill(cr);

        // GPU-Auslastungsbalken (grün)
        const int gpu_y_base = H - bar_height - 10;
        const int gpu_fill = (int)(bar_height * gpu_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.4, 1.0, 0.4);
        cairo_rectangle(cr, bar_base_x, gpu_y_base + bar_height - gpu_fill, 
                       bar_width, gpu_fill);
        cairo_fill(cr);
        
        // GPU-RAM-Auslastungsbalken (lila)
        const int gpu_ram_fill = (int)(bar_height * gpu_ram_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.8, 0.5, 1.0);
        cairo_rectangle(cr, bar_base_x + bar_width + bar_spacing, 
                       gpu_y_base + bar_height - gpu_ram_fill, bar_width, gpu_ram_fill);
        cairo_fill(cr);
    }
    // --- Zwei Kreisdiagramme für Mode 2 ---
    if (strcmp(mode, "2") == 0) {
        // Konstanten für bessere Performance
        const double cx = 26.0;         // Einheitliche X-Position
        const double cy_top = 26.0;     // Y-Position oben
        const double cy_bot = H - 26.0; // Y-Position unten
        const double radius = 22.0;     // Einheitlicher Radius
        const double thickness = 6.0;   // Einheitliche Dicke
        const double inner_radius = radius - thickness - 2.0;
        
        // Oben: CPU-Auslastung (blau) und RAM-Auslastung (lila)
        // Hintergrundkreise (grau)
        cairo_set_line_width(cr, thickness);
        cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        cairo_arc(cr, cx, cy_top, radius, 0, 2 * M_PI);
        cairo_stroke(cr);
        cairo_arc(cr, cx, cy_top, inner_radius, 0, 2 * M_PI);
        cairo_stroke(cr);
        
        // CPU-Auslastung (äußerer Ring, blau)
        const double cpu_angle = 2 * M_PI * cpu_usage / 100.0;
        cairo_set_source_rgb(cr, 0.3, 0.7, 1.0);
        cairo_arc(cr, cx, cy_top, radius, -M_PI/2, -M_PI/2 + cpu_angle);
        cairo_stroke(cr);
        
        // RAM-Auslastung (innerer Ring, lila)
        const double ram_angle = 2 * M_PI * ram_usage / 100.0;
        cairo_set_source_rgb(cr, 0.8, 0.5, 1.0);
        cairo_arc(cr, cx, cy_top, inner_radius, -M_PI/2, -M_PI/2 + ram_angle);
        cairo_stroke(cr);

        // Unten: GPU-Auslastung (grün) und GPU-RAM-Auslastung (lila)
        // Hintergrundkreise (grau)
        cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        cairo_arc(cr, cx, cy_bot, radius, 0, 2 * M_PI);
        cairo_stroke(cr);
        cairo_arc(cr, cx, cy_bot, inner_radius, 0, 2 * M_PI);
        cairo_stroke(cr);
        
        // GPU-Auslastung (äußerer Ring, grün)
        const double gpu_angle = 2 * M_PI * gpu_usage / 100.0;
        cairo_set_source_rgb(cr, 0.4, 1.0, 0.4);
        cairo_arc(cr, cx, cy_bot, radius, -M_PI/2, -M_PI/2 + gpu_angle);
        cairo_stroke(cr);
        
        // GPU-RAM-Auslastung (innerer Ring, lila)
        const double gpu_ram_angle = 2 * M_PI * gpu_ram_usage / 100.0;
        cairo_set_source_rgb(cr, 0.8, 0.5, 1.0);
        cairo_arc(cr, cx, cy_bot, inner_radius, -M_PI/2, -M_PI/2 + gpu_ram_angle);
        cairo_stroke(cr);
    }
    // --- Horizontale Auslastungsbalken für Mode 3 ---
    if (strcmp(mode, "3") == 0) {
        // Konstanten für bessere Performance und Übersichtlichkeit
        const int offset_x = bar_x + 1;
        const int bar_height_small = 5;
        const int bar_spacing = 6;
        const int max_bar_width = bar_w - 2;
        
        // CPU-Auslastung (blau)
        const int cpu_usage_width = (int)(max_bar_width * cpu_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.3, 0.7, 1.0);
        cairo_rectangle(cr, offset_x, cpu_bar_y + 1, cpu_usage_width, bar_height_small);
        cairo_fill(cr);
        
        // RAM-Auslastung (lila)
        const int ram_usage_width = (int)(max_bar_width * ram_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.8, 0.5, 1.0);
        cairo_rectangle(cr, offset_x, cpu_bar_y + 1 + bar_spacing, ram_usage_width, bar_height_small);
        cairo_fill(cr);
        
        // GPU-Auslastung (gelb)
        const int gpu_usage_width = (int)(max_bar_width * gpu_usage / 100.0f);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_rectangle(cr, offset_x, gpu_bar_y + 1, gpu_usage_width, bar_height_small);
        cairo_fill(cr);
        
        // GPU-RAM-Auslastung (lila)
        const int gpu_ram_usage_width = (int)(max_bar_width * gpu_ram_usage / 100.0f);
        cairo_set_source_rgb(cr, 0.8, 0.5, 1.0);
        cairo_rectangle(cr, offset_x, gpu_bar_y + 1 + bar_spacing, gpu_ram_usage_width, bar_height_small);
        cairo_fill(cr);
    }

    // Schriftart für Temperaturwerte setzen
    cairo_select_font_face(cr, "DejaVuSans-Bold", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    // Layout- und Textvariablen für Temperaturanzeigen
    char temp_str[8];
    char cool_str[16];
    const double font_size = box_h * 1.50;
    const double deg_font_size = font_size * 0.75;
    const double coolant_font_size = font_size * 0.30;
    cairo_text_extents_t ext, deg_ext, cool_ext, deg_ext_cool, deg_ext_gpu;

    // CPU-Temperaturanzeige (zentriert, groß)
    snprintf(temp_str, sizeof(temp_str), "%d", (int)cpu_temp);
    cairo_set_font_size(cr, font_size);
    cairo_text_extents(cr, temp_str, &ext);
    const double cpu_temp_x = cpu_box_x + (box_w - ext.width) / 2;
    const double cpu_temp_y = cpu_box_y + ext.height + 8;
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, cpu_temp_x, cpu_temp_y);
    cairo_show_text(cr, temp_str);
    
    // Grad-Symbol für CPU
    cairo_set_font_size(cr, deg_font_size);
    cairo_text_extents(cr, "°", &deg_ext);
    cairo_move_to(cr, cpu_temp_x + ext.width + 8, cpu_temp_y - deg_ext.height + 4);
    cairo_show_text(cr, "°");

    // Coolant-Temperaturanzeige (klein, unter Box) nur anzeigen, wenn CPU <= 99°C
    if (cpu_temp <= 99.0f) {
        snprintf(cool_str, sizeof(cool_str), "%.1f", coolant_temp);
        cairo_set_font_size(cr, coolant_font_size);
        cairo_text_extents(cr, cool_str, &cool_ext);
        const double cool_x = coolant_box_x + (box_w - cool_ext.width) / 2;
        const double cool_y = coolant_box_y + box_h + 8;
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_move_to(cr, cool_x + 4, cool_y);
        cairo_show_text(cr, cool_str);
        
        // Grad-Symbol für Coolant
        cairo_set_font_size(cr, deg_font_size * 0.30);
        cairo_text_extents(cr, "°", &deg_ext_cool);
        cairo_move_to(cr, cool_x + cool_ext.width + 8, cool_y - deg_ext_cool.height - 2);
        cairo_show_text(cr, "°");
    }

    // GPU-Temperaturanzeige (zentriert, groß)
    snprintf(temp_str, sizeof(temp_str), "%d", (int)gpu_temp);
    cairo_set_font_size(cr, font_size);
    cairo_text_extents(cr, temp_str, &ext);
    const double gpu_temp_x = gpu_box_x + (box_w - ext.width) / 2;
    const double gpu_temp_y = gpu_box_y + box_h - 8;
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, gpu_temp_x, gpu_temp_y);
    cairo_show_text(cr, temp_str);
    
    // Grad-Symbol für GPU
    cairo_set_font_size(cr, deg_font_size);
    cairo_text_extents(cr, "°", &deg_ext_gpu);
    cairo_move_to(cr, gpu_temp_x + ext.width + 8, gpu_temp_y - deg_ext_gpu.height + 4);
    cairo_show_text(cr, "°");

    // Änderungserkennung für bessere Performance
    static float last_cpu_temp = -1.0f, last_gpu_temp = -1.0f, last_coolant_temp = -1.0f;
    static float last_cpu_usage = -1.0f, last_gpu_usage = -1.0f, last_ram_usage = -1.0f, last_gpu_ram_usage = -1.0f;
    static int did_mkdir = 0;
    
    // Toleranzen für Änderungserkennung
    const float TEMP_TOLERANCE = 0.1f;
    const float USAGE_TOLERANCE = 0.5f;
    
    // Prüfe auf signifikante Änderungen
    const int needs_update = (
        fabsf(cpu_temp - last_cpu_temp) > TEMP_TOLERANCE ||
        fabsf(gpu_temp - last_gpu_temp) > TEMP_TOLERANCE ||
        fabsf(coolant_temp - last_coolant_temp) > TEMP_TOLERANCE ||
        fabsf(cpu_usage - last_cpu_usage) > USAGE_TOLERANCE ||
        fabsf(gpu_usage - last_gpu_usage) > USAGE_TOLERANCE ||
        fabsf(ram_usage - last_ram_usage) > USAGE_TOLERANCE ||
        fabsf(gpu_ram_usage - last_gpu_ram_usage) > USAGE_TOLERANCE ||
        !did_mkdir
    );

    if (needs_update) {
        if (!did_mkdir) {
            struct stat st = {0};
            if (stat(IMAGE_DIR, &st) == -1) {
                mkdir(IMAGE_DIR, 0755);
            }
            did_mkdir = 1;
        }
        
        if (cairo_surface_write_to_png(surface, IMAGE_PATH) == CAIRO_STATUS_SUCCESS) {
            last_cpu_temp = cpu_temp;
            last_gpu_temp = gpu_temp;
            last_coolant_temp = coolant_temp;
            last_cpu_usage = cpu_usage;
            last_gpu_usage = gpu_usage;
            last_ram_usage = ram_usage;
            last_gpu_ram_usage = gpu_ram_usage;
            
            // Bild 2x an das Display senden (das behebt einen fehler!!)
            if (session_initialized && strlen(kraken_uid) > 0) {
                for (int i = 0; i < 2; ++i) {
                    send_image_to_lcd(IMAGE_PATH, kraken_uid);
                }
            }
        }
    }
    
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

/**
 * Initialisiert cURL und authentifiziert sich beim CoolerControl Daemon
 */
int init_coolercontrol_session() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
    if (!curl_handle) return 0;
    
    // Cookie-Jar für Session-Management
    snprintf(cookie_jar, sizeof(cookie_jar), "/tmp/nzxt_cookies_%d.txt", getpid());
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, cookie_jar);
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, cookie_jar);
    
    // Login beim Daemon
    char login_url[128];
    snprintf(login_url, sizeof(login_url), "%s/login", DAEMON_ADDRESS);
    
    char userpwd[64];
    snprintf(userpwd, sizeof(userpwd), "CCAdmin:%s", DAEMON_PASSWORD);
    
    curl_easy_setopt(curl_handle, CURLOPT_URL, login_url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, userpwd);
    curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, "");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, NULL); // Ignoriere Response
    
    CURLcode res = curl_easy_perform(curl_handle);
    long response_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    
    return (res == CURLE_OK && (response_code == 200 || response_code == 204));
}

/**
 * Sendet ein Bild direkt an das LCD des CoolerControl Geräts
 */
int send_image_to_lcd(const char* image_path, const char* device_uid) {
    if (!curl_handle || !image_path || !device_uid) return 0;
    
    // Prüfe ob Datei existiert
    FILE *test_file = fopen(image_path, "rb");
    if (!test_file) return 0;
    fclose(test_file);
    
    // URL für LCD-Image-Upload
    char upload_url[256];
    snprintf(upload_url, sizeof(upload_url), 
             "%s/devices/%s/settings/lcd/lcd/images", DAEMON_ADDRESS, device_uid);
    
    // MIME-Type bestimmen
    const char* mime_type = "image/png";
    if (strstr(image_path, ".jpg") || strstr(image_path, ".jpeg")) {
        mime_type = "image/jpeg";
    } else if (strstr(image_path, ".gif")) {
        mime_type = "image/gif";
    }
    
    // Multipart Form erstellen
    curl_mime *form = curl_mime_init(curl_handle);
    curl_mimepart *field;
    
    // mode field
    field = curl_mime_addpart(form);
    curl_mime_name(field, "mode");
    curl_mime_data(field, "image", CURL_ZERO_TERMINATED);
    
    // brightness field
    field = curl_mime_addpart(form);
    curl_mime_name(field, "brightness");
    curl_mime_data(field, "100", CURL_ZERO_TERMINATED);
    
    // orientation field
    field = curl_mime_addpart(form);
    curl_mime_name(field, "orientation");
    curl_mime_data(field, "0", CURL_ZERO_TERMINATED);
    
    // images[] field (das eigentliche Bild)
    field = curl_mime_addpart(form);
    curl_mime_name(field, "images[]");
    curl_mime_filedata(field, image_path);
    curl_mime_type(field, mime_type);
    
    // cURL konfigurieren
    curl_easy_setopt(curl_handle, CURLOPT_URL, upload_url);
    curl_easy_setopt(curl_handle, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "PUT");
    
    // Request ausführen
    CURLcode res = curl_easy_perform(curl_handle);
    long response_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    
    // Cleanup
    curl_mime_free(form);
    curl_easy_setopt(curl_handle, CURLOPT_MIMEPOST, NULL);
    curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, NULL);
    
    return (res == CURLE_OK && response_code == 200);
}

/**
 * Beendet die CoolerControl Session und räumt auf
 */
void cleanup_coolercontrol_session() {
    static int cleanup_done = 0;
    if (cleanup_done) return;
    cleanup_done = 1;
    
    if (curl_handle) {
        curl_easy_cleanup(curl_handle);
        curl_handle = NULL;
    }
    curl_global_cleanup();
    unlink(cookie_jar); // Cookie-Datei löschen
}

/**
 * Prüft, ob eine andere Instanz läuft, und beendet sie ggf.
 *
 * @param pid_file Pfad zur PID-Datei
 * @return 1, wenn Instanz beendet wurde, sonst 0
 */
int check_and_terminate_existing_instance(const char *pid_file) {
    FILE *file = fopen(pid_file, "r");
    if (file) {
        pid_t existing_pid;
        if (fscanf(file, "%d", &existing_pid) == 1) {
            if (kill(existing_pid, 0) == 0) {
                kill(existing_pid, SIGTERM);
                fclose(file);
                // Warten, bis Prozess beendet ist
                for (int i = 0; i < 10; ++i) {
                    if (kill(existing_pid, 0) != 0) break;
                    struct timespec ts_usleep = {0, 100000000};
                    nanosleep(&ts_usleep, NULL);
                }
                return 1;
            }
        }
        fclose(file);
    }
    return 0;
}

/**
 * Schreibt die eigene PID in die PID-Datei.
 *
 * @param pid_file Pfad zur PID-Datei
 */
void write_pid_file(const char *pid_file) {
    FILE *file = fopen(pid_file, "w");
    if (file) {
        fprintf(file, "%d", getpid());
        fclose(file);
    }
}

/**
 * Initialisiert die hwmon-Sensor-Pfade beim Start (einmalig)
 */
void init_sensor_paths(void) {
    DIR *dir = opendir(HWMON_PATH);
    if (!dir) return;
    
    struct dirent *entry;
    char label_path[512], label[64];
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        for (int i = 1; i <= 9; ++i) {
            snprintf(label_path, sizeof(label_path), HWMON_PATH"/%s/temp%d_label", entry->d_name, i);
            FILE *flabel = fopen(label_path, "r");
            if (!flabel) continue;
            
            if (fgets(label, sizeof(label), flabel)) {
                // CPU-Temperatur Pfad cachen
                if (strstr(label, "Package id 0") && strlen(cpu_temp_path) == 0) {
                    snprintf(cpu_temp_path, sizeof(cpu_temp_path), HWMON_PATH"/%s/temp%d_input", entry->d_name, i);
                }
                // Coolant-Temperatur Pfad cachen
                else if ((strstr(label, "Coolant") || strstr(label, "coolant")) && strlen(coolant_temp_path) == 0) {
                    snprintf(coolant_temp_path, sizeof(coolant_temp_path), HWMON_PATH"/%s/temp%d_input", entry->d_name, i);
                }
            }
            fclose(flabel);
        }
    }
    closedir(dir);
}

/**
 * Hauptfunktion: Initialisiert, verarbeitet Argumente, startet Hauptloop.
 *
 * @param argc Argumentanzahl
 * @param argv Argumentvektor
 * @return Exit-Code
 */
int main(int argc, char *argv[]) {
    // Signal-Handler setzen
    struct sigaction sa;
    sa.sa_handler = cleanup_and_exit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Standard: Modus "def" (ohne vertikale balken)
    char mode[8] = "def";
    for (int i = 1; i < argc - 1; ++i) {
        if (strcmp(argv[i], "--mode") == 0) {
            if (
                strncmp(argv[i + 1], "def", 3) == 0 ||
                strncmp(argv[i + 1], "1", 1) == 0 ||
                strncmp(argv[i + 1], "2", 1) == 0 ||
                strncmp(argv[i + 1], "3", 1) == 0
            ) {
                strncpy(mode, argv[i + 1], sizeof(mode) - 1);
                mode[sizeof(mode) - 1] = '\0';
            } else {
                fprintf(stderr, "Fehler: --mode erlaubt nur 'def', '1', '2', '3'!\n");
                fprintf(stderr, "Beispiel: ./nzxt --mode def   ./nzxt --mode 1   ./nzxt --mode 2\n");
                exit(1);
            }
        }
    }

    // Überprüfen und beenden einer laufenden Instanz
    check_and_terminate_existing_instance(pid_file);
    // PID der aktuellen Instanz schreiben
    write_pid_file(pid_file);

    // CoolerControl Session einmalig initialisieren
    if (init_coolercontrol_session()) {
        // Verwende direkt die hardcodierte UID ohne API-Aufruf
        strcpy(kraken_uid, "8d4becb03bca2a8e8d4213ac376a1094f39d2786f688549ad3b6a591c3affdf9");
        printf("CoolerControl: Connected to Kraken LCD\n");
        session_initialized = 1;
    } else {
        printf("Warning: Could not initialize CoolerControl session\n");
    }

    // Sensor-Pfade initialisieren
    init_sensor_paths();

    // Cleanup bei Exit
    atexit(cleanup_coolercontrol_session);
    
    while (running) {
        draw_combined_image(mode);
        struct timespec ts = {2, 500000000}; // 2,5 Sekunden
        nanosleep(&ts, NULL);
    }
    
    // Cleanup wird automatisch durch atexit() aufgerufen
    unlink(pid_file);
    return 0;
}
