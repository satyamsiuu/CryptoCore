#!/bin/bash

# ANSI color codes
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Get the PID of the CryptoCore GUI
pid=$(pgrep cryptocore_gui)

if [ -z "$pid" ]; then
    echo -e "${RED}Error: CryptoCore GUI is not running!${NC}"
    echo "Please start the application first."
    exit 1
fi

clear
echo -e "${BLUE}=== CryptoCore Process Analysis ===${NC}"
echo -e "${GREEN}GUI Application Process ID: $pid${NC}\n"

echo -e "${YELLOW}1. Process Hierarchy:${NC}"
echo -e "${CYAN}Main and Child Processes:${NC}"
ps -o pid,ppid,command -p $pid $(ps -o pid= -p $pid) | sed 's/^/  /'
echo

echo -e "${YELLOW}2. Worker Processes (Process Mode Analysis):${NC}"
echo -e "${CYAN}Active CryptoCore Processes:${NC}"
ps aux | grep cryptocore | grep -v grep | grep -v monitor.sh | \
    awk '{printf "  PID: %-6s  PPID: %-6s  CPU: %5s%%  MEM: %5.1f MB  CMD: %s\n", $2, $3, $3, $6/1024, $11}' | sed 's/^/  /'
echo

echo -e "${YELLOW}3. Thread Analysis (Thread Mode):${NC}"
echo -e "${CYAN}Thread Status for Process $pid:${NC}"
echo "  Format: PID STATE %CPU TIME COMMAND"
ps -M -p $pid | head -n 1 | sed 's/^/  /'
ps -M -p $pid | tail -n +2 | sort -rn -k4 | sed 's/^/  /'
echo

echo -e "${YELLOW}4. CPU & Memory Usage:${NC}"
top -pid $pid -l 1 -stats pid,cpu,mem,command -n 1 | tail -n +7 | head -n 1 | \
    awk '{printf "  PID: %-6s  CPU: %5s%%  Memory: %5s%%  Command: %s\n", $1, $2, $3, $4}' 
echo

echo -e "${BLUE}=== How to Interpret the Results ===${NC}"
echo -e "1. ${CYAN}For Process Mode:${NC}"
echo "   - Section 1 shows parent-child process relationships"
echo "   - Section 2 shows separate worker processes with their own PIDs"
echo "   - Look for multiple processes with cryptocore_gui as parent"
echo
echo -e "2. ${CYAN}For Thread Mode:${NC}"
echo "   - Section 3 shows all threads within the main process"
echo "   - High CPU% threads (near 100%) are active worker threads"
echo "   - Thread count increases during encryption/decryption"
echo
echo -e "${YELLOW}How to Demonstrate Parallel Processing:${NC}"
echo "1. Run this script BEFORE starting encryption/decryption"
echo "2. Start encrypting/decrypting a large file"
echo "3. Run this script AGAIN to see:"
echo "   - Process Mode: New child processes appear"
echo "   - Thread Mode: New active threads with high CPU usage"