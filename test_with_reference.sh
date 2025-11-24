#!/bin/bash
# Test script for CTP sender with reference receiver
# Run this on a Linux machine (lab computer or VM)

set -e  # Exit on error

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "=========================================="
echo "CTP Protocol Testing with Reference Implementation"
echo "Exercise 1 - Lab N6"
echo "=========================================="
echo ""

# Check if we're on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo -e "${YELLOW}⚠️  Warning: This script is designed for Linux${NC}"
    echo "The reference binaries are ELF executables and require Linux"
    echo "Press Ctrl+C to cancel, or Enter to continue anyway..."
    read
fi

# Step 1: Compile your sender
echo -e "${BLUE}Step 1: Compiling your CTP sender...${NC}"
if g++ -std=c++17 -Wall -O2 -I. -o ctp_sender src/sender.cc -lz 2>&1; then
    echo -e "${GREEN}✓ Sender compiled${NC}"
else
    echo -e "${RED}✗ Compilation failed${NC}"
    exit 1
fi
echo ""

# Step 2: Check reference receiver
echo -e "${BLUE}Step 2: Checking reference receiver...${NC}"
if [ ! -f "reference-implementation/receiver" ]; then
    echo -e "${RED}✗ Reference receiver not found${NC}"
    echo "Expected: reference-implementation/receiver"
    exit 1
fi

if [ ! -x "reference-implementation/receiver" ]; then
    chmod +x reference-implementation/receiver
fi

echo -e "${GREEN}✓ Reference receiver found${NC}"
echo ""

# Step 3: Create test files if they don't exist
echo -e "${BLUE}Step 3: Preparing test files...${NC}"
mkdir -p test_files
if [ ! -f "test_files/test_1byte.bin" ]; then
    echo "X" | tr -d '\n' > test_files/test_1byte.bin
fi
if [ ! -f "test_files/test_256bytes.bin" ]; then
    dd if=/dev/urandom of=test_files/test_256bytes.bin bs=1 count=256 2>/dev/null
fi
if [ ! -f "test_files/test_512bytes.bin" ]; then
    dd if=/dev/urandom of=test_files/test_512bytes.bin bs=1 count=512 2>/dev/null
fi

echo "Test files:"
ls -lh test_files/*.bin
echo -e "${GREEN}✓ Test files ready${NC}"
echo ""

# Create results directory
mkdir -p test_results

# ===================================================================
# TEST 1: IPv4 - 1 byte file
# ===================================================================
echo "=========================================="
echo -e "${BLUE}TEST 1: IPv4 - 1 byte file${NC}"
echo "=========================================="

echo "Starting reference receiver on port 5001..."
./reference-implementation/receiver -f test_results/ipv4_1byte_output.bin 127.0.0.1 5001 > test_results/ipv4_1byte_receiver.log 2>&1 &
RECV_PID=$!
sleep 1

echo "Sending 1-byte file with your sender..."
if ./ctp_sender -f test_files/test_1byte.bin 127.0.0.1 5001 > test_results/ipv4_1byte_sender.log 2>&1; then
    sleep 1
    kill $RECV_PID 2>/dev/null || true
    wait $RECV_PID 2>/dev/null || true

    if diff test_files/test_1byte.bin test_results/ipv4_1byte_output.bin > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASS: Files match!${NC}"
    else
        echo -e "${RED}❌ FAIL: Files don't match${NC}"
        echo "Input:  $(wc -c < test_files/test_1byte.bin) bytes"
        echo "Output: $(wc -c < test_results/ipv4_1byte_output.bin) bytes"
    fi
else
    echo -e "${RED}❌ FAIL: Sender error${NC}"
    kill $RECV_PID 2>/dev/null || true
fi
echo ""

# ===================================================================
# TEST 2: IPv4 - 256 byte file
# ===================================================================
echo "=========================================="
echo -e "${BLUE}TEST 2: IPv4 - 256 byte file${NC}"
echo "=========================================="

echo "Starting reference receiver on port 5002..."
./reference-implementation/receiver -f test_results/ipv4_256byte_output.bin 127.0.0.1 5002 > test_results/ipv4_256byte_receiver.log 2>&1 &
RECV_PID=$!
sleep 1

echo "Sending 256-byte file with your sender..."
if ./ctp_sender -f test_files/test_256bytes.bin 127.0.0.1 5002 > test_results/ipv4_256byte_sender.log 2>&1; then
    sleep 1
    kill $RECV_PID 2>/dev/null || true
    wait $RECV_PID 2>/dev/null || true

    if diff test_files/test_256bytes.bin test_results/ipv4_256byte_output.bin > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASS: Files match!${NC}"
    else
        echo -e "${RED}❌ FAIL: Files don't match${NC}"
        echo "Input:  $(wc -c < test_files/test_256bytes.bin) bytes"
        echo "Output: $(wc -c < test_results/ipv4_256byte_output.bin) bytes"
    fi
else
    echo -e "${RED}❌ FAIL: Sender error${NC}"
    kill $RECV_PID 2>/dev/null || true
fi
echo ""

# ===================================================================
# TEST 3: IPv4 - 512 byte file (MAXIMUM SIZE)
# ===================================================================
echo "=========================================="
echo -e "${BLUE}TEST 3: IPv4 - 512 byte file (MAX)${NC}"
echo "=========================================="

echo "Starting reference receiver on port 5003..."
./reference-implementation/receiver -f test_results/ipv4_512byte_output.bin 127.0.0.1 5003 > test_results/ipv4_512byte_receiver.log 2>&1 &
RECV_PID=$!
sleep 1

echo "Sending 512-byte file with your sender..."
if ./ctp_sender -f test_files/test_512bytes.bin 127.0.0.1 5003 > test_results/ipv4_512byte_sender.log 2>&1; then
    sleep 1
    kill $RECV_PID 2>/dev/null || true
    wait $RECV_PID 2>/dev/null || true

    if diff test_files/test_512bytes.bin test_results/ipv4_512byte_output.bin > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASS: Files match!${NC}"
    else
        echo -e "${RED}❌ FAIL: Files don't match${NC}"
        echo "Input:  $(wc -c < test_files/test_512bytes.bin) bytes"
        echo "Output: $(wc -c < test_results/ipv4_512byte_output.bin) bytes"
    fi
else
    echo -e "${RED}❌ FAIL: Sender error${NC}"
    kill $RECV_PID 2>/dev/null || true
fi
echo ""

# ===================================================================
# TEST 4: IPv6 - 1 byte file
# ===================================================================
echo "=========================================="
echo -e "${BLUE}TEST 4: IPv6 - 1 byte file${NC}"
echo "=========================================="

echo "Starting reference receiver on port 5004 (IPv6)..."
./reference-implementation/receiver -f test_results/ipv6_1byte_output.bin ::1 5004 > test_results/ipv6_1byte_receiver.log 2>&1 &
RECV_PID=$!
sleep 1

echo "Sending 1-byte file with your sender (IPv6)..."
if ./ctp_sender -f test_files/test_1byte.bin ::1 5004 > test_results/ipv6_1byte_sender.log 2>&1; then
    sleep 1
    kill $RECV_PID 2>/dev/null || true
    wait $RECV_PID 2>/dev/null || true

    if diff test_files/test_1byte.bin test_results/ipv6_1byte_output.bin > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASS: Files match!${NC}"
    else
        echo -e "${RED}❌ FAIL: Files don't match${NC}"
    fi
else
    echo -e "${RED}❌ FAIL: Sender error${NC}"
    kill $RECV_PID 2>/dev/null || true
fi
echo ""

# ===================================================================
# TEST 5: IPv6 - 512 byte file
# ===================================================================
echo "=========================================="
echo -e "${BLUE}TEST 5: IPv6 - 512 byte file${NC}"
echo "=========================================="

echo "Starting reference receiver on port 5005 (IPv6)..."
./reference-implementation/receiver -f test_results/ipv6_512byte_output.bin ::1 5005 > test_results/ipv6_512byte_receiver.log 2>&1 &
RECV_PID=$!
sleep 1

echo "Sending 512-byte file with your sender (IPv6)..."
if ./ctp_sender -f test_files/test_512bytes.bin ::1 5005 > test_results/ipv6_512byte_sender.log 2>&1; then
    sleep 1
    kill $RECV_PID 2>/dev/null || true
    wait $RECV_PID 2>/dev/null || true

    if diff test_files/test_512bytes.bin test_results/ipv6_512byte_output.bin > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASS: Files match!${NC}"
    else
        echo -e "${RED}❌ FAIL: Files don't match${NC}"
    fi
else
    echo -e "${RED}❌ FAIL: Sender error${NC}"
    kill $RECV_PID 2>/dev/null || true
fi
echo ""

# ===================================================================
# SUMMARY
# ===================================================================
echo "=========================================="
echo "TEST SUMMARY"
echo "=========================================="
echo ""
echo "Results saved in test_results/"
echo "Check logs for details:"
echo "  - test_results/*.log"
echo ""
echo "Grading criteria:"
echo "  [6 pts] IPv4 tests (1, 256, 512 bytes)"
echo "  [6 pts] IPv6 tests (1, 512 bytes)"
echo "  [4 pts] Command line arguments (-f host port)"
echo "  [4 pts] Code organization"
echo ""
echo "All tests complete!"
