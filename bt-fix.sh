#!/usr/bin/env bash
# Bluetooth Doctor: Automated fix for common Linux Bluetooth issues
# Requires root privileges for service and module operations.

set -euo pipefail

# Ensure running as root
if [[ $EUID -ne 0 ]]; then
   echo "[-] Please run this script with sudo." 
   exit 1
fi

echo "[*] Starting Bluetooth diagnostic and recovery..."

# Check and unblock rfkill
echo "[*] Checking rfkill status..."
if rfkill list bluetooth | grep -q "Soft blocked: yes"; then
    echo "[+] Bluetooth is soft-blocked. Unblocking..."
    rfkill unblock bluetooth
else
    echo "[✓] No soft-blocks detected."
fi

# Restart and enable BlueZ service
echo "[*] Restarting bluetooth service..."
systemctl restart bluetooth
systemctl enable bluetooth

# Reload kernel modules
echo "[*] Reloading kernel modules (btusb, bluetooth)..."
modprobe -r btusb || echo "[-] Could not unload btusb (in use?)"
modprobe -r bluetooth || echo "[-] Could not unload bluetooth"
modprobe bluetooth
modprobe btusb

# Find the logged-in graphical user to restart user-space audio services
TARGET_USER=$(loginctl list-users --no-legend | awk '{print $2}' | head -n 1)
if [[ -n "${TARGET_USER}" ]]; then
    TARGET_UID=$(id -u "${TARGET_USER}")
    echo "[*] Restarting audio daemons for user ${TARGET_USER} (UID ${TARGET_UID})..."
    
    su - "${TARGET_USER}" -c "export XDG_RUNTIME_DIR=/run/user/${TARGET_UID}; \
        systemctl --user restart pipewire wireplumber pipewire-pulse 2>/dev/null || \
        pulseaudio -k 2>/dev/null || true"
fi

echo "[✓] Bluetooth recovery complete!"
systemctl status bluetooth --no-pager
