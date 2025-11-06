# ──────────────────────────────
# QyzylShell v1 Cross-Platform Makefile
# ──────────────────────────────

UNAME_S := $(shell uname -s)

# ──────────────────────────────
# Platform-specific settings
# ──────────────────────────────
ifeq ($(UNAME_S),Linux)
    CC       := gcc
    PREFIX   := /usr/local
    LDFLAGS  := -lreadline
    SED_INPLACE := sed -i
endif

ifeq ($(UNAME_S),Darwin)
    CC       := clang
    PREFIX   := /opt/homebrew
    # Prefer Homebrew’s readline
    LDFLAGS  := -L$(PREFIX)/opt/readline/lib -I$(PREFIX)/opt/readline/include -lreadline
    # macOS sed needs a backup suffix
    SED_INPLACE := sed -i ''
endif

# ──────────────────────────────
CFLAGS   := -Wall -Wextra -O2 -g
TARGET   := qyzylshell
SRC      := main.c
OBJ      := $(SRC:.c=.o)

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
	@echo "🧹 Cleaning /etc/shells entry..."
	sudo $(SED_INPLACE) "\|$(PREFIX)/bin/$(TARGET)|d" /etc/shells
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
	@echo "  make install   - Install to $(PREFIX)/bin"
	@echo "  make uninstall - Remove from system"
	@echo "  make clean     - Clean build files"
