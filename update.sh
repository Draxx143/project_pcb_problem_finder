#!/bin/bash

echo "🔄 Pulling latest changes from GitHub..."
git pull

echo "🔑 Setting executable permissions..."
chmod +x run.sh
chmod +x setup_linux.sh

echo "🛠️ Updating Desktop Entry for CircuitGuard..."
DESKTOP_FILE="$HOME/.local/share/applications/circuit-guard.desktop"

mkdir -p "$(dirname "$DESKTOP_FILE")"

cat << 'EOF2' > "$DESKTOP_FILE"
[Desktop Entry]
Version=1.0
Name=CircuitGuard
Comment=Industrial PCB Layout Viewer and AI Diagnostic Suite
Exec=/home/draxx/Projects/project_pcb_problem_finder/run.sh
Icon=/home/draxx/Projects/project_pcb_problem_finder/src/assets/icon.jpeg
Terminal=false
Type=Application
Categories=Development;Engineering;Electronics;Science;
Keywords=PCB;Altium;Electronics;Hardware;AI;Debugging;
StartupWMClass=CircuitGuard
StartupNotify=true
EOF2

chmod +x "$DESKTOP_FILE"
update-desktop-database "$HOME/.local/share/applications/"

echo "✅ CircuitGuard update completed successfully!"
