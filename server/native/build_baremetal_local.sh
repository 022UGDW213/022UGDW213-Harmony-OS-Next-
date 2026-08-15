# Configuration
# Support both Homebrew (x86_64-elf-gcc) and Docker (gcc) paths
# Explicitly check for /usr/local/bin/x86_64-elf-gcc which is standard for Brew
set -e
if [ -x "/usr/local/bin/x86_64-elf-gcc" ]; then
    CC="/usr/local/bin/x86_64-elf-gcc"
    LD="/usr/local/bin/x86_64-elf-ld"
elif command -v x86_64-elf-gcc >/dev/null 2>&1; then
    CC="x86_64-elf-gcc"
    LD="x86_64-elf-ld"
else
    CC="gcc"
    LD="ld"
fi

OUT="harmony_kernel"
BUILD="build"
CFLAGS="-ffreestanding -O2 -Wall -Wextra -m32 -I. -I./kernel -I./drivers -I./boot -fno-pie"
LDFLAGS="-m elf_i386 -T linker.ld -nostdlib"

# 1. Setup
echo "🚀 Building HarmonyOS Bare Metal (Toolchain: $CC)"
rm -rf $BUILD $OUT
mkdir -p $BUILD

# 2. Compile Bootloader
# Use -x assembler-with-cpp BEFORE the file
echo "🔨 Assembling boot.S..."
$CC -x assembler-with-cpp $CFLAGS -c boot/boot.S -o $BUILD/boot.o

# 2b. Compile 32-bit ISR stubs (plain gas, no preprocessor needed)
echo "🔨 Assembling kernel/isr.s..."
$CC $CFLAGS -c kernel/isr.s -o $BUILD/isr.o

# 3. Compile Bare Metal Kernel Sources
# ONLY compile the files we explicitly wrote for the bare-metal phase
SOURCES=(
    "kernel/kmain_bare.c"
    "boot/multiboot.c"
    "drivers/vga.c"
    "drivers/serial.c"
    "drivers/keyboard.c"
    "drivers/pic.c"
    "drivers/pit.c"
    "kernel/gdt.c"
    "kernel/idt.c"
    "kernel/kprintf.c"
    "kernel/meminfo.c"
    "kernel/alloc.c"
    "kernel/task.c"
    "kernel/switch.s"
    "kernel/ipc.c"
    "kernel/dev.c"
    "kernel/string.c"
    "kernel/shell.c"
    "kernel/hal/bare/hal.c"
)

for SRC in "${SOURCES[@]}"; do
    OBJ="$BUILD/$(basename ${SRC%.c}.o)"
    echo "🔨 Compiling $SRC..."
    $CC -c $SRC -o $OBJ $CFLAGS
done

# 4. Link
echo "🔗 Linking $OUT..."
$LD -n -o $OUT $LDFLAGS $BUILD/*.o

# 5. Verify
if [ -f "$OUT" ]; then
    echo "✅ Build Complete: $OUT"
    echo "   Geometry: $(file $OUT)"
    
    echo ""
    echo "👉 To run in QEMU:"
    echo "   qemu-system-x86_64 -kernel $OUT -serial stdio -m 512M"
else
    echo "❌ Build Failed"
    exit 1
fi
