# ──────────────────────────────
# QyzylShell v1 Makefile
# ──────────────────────────────

CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -g
LDFLAGS  := -lreadline
TARGET   := qyzylshell
SRC      := main.c
OBJ      := $(SRC:.c=.o)
PREFIX   := /usr/local

# ──────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJ)
	@echo "🔧 Linking $@..."
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "✅ Build complete: ./$@"

%.o: %.c
	@echo "🧠 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# ──────────────────────────────
install: all
	@echo "📦 Installing $(TARGET) to $(PREFIX)/bin..."
	sudo cp $(TARGET) $(PREFIX)/bin/
	@echo "🔍 Checking /etc/shells entry..."
	@if ! grep -qx "$(PREFIX)/bin/$(TARGET)" /etc/shells; then \
		echo "$(PREFIX)/bin/$(TARGET)" | sudo tee -a /etc/shells >/dev/null; \
		echo "✅ Added $(PREFIX)/bin/$(TARGET) to /etc/shells"; \
	else \
		echo "ℹ️  Already present in /etc/shells"; \
	fi
	@echo "✅ Installed successfully."

uninstall:
	@echo "❌ Removing binary..."
	sudo rm -f $(PREFIX)/bin/$(TARGET)
	@echo "🗑️  Removed $(PREFIX)/bin/$(TARGET)."
	@echo "🧹 Cleaning /etc/shells entry..."
	sudo sed -i "\|$(PREFIX)/bin/$(TARGET)|d" /etc/shells
	@echo "✅ Uninstalled cleanly."

# ──────────────────────────────
run: all
	@./$(TARGET)

clean:
	@rm -f $(OBJ) $(TARGET)
	@echo "✨ Clean complete."

help:
	@echo "QyzylShell Makefile usage:"
	@echo "  make           - Compile"
	@echo "  make run       - Compile and run"
	@echo "  make install   - İnstall and adding to /etc/shells"
	@echo "  make uninstall - Remove"
	@echo "  make clean     - Clean"
