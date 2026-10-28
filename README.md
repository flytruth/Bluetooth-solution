# 🎧 Linux Bluetooth Fixer & Troubleshooting Guide

A comprehensive toolkit, automated scripts, and deep-dive troubleshooting guide to fix persistent Bluetooth failures on Linux laptops.

Laptops running Linux frequently suffer from Bluetooth adapter drops after suspend/resume, corrupted firmware loading, rfkill soft-blocks, and audio daemon deadlocks. This repository provides both quick recovery tools and permanent system-level configurations to resolve these issues.

🚀 Quick Start (Automated Fix)

If your Bluetooth suddenly stops working or vanishes after waking your laptop from sleep, run the included bt-fix.sh script to perform an instant reset of the driver modules, service daemons, and user audio sockets.

Run directly:

chmod +x bt-fix.sh
sudo ./bt-fix.sh


What bt-fix.sh Does:

Unblocks rfkill: Clears stuck hardware and software radio blocks.

Restarts BlueZ: Reloads bluetooth.service.

Resets Kernel Modules: Forcefully unloads and reloads btusb and bluetooth kernel drivers to power-cycle the controller.

Syncs Audio Daemons: Restarts user-space audio servers (PipeWire / PulseAudio / WirePlumber) without requiring a full logout or reboot.

🛠️ Permanent Fixes for Deep System Issues

If your Bluetooth breaks every time your laptop goes to sleep, basic script restarts won't permanently solve the underlying hardware/kernel race conditions. Follow the solutions below based on your issue:

1. Disable USB Autosuspend for Bluetooth

Most laptop Bluetooth controllers operate internally over USB. Linux aggressive power management frequently cuts power to the USB bus during sleep, causing the controller to disappear on wake.

Solution:

Create a kernel module option file to prevent btusb from autosuspending:

echo "options btusb enable_autosuspend=n" | sudo tee /etc/modprobe.d/btusb.conf


After creating the file, update your initramfs and reboot:

# Ubuntu / Debian
sudo update-initramfs -u

# Arch Linux
sudo mkinitcpio -P

# Fedora
sudo dracut --force


2. Auto-Reset Bluetooth on System Resume (Systemd Hook)

To automatically execute a hardware driver power cycle every time your laptop wakes from sleep, install a systemd sleep hook.

Setup:

Create /etc/systemd/system/bluetooth-resume.service:

[Unit]
Description=Reset Bluetooth after system resume
After=suspend.target hybrid-sleep.target hibernate.target

[Service]
Type=oneshot
ExecStart=/usr/bin/modprobe -r btusb
ExecStartPost=/usr/bin/modprobe btusb

[Install]
WantedBy=suspend.target hybrid-sleep.target hibernate.target


Enable the service:

sudo systemctl daemon-reload
sudo systemctl enable bluetooth-resume.service


3. Diagnose Firmware Drops & Kernel Errors

If your Wi-Fi/Bluetooth combo card (Intel, Realtek, Broadcom) fails to reload firmware blobs after suspend, check your kernel logs:

journalctl -k -b 0 | grep -iE "blue|firmware|btusb"


Common Error Patterns:

Direct firmware load failed: Missing proprietary firmware binary. Ensure your distro's firmware package is installed (sudo apt install linux-firmware or distro equivalent).

Initialization timed out: Hardware power-state mismatch. Applying the USB Autosuspend fix above usually resolves this.

📋 System Requirements

OS: Any Linux distribution running kernel 5.x or 6.x

Init System: Systemd

Audio Server: PipeWire, WirePlumber, or PulseAudio

Privileges: Root / Sudo access required for module and service management

🤝 Contributing

Forks, PRs, and issue reports for specific laptop models or Bluetooth chipsets are welcome!

📜 License

MIT License. Free to use, modify, and distribute.
