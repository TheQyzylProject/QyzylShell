# QyzylShell
**QyzylShell** is a tiny, aesthetically pleasing and functional unix shell for [QUNICS](https://github.com/LibreQyzyl/qunics) —<br>

<img width="933" height="703" alt="resim" src="https://github.com/user-attachments/assets/66397eb8-dc2a-4691-95fd-618374595684" />

## Supported Operating Systems
| OS    |  Stable Release Version |
|-------------- | --------------|
| GNU/Linux |  QyzylShell v0.6
| QUNICS |  QyzylShell 0.1
| MacOS      |  X
| BSD      |  X 

# Installation
## Arch Linux
add `qyzylrepo` in `/etc/pacman.conf`
```shell
[qyzylrepo]
SigLevel = Never
Server = https://mirrors.qyzyl.xyz/custom-repos-for-other-distros/archlinux/$arch
```
then;
```sh
sudo pacman -Sy
sudo pacman -S qyzylshell
```

## Manual
```sh
git clone https://github.com/TheQyzylProject/QyzylShell
cd QyzylShell
sudo make install
```
target location: `/usr/local/bin/qyzylshell`

On some Linux distributions, compiling programs that use the GNU Readline library may require the development package (for example, `readline-devel` on Void Linux). This package provides the necessary header files such as `readline/readline.h`. Without it, the compiler will report a missing file error even if the `readline` runtime library is installed.

# Requirements
- Unix-like or Unix-based operating system
- readline
- readline-devel (for some distributions)

# Uninstall
```sh
sudo make uninstall # on QyzylShell directory
```
If you deleted the QyzylShell folder, it doesn't matter. You can download it again and use the `sudo make uninstall` command.
