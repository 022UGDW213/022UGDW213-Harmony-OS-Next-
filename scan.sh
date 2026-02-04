#!/bin/bash
# scan.sh - Verify Kernel Convergence (HAL Compliance)

ECHO_GREEN='\033[0;32m'
ECHO_RED='\033[0;31m'
ECHO_NC='\033[0m'

TARGET_DIR="server/native/kernel"
FORBIDDEN=("printf" "malloc" "free" "stdio.h" "stdlib.h" "string.h")
IGNORE_FILES=("hal_stdio.c" "hal_common.c" "hal.h") # Files allowed to define/implement these or use them carefully

echo "🔍 Scanning '$TARGET_DIR' for HAL violations..."
ERRORS=0

for TERM in "${FORBIDDEN[@]}"; do
    # Grep for term with word boundary if possible, or refined matching
    # We use -w for function names to avoid matching "free_bytes" when looking for "free"
    # But files might use "free(" so -w works if ( is a non-word char (it is).
    
    if [ "$TERM" == "free" ] || [ "$TERM" == "malloc" ] || [ "$TERM" == "printf" ]; then
         MATCHES=$(grep -r -w "$TERM" $TARGET_DIR | grep -v "Binary file" | grep -v "\.original")
    else
         MATCHES=$(grep -r "$TERM" $TARGET_DIR | grep -v "Binary file" | grep -v "\.original")
    fi
    
    # Filter out ignored files and pending modules (Phase 14)
    for IGNORE in "${IGNORE_FILES[@]}" "syscall.c" "interrupts.c"; do
        MATCHES=$(echo "$MATCHES" | grep -v "$IGNORE")
    done
    
    MATCHES=$(echo "$MATCHES" | grep -v "//.*include")
    
    if [ "$TERM" == "malloc" ]; then
        MATCHES=$(echo "$MATCHES" | grep -v "kmalloc" | grep -v "HAL_MALLOC")
    fi
     if [ "$TERM" == "free" ]; then
        MATCHES=$(echo "$MATCHES" | grep -v "kfree" | grep -v "HAL_FREE" | grep -v "_free")
    fi
     if [ "$TERM" == "printf" ]; then
        MATCHES=$(echo "$MATCHES" | grep -v "hal_printf" | grep -v "HAL_PRINTF" | grep -v "snprintf" | grep -v "vprintf")
    fi
    
    if [ ! -z "$MATCHES" ]; then
        echo -e "${ECHO_RED}❌ Found forbidden term '$TERM':${ECHO_NC}"
        echo "$MATCHES"
        ERRORS=$((ERRORS + 1))
    fi
done

echo ""
if [ $ERRORS -eq 0 ]; then
    echo -e "${ECHO_GREEN}✅ Clean! No HAL violations found in core kernel.${ECHO_NC}"
    exit 0
else
    echo -e "${ECHO_RED}⚠️  Found $ERRORS violations (see above).${ECHO_NC}"
    exit 1
fi
