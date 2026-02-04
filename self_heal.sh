#!/bin/bash
# self_heal.sh - Repository Health & Recovery Automation
# Usage: ./self_heal.sh

ECHO_GREEN='\033[0;32m'
ECHO_YELLOW='\033[0;33m'
ECHO_RED='\033[0;31m'
ECHO_NC='\033[0m'

echo -e "${ECHO_GREEN}🚑 HarmonyOS Self-Healing Protocol Initiated...${ECHO_NC}"

# Section 1: Codebase Hygiene
echo -e "\n${ECHO_YELLOW}[1/3] Checking Codebase Hygiene...${ECHO_NC}"
if [ -f "./scan.sh" ]; then
    ./scan.sh
    if [ $? -ne 0 ]; then
        echo -e "${ECHO_RED}⚠️  Violations found. Please review scan.sh output above.${ECHO_NC}"
        # Future: Implement auto-fixer for known trivials (e.g. removing printf in comments)
    fi
else
    echo "⚠️  scan.sh not found. Skipping."
fi

# Section 2: Environment Reset
echo -e "\n${ECHO_YELLOW}[2/3] Resetting Build Environment...${ECHO_NC}"
if [ -d "server/native" ]; then
    cd server/native
    
    # Capture output to log unless error
    make clean > /dev/null
    rm -rf build/ harmony_hosted harmony_bare harmony.iso iso
    echo "✅ Build artifacts purged."
else
    echo -e "${ECHO_RED}❌ Error: server/native directory missing!${ECHO_NC}"
    exit 1
fi

# Section 3: Convergence Verification & Recovery
echo -e "\n${ECHO_YELLOW}[3/3] Verifying Core Systems...${ECHO_NC}"
./test_convergence.sh > healing.log 2>&1

if [ $? -eq 0 ]; then
    echo -e "${ECHO_GREEN}✅ System Healthy! Convergence verified.${ECHO_NC}"
    rm healing.log
    exit 0
else
    echo -e "${ECHO_RED}❌ Verification Failed!${ECHO_NC}"
    echo -e "${ECHO_YELLOW}🩹 Attempting Deep Recovery (Aggressive Clean)...${ECHO_NC}"
    
    # Recovery Strategy 1: Aggressive Rebuild
    make clean
    rm -rf build/
    
    # Re-run test
    echo "Retrying verification..."
    ./test_convergence.sh >> healing.log 2>&1
    
    if [ $? -eq 0 ]; then
        echo -e "${ECHO_GREEN}✅ RECOVERY SUCCESSFUL! System is back online.${ECHO_NC}"
        echo "Report: Verification passed after deep clean."
        exit 0
    else
        echo -e "${ECHO_RED}💀 Recovery Failed.${ECHO_NC}"
        echo "Critical errors persists. See 'server/native/healing.log' for details."
        tail -n 10 healing.log
        exit 1
    fi
fi
