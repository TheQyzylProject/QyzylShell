# QyzylShell v0.6 — Makefile
CC        = gcc
CFLAGS    = -Wall -Wextra -O2
LDFLAGS   = -lreadline
TARGET    = qshell
SRC       = src/main.c
PREFIX    = /usr/local
BINDIR    = $(PREFIX)/bin
INSTALL_PATH = $(BINDIR)/$(TARGET)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo "✅ Build complete: ./$(TARGET)"

install: $(TARGET)
	@echo "Installing QyzylShell to system directory..."
	mkdir -p $(BINDIR)
	cp $(TARGET) $(INSTALL_PATH)
	chmod 755 $(INSTALL_PATH)

	@echo "Ensuring /etc/shells contains $(INSTALL_PATH)..."
	@if ! grep -qx '$(INSTALL_PATH)' /etc/shells; then \
		echo '$(INSTALL_PATH)' | sudo tee -a /etc/shells >/dev/null; \
		echo "✅ Added to /etc/shells."; \
	else \
		echo "ℹ️  Already present in /etc/shells."; \
	fi

	@echo "Would you like to set QyzylShell as your default shell? [y/n]"
	@read ans; \
	if [ "$$ans" = "y" ] || [ "$$ans" = "Y" ]; then \
		chsh -s $(INSTALL_PATH); \
		echo "✅ QyzylShell is now your default shell."; \
	else \
		echo "ℹ️  Installed but not set as default."; \
	fi

	@echo "Installation complete! Run it with: qshell"

uninstall:
	@echo "Removing QyzylShell..."
	rm -f $(INSTALL_PATH)
	@sed -i "\|$(INSTALL_PATH)|d" /etc/shells
	@echo "✅ Uninstalled successfully."

clean:
	rm -f $(TARGET)
	@echo "Cleaned build files."
