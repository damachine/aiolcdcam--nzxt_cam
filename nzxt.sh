#!/bin/bash
#
# nzxt.sh
#

#while true; do
#    # CPU-Bild erzeugen (überschreibt die Datei)
#    python nzxt.py cpu_stats || echo "⚠️ CPU-Statserzeugung fehlgeschlagen"
#    sleep 0.33

#    # CPU-Bild anzeigen
#    for _ in {1..3}; do
#        python /bin/cc.py --match kraken --channel lcd --image "/opt/nzxt_cam/image/cpu_status.png"
#        sleep 1
#    done

#    # GPU-Bild erzeugen (überschreibt die Datei)
#    python nzxt.py gpu_stats || echo "⚠️ GPU-Statserzeugung fehlgeschlagen"
#    sleep 0.3

#    # GPU-Bild anzeigen
#    for _ in {1..3}; do
#        python /bin/cc.py --match kraken --channel lcd --image "/opt/nzxt_cam/image/gpu_status.png"
#        sleep 1
#    done
#done

#while true; do
#    # Kombiniertes Bild erzeugen (überschreibt die Datei)
#    python nzxt.py || echo "⚠️ Statserzeugung fehlgeschlagen"
#    sleep 0.33

#    # Bild anzeigen (3x wiederholen)
#    for _ in {1..3}; do
#        python /bin/cc.py --match kraken --channel lcd --image "/opt/nzxt_cam/image/cpu_gpu_temp.png"
#        sleep 1
#    done
#done

# Programm zum Bild erzeugen der Kraken-Daten
if [ -e /opt/nzxt_cam/nzxt ]; then
    (cd /opt/nzxt_cam/ && ./nzxt --mode 3) &
    sleep 1
fi

while true; do
    sleep 2.5

    for _ in {1..2}; do
        python /opt/coolercontrol-extra/cc.py --match kraken --channel lcd --image "/opt/nzxt_cam/image/cpu_gpu_temp.png"
    done
done
