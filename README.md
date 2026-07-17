# 🌌 MyDesktop

A sleek, minimalist, and highly optimized Hyprland configuration.

> [!TIP]
> This build is highly recommended for Arch-based distributions (Arch Linux, CachyOS, Archcraft, EndeavourOS, etc.).

---

## 🛠️ Components & Features

This Hyprland environment is built using the following core software:

* **Window Manager:** [Hyprland](https://hyprland.org/) *(Dynamic tiling Wayland compositor)*
* **Status Bar:** [Waybar](https://github.com/Alexays/Waybar) *(Highly customizable Wayland bar)*
* **App Launcher:** Rofi (Wayland fork) / Wofi
* **Terminal Emulator:** [Kitty](https://sw.kovidgoyal.net/kitty/) *(Fast, feature-rich, GPU-based terminal)*
* **Wallpaper Manager:** `awww`
* **Screen Locker:** [Hyprlock](https://wiki.hyprland.org/Hypr-ecosystem/hyprlock/) *(Fast, secure screen locker)*

---

## 🚀 Installation

### How installand?

### Option 1: Automated Installation (Recommended)
This script will automatically install `yay` (AUR helper) and all the required dependencies
```bash
sudo pacman -S --needed git base-devel # Yay
git clone https://aur.archlinux.org/yay.git
cd yay
makepkg -si

cd ~
yay -Syu
yay -S hyprland waybar hyprlauncher kitty awww bluema pipes.sh lavat peaclock cmatrix cava fastfetch asciiquarium hyprlock pavucontrol
```
### Option 2: Manual Installation
```bash
#Pacman Execution
sudo pacman -Syu # Update | Recomend : yay -Syu
sudo pacman -S hyprland waybar hyprlauncher kitty awww

#yay Execution
yay -Syu # Update
yay -S hyprland waybar hyprlauncher kitty awww  # Basic / Hypr + waybar + awww

# Tools
sudo pacman -S cmatrix cava fastfetch asciiquarium hyprlock pavucontrol bluema
yay -S pipes.sh lavat peaclock

git clone https://github.com/Dimitrof04/Dimitrof04Desktop.git
cd Dimitrof04Desktop
```
# thanks for use :3