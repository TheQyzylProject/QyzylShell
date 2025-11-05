# ──────────────────────────────
# QyzylShell v1
# ──────────────────────────────

CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -g
LDFLAGS  := -lreadline
TARGET   := qyzylshell
SRC      := main.c
OBJ      := $(SRC:.c=.o)

# ──────────────────────────────
# Varsayılan hedef
all: $(TARGET)

# Derleme kuralları
$(TARGET): $(OBJ)
	@echo "🔧 Linking $@..."
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "✅ Build complete: ./$@"

%.o: %.c
	@echo "🧠 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# ──────────────────────────────
# Ekstra görevler
run: all
	@echo "🚀 Running QyzylShell..."
	@./$(TARGET)

clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(OBJ) $(TARGET)
	@echo "✨ Clean complete."

install: all
	@echo "📦 Installing to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "✅ Installed: $(TARGET) → /usr/local/bin/"

uninstall:
	@echo "❌ Removing installed binary..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "🗑️  Uninstalled $(TARGET)."

# ──────────────────────────────
# Yardım mesajı
help:
	@echo "QyzylShell Makefile usage:"
	@echo "  make           - Derle"
	@echo "  make run       - Derle ve çalıştır"
	@echo "  make clean     - Temizle"
	@echo "  make install   - Sisteme kur"
	@echo "  make uninstall - Kurulumu kaldır"
