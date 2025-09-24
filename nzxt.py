import os
import psutil
import subprocess
from PIL import Image, ImageDraw, ImageFont


def get_cpu_temp():
    temps = psutil.sensors_temperatures()
    fallback = None
    for chip, entries in temps.items():
        for entry in entries:
            if entry.label in ("Package id 0", "Tdie", "Tctl"):
                return round(entry.current, 1)
            if fallback is None and entry.current:
                fallback = entry.current
    return round(fallback or 0.0, 1)


def get_cpu_usage():
    return psutil.cpu_percent(interval=0.3)


def get_gpu_temp():
    try:
        output = subprocess.check_output(
            ["nvidia-smi", "--query-gpu=temperature.gpu", "--format=csv,noheader,nounits"],
            encoding='utf-8'
        )
        return round(float(output.strip()), 1)
    except Exception as e:
        print(f"⚠️ GPU-Auslesefehler: {e}")
        return 0.0


def get_gpu_usage():
    try:
        output = subprocess.check_output(
            ["nvidia-smi", "--query-gpu=utilization.gpu", "--format=csv,noheader,nounits"],
            encoding='utf-8'
        )
        return round(float(output.strip()), 1)
    except Exception as e:
        print(f"⚠️ GPU-Auslastungsauslesefehler: {e}")
        return 0.0


def lerp_temp_color(val):
    if val <= 55:
        return (0, 255, 0)
    elif val <= 65:
        return (255, 140, 0)
    elif val <= 75:
        return (255, 70, 0)
    else:
        return (255, 0, 0)


def draw_combined_image(cpu_temp, gpu_temp, cpu_usage, gpu_usage):
    W, H = 240, 240
    img = Image.new("RGB", (W, H), "black")
    draw = ImageDraw.Draw(img)

    try:
        font_temp = ImageFont.truetype("DejaVuSans-Bold.ttf", 74)
        font_label = ImageFont.truetype("DejaVuSans-Bold.ttf", 14)
    except:
        font_temp = font_label = ImageFont.load_default()

    bar_w = 220
    bar_h = 35
    usage_h = 10
    bar_x = (W - bar_w) // 2
    gap = 3

    def draw_cpu_block(temp, usage, top_y):
        label = "CPU:"
        temp_text = f"{int(temp)}°"
        label_w = draw.textlength(label, font=font_label)
        temp_w = draw.textlength(temp_text, font=font_temp)
        full_w = label_w + 6 + temp_w
        text_x = (W - full_w) // 2

        # Text anzeigen
        draw.text((text_x, top_y - 6), label, fill="white", font=font_label)
        draw.text((text_x + label_w + 6, top_y - 8), temp_text, fill="white", font=font_temp)

        text_bottom_y = top_y + font_temp.size

        # Temperaturbalken direkt unter dem Auslastungsbalken
        bar_y = text_bottom_y + usage_h - 10
        val_w = int(((temp - 0) / (105 - 0)) * bar_w)
        val_w = max(0, min(val_w, bar_w))
        draw.rectangle([(bar_x - 2, bar_y - 2), (bar_x + bar_w + 2, bar_y + bar_h + 2)], fill=(25, 25, 25))
        draw.rectangle([(bar_x, bar_y), (bar_x + bar_w, bar_y + bar_h)], fill=(40, 40, 40))
        draw.rectangle([(bar_x, bar_y), (bar_x + val_w, bar_y + bar_h)], fill=lerp_temp_color(temp))

        # CPU-Auslastungsbalken direkt unter dem Text
        usage_w = int((usage / 100) * bar_w)
        draw.rectangle([(bar_x, text_bottom_y), (bar_x + usage_w, text_bottom_y + usage_h)], fill=(50, 100, 255))

        return bar_y + bar_h

    def draw_gpu_block(temp, usage, top_y):
        # Temperaturbalken
        val_w = int(((temp - 0) / (95 - 0)) * bar_w)
        val_w = max(0, min(val_w, bar_w))
        bar_y = top_y
        draw.rectangle([(bar_x - 2, bar_y - 2), (bar_x + bar_w + 2, bar_y + bar_h + 2)], fill=(25, 25, 25))
        draw.rectangle([(bar_x, bar_y), (bar_x + bar_w, bar_y + bar_h)], fill=(40, 40, 40))
        draw.rectangle([(bar_x, bar_y), (bar_x + val_w, bar_y + bar_h)], fill=lerp_temp_color(temp))

        # GPU-Auslastungsbalken direkt darunter
        usage_w = int((usage / 100) * bar_w)
        usage_y = bar_y + bar_h - 10
        draw.rectangle([(bar_x, usage_y), (bar_x + usage_w, usage_y + usage_h)], fill=(0, 255, 200))
        # Text direkt unter GPU-Auslastung
        text_y = usage_y + usage_h + 6
        label = "GPU:"
        temp_text = f"{int(temp)}°"
        label_w = draw.textlength(label, font=font_label)
        temp_w = draw.textlength(temp_text, font=font_temp)
        full_w = label_w + 6 + temp_w
        text_x = (W - full_w) // 2
        draw.text((text_x, text_y), label, fill="white", font=font_label)
        draw.text((text_x + label_w + 6, text_y - 6), temp_text, fill="white", font=font_temp)

    # Blöcke zeichnen
    next_y = draw_cpu_block(cpu_temp, cpu_usage, top_y=12)
    draw_gpu_block(gpu_temp, gpu_usage, top_y=next_y + gap)

    return img


def generate_combined_image():
    cpu_temp = get_cpu_temp()
    gpu_temp = get_gpu_temp()
    cpu_usage = get_cpu_usage()
    gpu_usage = get_gpu_usage()
    img = draw_combined_image(cpu_temp, gpu_temp, cpu_usage, gpu_usage)

    os.makedirs("image", exist_ok=True)
    filepath = os.path.join("image", "cpu_gpu_temp.png")
    img.save(filepath, format="PNG")
    print(f"✅ Bild gespeichert: {filepath}")


if __name__ == "__main__":
    generate_combined_image()
