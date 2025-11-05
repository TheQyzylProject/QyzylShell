# QyzylShell
**QyzylShell** is a tiny, aesthetically pleasing and functional unix shell for [QUNICS](https://github.com/LibreQyzyl/qunics) —<br>

<img width="474" height="292" alt="resim" src="https://github.com/user-attachments/assets/4f5f475f-43ae-4018-905f-b93bb0c20dad" />


# Installation
```bash
git clone https://github.com/TheQyzylProject/QyzylShell
cd QyzylShell
make install # as root / yönetici olarak çalıştırın
```
target location: `/usr/local/bin/qyzylshell`

On some Linux distributions, compiling programs that use the GNU Readline library may require the development package (for example, `readline-devel` on Void Linux). This package provides the necessary header files such as `readline/readline.h`. Without it, the compiler will report a missing file error even if the `readline` runtime library is installed.

# Requirements
- Unix-like or Unix-based operating system
- readline
- readline-devel (for some distributions)
