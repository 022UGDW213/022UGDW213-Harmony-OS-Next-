# Updated Makefile with HAL support
# Detect compilers
HOST_CC = cc
CROSS_CC = /usr/local/bin/x86_64-elf-gcc
CROSS_LD = /usr/local/bin/x86_64-elf-ld
AS = /usr/local/bin/nasm

# Flags
COMMON_CFLAGS = -O2 -Wall -Wextra -I. -I./kernel -I./kernel/include

# Build configuration
ifeq ($(BUILD),bare)
    CC = $(CROSS_CC)
    LD = $(CROSS_LD)
    # Force 32-bit for Multiboot compatibility
    CFLAGS = -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL
    LDFLAGS = -m elf_i386
    TARGET = harmony_bare
    HAL_SRC = kernel/hal/bare/hal.c
    DRIVERS = drivers/vga.o drivers/serial.o drivers/keyboard.o
    # Bare metal needs bootloader
    BOOT_OBJ = build/boot/boot.o
else
    # Hosted Mode (macOS/Linux)
    CC = $(HOST_CC)
    LD = $(HOST_CC) # Use CC for linking hosted
    # No -nostdlib for hosted, we WANT libc
    CFLAGS = $(COMMON_CFLAGS) -DHARMONY_HOSTED
    LDFLAGS = 
    TARGET = harmony_hosted
    HAL_SRC = kernel/hal/hosted/hal.c
    DRIVERS_OBJ =
    BOOT_OBJ =
endif

# Sources
KERNEL_COMMON_SRC = kernel/main.c \
                    kernel/hal/common/hal_common.c \
                    kernel/hal/common/hal_stdio.c \
                    kernel/log.c \
                    kernel/memory.c \
                    kernel/scheduler.c

DRIVERS_SRC = drivers/vga.c drivers/serial.c drivers/keyboard.c

KERNEL_SRC = $(KERNEL_COMMON_SRC) $(HAL_SRC)

KERNEL_OBJ = $(patsubst %.c, build/%.o, $(KERNEL_SRC)) \
             $(BOOT_OBJ) \
             $(DRIVERS_OBJ)

# Object mapping
OBJ_HOSTED = build/hosted/main.o build/hosted/hal_common.o build/hosted/hal_stdio.o build/hosted/log.o build/hosted/memory.o build/hosted/scheduler.o build/hosted/hal.o
OBJ_BARE = build/bare/main.o build/bare/hal_common.o build/bare/hal_stdio.o build/bare/log.o build/bare/memory.o build/bare/scheduler.o build/bare/hal.o build/bare/drivers/vga.o build/bare/drivers/serial.o build/bare/drivers/keyboard.o build/boot/boot.o

all: harmony_hosted harmony_bare

# --- HOSTED BUILD ---
harmony_hosted: $(OBJ_HOSTED)
	@echo "🔗 Linking Hosted Kernel..."
	$(HOST_CC) -o $@ $^

build/hosted/main.o: kernel/main.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

build/hosted/log.o: kernel/log.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

build/hosted/memory.o: kernel/memory.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

build/hosted/scheduler.o: kernel/scheduler.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

build/hosted/hal_common.o: kernel/hal/common/hal_common.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

build/hosted/hal_stdio.o: kernel/hal/common/hal_stdio.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

build/hosted/hal.o: kernel/hal/hosted/hal.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(COMMON_CFLAGS) -DHARMONY_HOSTED -c $< -o $@

# --- BARE METAL BUILD ---
harmony_bare: $(OBJ_BARE) linker.ld
	@echo "🔗 Linking Bare Metal Kernel..."
	$(CROSS_LD) $(LDFLAGS) -n -o $@ -T linker.ld $(OBJ_BARE)

build/bare/main.o: kernel/main.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/log.o: kernel/log.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/memory.o: kernel/memory.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/scheduler.o: kernel/scheduler.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/hal_stdio.o: kernel/hal/common/hal_stdio.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/hal_common.o: kernel/hal/common/hal_common.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/hal.o: kernel/hal/bare/hal.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/bare/drivers/%.o: drivers/%.c
	@mkdir -p $(dir $@)
	$(CROSS_CC) -ffreestanding -m32 -fno-pie -fno-stack-protector -mno-red-zone -nostdlib -fno-builtin $(COMMON_CFLAGS) -DHARMONY_BARE_METAL -c $< -o $@

build/boot/boot.o: boot/boot.S
	@mkdir -p $(dir $@)
	$(CROSS_CC) -m32 -c $< -o $@ $(CFLAGS)

# --- UTILS ---
clean:
	rm -rf build harmony_hosted harmony_bare harmony.iso iso

run-hosted: harmony_hosted
	./harmony_hosted

run-bare: harmony_bare
	/usr/local/bin/qemu-system-x86_64 -kernel harmony_bare -serial stdio -m 512M

iso: harmony_bare
	@mkdir -p iso/boot/grub
	cp harmony_bare iso/boot/harmony_kernel
	@echo 'menuentry "HarmonyOS" {' > iso/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/harmony_kernel' >> iso/boot/grub/grub.cfg
	@echo '    boot' >> iso/boot/grub/grub.cfg
	@echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o harmony.iso iso/

.PHONY: all clean run-hosted run-bare iso
