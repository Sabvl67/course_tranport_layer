#!/bin/bash
# Quick start script for Lab N6 Exercise 1 testing
# Run this on the lab machine after copying your code

set -e  # Exit on error

echo "=========================================="
echo "CTP Protocol - Lab N6 Exercise 1"
echo "Quick Start Testing Script"
echo "=========================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Check we're in the right directory
echo "Step 1: Checking directory structure..."
if [ ! -d "src" ] || [ ! -d "include" ]; then
    echo -e "${RED}ERROR: src/ or include/ directory not found${NC}"
    echo "Please run this script from your project root directory"
    exit 1
fi
echo -e "${GREEN}✓ Directory structure OK${NC}"
echo ""

# Step 2: Compile
echo "Step 2: Compiling your code..."
echo "  Compiling sender..."
if g++ -std=c++17 -Wall -O2 -I. -o ctp_sender src/sender.cc -lz 2>&1; then
    echo -e "${GREEN}✓ Sender compiled${NC}"
else
    echo -e "${RED}✗ Sender compilation failed${NC}"
    exit 1
fi

echo "  Compiling receiver..."
if g++ -std=c++17 -Wall -O2 -I. -o ctp_receiver src/ctp_receiver.cc -lz 2>&1; then
    echo -e "${GREEN}✓ Receiver compiled${NC}"
else
    echo -e "${RED}✗ Receiver compilation failed${NC}"
    exit 1
fi
echo ""

# Step 3: Check for reference implementation
echo "Step 3: Looking for reference implementation..."
REF_DIR="/home/lib3780/project/reference-implementation"
if [ -d "$REF_DIR" ]; then
    echo -e "${GREEN}✓ Found reference directory: $REF_DIR${NC}"
    echo "  Files available:"
    ls -1 "$REF_DIR" | head -10

    # Try to copy reference files
    mkdir -p reference_binaries
    echo "  Copying reference files..."
    cp "$REF_DIR"/* reference_binaries/ 2>/dev/null || echo "  (Some files may not be accessible)"
else
    echo -e "${YELLOW}⚠ Reference directory not found at $REF_DIR${NC}"
    echo "  You'll need to locate it manually or test without reference"
fi
echo ""

# Step 4: Create test files
echo "Step 4: Creating test files..."
echo "Small CTP test file" > test_small.txt
dd if=/dev/urandom of=test_256.bin bs=1 count=256 2>/dev/null
dd if=/dev/urandom of=test_512.bin bs=1 count=512 2>/dev/null
echo -e "${GREEN}✓ Test files created:${NC}"
ls -lh test_*.txt test_*.bin
echo ""

# Step 5: Quick self-test
echo "Step 5: Running quick self-test (your sender -> your receiver)..."
echo "  Starting receiver on port 9001..."
./ctp_receiver -4 9001 selftest_output.txt > receiver.log 2>&1 &
RECV_PID=$!
sleep 1

echo "  Sending test file..."
if timeout 5 ./ctp_sender -f test_small.txt localhost 9001 > sender.log 2>&1; then
    sleep 1
    if diff test_small.txt selftest_output.txt > /dev/null 2>&1; then
        echo -e "${GREEN}✓✓✓ SUCCESS! Your sender and receiver work together!${NC}"
    else
        echo -e "${YELLOW}⚠ Transfer completed but files don't match${NC}"
        echo "  Check receiver.log and sender.log for details"
    fi
else
    echo -e "${YELLOW}⚠ Transfer timed out or failed${NC}"
    echo "  This is expected if there's a communication issue"
    echo "  Check receiver.log and sender.log for details"
fi

# Cleanup
kill $RECV_PID 2>/dev/null || true
echo ""

# Step 6: Next steps
echo "=========================================="
echo "Setup Complete!"
echo "=========================================="
echo ""
echo "Your executables are ready:"
echo "  - ./ctp_sender"
echo "  - ./ctp_receiver"
echo ""
echo "Next steps:"
echo "  1. Review LAB_TESTING_GUIDE.md for detailed testing procedures"
echo "  2. Follow LAB_CHECKLIST.md for systematic testing"
echo "  3. Test against reference implementation (if available)"
echo ""
echo "Quick test commands:"
echo ""
echo "  # Your receiver:"
echo "  ./ctp_receiver 5000 output.txt"
echo ""
echo "  # Your sender (in another terminal):"
echo "  ./ctp_sender -f test_small.txt localhost 5000"
echo ""
echo "  # With reference receiver (if available):"
echo "  ./reference_binaries/ctp_receiver_ref 5001 ref_output.txt"
echo "  ./ctp_sender -f test_small.txt localhost 5001"
echo ""
echo "Logs saved to: receiver.log, sender.log"
echo ""
