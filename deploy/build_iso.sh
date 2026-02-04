#!/bin/bash
# deploy/build_iso.sh - Full bare-metal build pipeline

set -e

# Configuration
KERNEL_NAME="harmony.kernel"
ISO_DIR="isodir"
BOOT_DIR="$ISO_DIR/boot"
GRUB_DIR="$BOOT_DIR/grub"
OUTPUT_ISO="harmony.iso"
SRC_DIR="server/native"

echo "💿 Starting HarmonyOS ISO Build..."

# 1. Clean previous builds
echo "   Cleaning..."
rm -rf $ISO_DIR $OUTPUT_ISO
make -C $SRC_DIR clean_bare

# 2. Build kernel (using Docker-compatible flags if running locally)
# Check for cross-compiler
if command -v i686-elf-gcc &> /dev/null; then
    COMPILER="i686-elf-gcc"
else
    # Fallback to gcc -m32 for local macOS/Linux testing (Hosted Simulation Env)
    # NOTE: This might fail on macOS due to Mach-O. 
    # This script assumes it's running IN DOCKER or on a Linux box with multilib.
    COMPILER="gcc"
fi

echo "   Building Kernel ELF..."
make -C $SRC_DIR harmony_bare.elf

# 3. Create ISO structure
echo "   Creating ISO structure..."
mkdir -p $GRUB_DIR

# 4. Copy kernel
cp $SRC_DIR/harmony_bare.elf $BOOT_DIR/$KERNEL_NAME

# 5. Create GRUB config
cat > $GRUB_DIR/grub.cfg << EOF
set timeout=0
set default=0
menuentry "HarmonyOS Next" {
    multiboot2 /boot/$KERNEL_NAME
    boot
}
EOF

# 6. Build ISO
echo "   Generating ISO image..."
if command -v grub-mkrescue &> /dev/null; then
    grub-mkrescue -o $OUTPUT_ISO $ISO_DIR
    echo "✅ ISO created: $OUTPUT_ISO"
    echo "   To test (if QEMU installed):"
    echo "   qemu-system-i386 -cdrom $OUTPUT_ISO -serial stdio"
else
    echo "⚠️  grub-mkrescue not found. ISO generation skipped."
    echo "   Kernel ELF is ready at: $SRC_DIR/harmony_bare.elf"
fi
