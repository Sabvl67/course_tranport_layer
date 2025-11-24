# Lab N6 Exercise 1 - Testing Guide

## Overview
This guide helps you test your CTP sender/receiver against the reference implementation on lab machines.

---

## Current Status

### ✅ Completed Implementation

Your code includes:

1. **CTP Sender** - [ctp_sender](ctp_sender)
   - Command line: `./ctp_sender [-f file] host port`
   - Reads file (max 512 bytes)
   - Sends DATA packet with CRC1 and CRC2
   - Waits for ACK
   - Sends zero-length DATA to signal end
   - Waits for final ACK

2. **CTP Receiver** - [ctp_receiver](ctp_receiver)
   - Command line: `./ctp_receiver [-4|-6] port output_file`
   - Binds to specified port
   - Receives DATA packets, verifies CRC
   - Sends ACK packets
   - Writes payload to binary file
   - Terminates on zero-length DATA

3. **Supporting Files**:
   - [src/ctp.cc](src/ctp.cc) - All CTP protocol logic
   - [include/ctp.h](include/ctp.h) - Function declarations
   - [src/header.cc](src/header.cc) - Packet header manipulation

---

## Testing on Lab Machines

### Step 1: Copy Your Code to Lab Machine

```bash
# From your local machine
scp -r ~/Desktop/course_tranport_layer username@student0.cs.uleth.ca:~/ctp_project/

# SSH into lab machine
ssh username@student0.cs.uleth.ca
cd ~/ctp_project/
```

### Step 2: Compile Your Code

```bash
# Compile sender
g++ -std=c++17 -Wall -O2 -I. -o ctp_sender src/sender.cc -lz

# Compile receiver
g++ -std=c++17 -Wall -O2 -I. -o ctp_receiver src/ctp_receiver.cc -lz

# Verify binaries exist
ls -l ctp_sender ctp_receiver
```

### Step 3: Get Reference Implementation

```bash
# Copy reference binaries (mentioned in Lab N6)
cp /home/lib3780/project/reference-implementation/ctp_sender_ref ./
cp /home/lib3780/project/reference-implementation/ctp_receiver_ref ./

# Make executable if needed
chmod +x ctp_sender_ref ctp_receiver_ref

# Check what's available
ls -l /home/lib3780/project/
```

### Step 4: Create Test Files

```bash
# Small test file (< 512 bytes)
echo "Hello from CTP protocol!" > test_small.txt

# Binary test file
dd if=/dev/urandom of=test_binary.bin bs=1 count=256

# Max size test (512 bytes exactly)
dd if=/dev/urandom of=test_512.bin bs=1 count=512

# Check sizes
ls -l test_*.txt test_*.bin
```

---

## Test Scenarios

### Test 1: Your Sender → Reference Receiver

**Goal:** Verify your sender works with the standard receiver

```bash
# Terminal 1: Start reference receiver
./ctp_receiver_ref 4000 received_output.txt

# Terminal 2: Run your sender
./ctp_sender -f test_small.txt localhost 4000

# Verify output
diff test_small.txt received_output.txt
echo "Exit code: $?"  # Should be 0 if files match
```

**Expected Result:** Files should be identical

---

### Test 2: Reference Sender → Your Receiver

**Goal:** Verify your receiver works with the standard sender

```bash
# Terminal 1: Start your receiver
./ctp_receiver 4001 my_received.txt

# Terminal 2: Run reference sender
./ctp_sender_ref -f test_small.txt localhost 4001

# Verify output
diff test_small.txt my_received.txt
echo "Exit code: $?"  # Should be 0
```

**Expected Result:** Files should be identical

---

### Test 3: Your Sender → Your Receiver

**Goal:** End-to-end test of your implementation

```bash
# Terminal 1: Start your receiver
./ctp_receiver -4 4002 final_output.txt

# Terminal 2: Run your sender
./ctp_sender -f test_512.bin localhost 4002

# Verify
diff test_512.bin final_output.txt
```

**Expected Result:** Files should be identical

---

### Test 4: IPv6 Testing

```bash
# Terminal 1: Receiver with IPv6
./ctp_receiver -6 4003 ipv6_output.txt

# Terminal 2: Sender to IPv6 localhost
./ctp_sender -f test_small.txt ::1 4003

# Verify
diff test_small.txt ipv6_output.txt
```

---

### Test 5: Cross-Machine Testing

```bash
# On student0:
./ctp_receiver 4004 cross_machine.txt

# On student1 (different terminal/SSH session):
./ctp_sender -f test_small.txt student0.cs.uleth.ca 4004

# Back on student0:
diff test_small.txt cross_machine.txt
```

---

## Debugging with tcpdump

### Capture Packets

```bash
# Start packet capture (need sudo or specific permissions)
tcpdump -i lo -w ctp_capture.pcap port 4000

# In another terminal, run your test
./ctp_receiver 4000 output.txt &
./ctp_sender -f test_small.txt localhost 4000

# Stop tcpdump with Ctrl+C

# Analyze capture
tcpdump -r ctp_capture.pcap -X
```

### Look for Packet Structure

Expected DATA packet structure (12-byte header + payload + 4-byte CRC2):
```
Byte 0:    Type(2) | TR(1) | Window(5)
Byte 1:    Sequence number
Bytes 2-3: Payload length (big-endian)
Bytes 4-7: Timestamp
Bytes 8-11: CRC1
Bytes 12+: Payload
Last 4:    CRC2 (if payload > 0)
```

Expected ACK packet structure (12-byte header only):
```
Byte 0:    Type(2) | TR(1) | Window(5)  [Type should be 0x80 for ACK]
Byte 1:    Sequence number
Bytes 2-3: Length = 0
Bytes 4-7: Timestamp
Bytes 8-11: CRC1
```

---

## Troubleshooting

### Problem: Sender hangs waiting for ACK

**Check:**
1. Is receiver actually running? `ps aux | grep ctp_receiver`
2. Are they using the same port?
3. Firewall blocking? `sudo iptables -L`
4. Check receiver output for error messages

**Debug:**
```bash
# Add debug output temporarily
# In receiver loop, before recvfrom, add:
std::cout << "Waiting for packet..." << std::endl;
```

### Problem: CRC mismatch errors

**Check:**
1. Byte order (big-endian vs little-endian)
2. TR bit must be forced to 0 before CRC calculation
3. CRC calculated over first 8 bytes only for CRC1

**Verify CRC calculation:**
```bash
# In 4-get-data directory
g++ -o test_crc crc32.cc -lz
./test_crc
# Should output: The crc32 value for: Hello world! is 1b851995
```

### Problem: "Unexpected packet type"

**Check:**
1. First byte format: `(type << 6) | (tr << 5) | window`
2. DATA type = 1 (0x40 when shifted)
3. ACK type = 2 (0x80 when shifted)

**Debug packet contents:**
```cpp
// Add to receiver before parse_and_verify_data:
std::cout << "Received " << recvd << " bytes" << std::endl;
std::cout << "First byte: 0x" << std::hex << (int)recv_buf[0] << std::endl;
std::cout << "Type bits: " << ((recv_buf[0] >> 6) & 0x03) << std::dec << std::endl;
```

---

## Reference Implementation Files Location

Look for these on lab machines:
```
/home/lib3780/project/reference-implementation/
├── ctp_sender_ref          # Reference sender
├── ctp_receiver_ref        # Reference receiver
├── link_simulator          # For simulating network errors (later)
└── sample_capture.pcap     # Example packet capture
```

Also check for:
```
/home/lib3780/project/dissector-CPSC3780.lua  # Wireshark dissector
```

---

## Success Criteria for Exercise 1

✅ **Your sender successfully sends file to reference receiver**
- [ ] File transferred completely
- [ ] No CRC errors
- [ ] Output file matches input file byte-for-byte

✅ **Your receiver successfully receives file from reference sender**
- [ ] File received completely
- [ ] No CRC errors
- [ ] Output file matches input file byte-for-byte

✅ **Your implementation works end-to-end**
- [ ] Your sender → Your receiver works
- [ ] Transfers work over IPv4
- [ ] Transfers work over IPv6 (optional)
- [ ] Works across different machines

---

## Quick Test Script

Save this as `test_all.sh`:

```bash
#!/bin/bash

echo "=== CTP Testing Script ==="

# Create test file
echo "Testing CTP protocol implementation" > test.txt

echo "Test 1: Your sender -> Reference receiver"
./ctp_receiver_ref 5000 ref_recv.txt &
RECV_PID=$!
sleep 1
./ctp_sender -f test.txt localhost 5000
wait $RECV_PID
diff test.txt ref_recv.txt && echo "✅ PASS" || echo "❌ FAIL"

echo ""
echo "Test 2: Reference sender -> Your receiver"
./ctp_receiver 5001 my_recv.txt &
RECV_PID=$!
sleep 1
./ctp_sender_ref -f test.txt localhost 5001
wait $RECV_PID
diff test.txt my_recv.txt && echo "✅ PASS" || echo "❌ FAIL"

echo ""
echo "Test 3: Your sender -> Your receiver"
./ctp_receiver 5002 final.txt &
RECV_PID=$!
sleep 1
./ctp_sender -f test.txt localhost 5002
wait $RECV_PID
diff test.txt final.txt && echo "✅ PASS" || echo "❌ FAIL"

# Cleanup
rm -f test.txt ref_recv.txt my_recv.txt final.txt
```

Make executable: `chmod +x test_all.sh`

---

## Notes

- Port numbers must be > 1024 (non-privileged)
- Use different ports for each test to avoid conflicts
- Kill old processes: `pkill -f ctp_receiver`
- Check open ports: `netstat -tulpn | grep <port>`

Good luck! 🚀
