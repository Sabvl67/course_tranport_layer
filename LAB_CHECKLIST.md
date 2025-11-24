# Lab Machine Testing Checklist

## Before Going to Lab

- [x] CTP sender implementation complete
- [x] CTP receiver implementation complete
- [x] Code compiles without errors
- [x] Testing guide prepared
- [ ] Have USB drive or way to transfer files to lab machine

---

## At the Lab - Step by Step

### 1. Initial Setup (5-10 min)

- [ ] Log into lab machine (student0.cs.uleth.ca or similar)
- [ ] Create working directory: `mkdir ~/ctp_project && cd ~/ctp_project`
- [ ] Transfer your code to lab machine
  ```bash
  # Option A: Git (if you have repo)
  git clone <your-repo-url>

  # Option B: SCP from another machine
  scp -r /path/to/code username@student0.cs.uleth.ca:~/ctp_project/

  # Option C: USB drive
  cp -r /media/usb/course_tranport_layer/* ~/ctp_project/
  ```

### 2. Find Reference Implementation (5 min)

- [ ] Locate reference files:
  ```bash
  ls -la /home/lib3780/project/
  ls -la /home/lib3780/project/reference-implementation/
  ```
- [ ] Note actual filenames (they might differ from guide)
- [ ] Copy reference binaries:
  ```bash
  cp /home/lib3780/project/reference-implementation/* ~/ctp_project/reference_binaries/
  ```
- [ ] Check protocol documentation:
  ```bash
  find /home/lib3780 -name "*chapter*8*" -o -name "*section*9*"
  ```

### 3. Compile Your Code (5 min)

- [ ] Compile sender:
  ```bash
  g++ -std=c++17 -Wall -O2 -I. -o ctp_sender src/sender.cc -lz
  ```
- [ ] Compile receiver:
  ```bash
  g++ -std=c++17 -Wall -O2 -I. -o ctp_receiver src/ctp_receiver.cc -lz
  ```
- [ ] Verify compilation:
  ```bash
  ls -l ctp_sender ctp_receiver
  ./ctp_sender --help 2>&1 | head -2  # Check if runs
  ./ctp_receiver --help 2>&1 | head -2
  ```

### 4. Create Test Files (2 min)

- [ ] Small text file:
  ```bash
  echo "CTP Protocol Test - Small File" > test_small.txt
  ```
- [ ] Medium file (256 bytes):
  ```bash
  dd if=/dev/urandom of=test_256.bin bs=1 count=256
  ```
- [ ] Maximum size (512 bytes):
  ```bash
  dd if=/dev/urandom of=test_512.bin bs=1 count=512
  ```
- [ ] Verify sizes:
  ```bash
  ls -lh test_*.txt test_*.bin
  wc -c test_*
  ```

### 5. Test Reference Implementation First (10 min)

**Why:** Verify network setup and understand expected behavior

- [ ] **Test A: Reference sender → Reference receiver**
  ```bash
  # Terminal 1
  ./reference_receiver 5000 ref_output.txt

  # Terminal 2
  ./reference_sender -f test_small.txt localhost 5000

  # Verify
  diff test_small.txt ref_output.txt
  ```
  - [ ] Transfer successful
  - [ ] Files match (diff shows no output)
  - [ ] Note any output messages

### 6. Exercise 1 - Main Tests (30 min)

#### Test 1: Your Sender → Reference Receiver ⭐ CRITICAL

- [ ] Start reference receiver:
  ```bash
  # Terminal 1
  ./reference_receiver 5001 from_your_sender.txt
  ```
- [ ] Run your sender:
  ```bash
  # Terminal 2
  ./ctp_sender -f test_small.txt localhost 5001
  ```
- [ ] Check results:
  ```bash
  diff test_small.txt from_your_sender.txt
  echo "Exit code: $?"  # 0 = success
  ```
- [ ] Record any errors: ________________
- [ ] Status: ❌ FAIL / ⚠️ PARTIAL / ✅ PASS

#### Test 2: Reference Sender → Your Receiver ⭐ CRITICAL

- [ ] Start your receiver:
  ```bash
  # Terminal 1
  ./ctp_receiver 5002 from_ref_sender.txt
  ```
- [ ] Run reference sender:
  ```bash
  # Terminal 2
  ./reference_sender -f test_small.txt localhost 5002
  ```
- [ ] Check results:
  ```bash
  diff test_small.txt from_ref_sender.txt
  ```
- [ ] Record any errors: ________________
- [ ] Status: ❌ FAIL / ⚠️ PARTIAL / ✅ PASS

#### Test 3: Your Sender → Your Receiver

- [ ] Start your receiver:
  ```bash
  ./ctp_receiver 5003 end_to_end.txt
  ```
- [ ] Run your sender:
  ```bash
  ./ctp_sender -f test_small.txt localhost 5003
  ```
- [ ] Check results:
  ```bash
  diff test_small.txt end_to_end.txt
  ```
- [ ] Status: ❌ FAIL / ⚠️ PARTIAL / ✅ PASS

### 7. Additional Tests (15 min)

- [ ] **Test with 512-byte file:**
  ```bash
  ./ctp_receiver 5004 max_size.bin
  ./ctp_sender -f test_512.bin localhost 5004
  diff test_512.bin max_size.bin
  ```

- [ ] **Test IPv6:**
  ```bash
  ./ctp_receiver -6 5005 ipv6_test.txt
  ./ctp_sender -f test_small.txt ::1 5005
  diff test_small.txt ipv6_test.txt
  ```

- [ ] **Test cross-machine (if available):**
  ```bash
  # On student0
  ./ctp_receiver 5006 cross_machine.txt

  # On student1
  ./ctp_sender -f test_small.txt student0.cs.uleth.ca 5006
  ```

### 8. If Tests Fail - Debug (20-30 min)

#### Problem: Sender hangs after sending

- [ ] Check if receiver is running: `ps aux | grep ctp`
- [ ] Check ports match
- [ ] Add debug output:
  ```cpp
  // In sender after sendto():
  std::cout << "Sent " << sent << " bytes, waiting for ACK..." << std::endl;
  ```
- [ ] Recompile and test
- [ ] Capture packets:
  ```bash
  sudo tcpdump -i lo -w debug.pcap port 5000 &
  # Run test
  # Analyze: tcpdump -r debug.pcap -X
  ```

#### Problem: CRC mismatch

- [ ] Verify CRC calculation matches reference
- [ ] Check byte order (big-endian)
- [ ] Verify TR bit forced to 0 before CRC
- [ ] Test CRC function:
  ```bash
  cd 4-get-data
  g++ -o test_crc crc32.cc -lz
  ./test_crc
  # Expected: 1b851995
  ```

#### Problem: Unexpected packet type

- [ ] Print first byte in hex:
  ```cpp
  printf("First byte: 0x%02x\n", recv_buf[0]);
  printf("Type: %d\n", (recv_buf[0] >> 6) & 0x03);
  ```
- [ ] Verify bit layout: `(type << 6) | (tr << 5) | window`
- [ ] DATA type = 1, ACK type = 2

### 9. Document Results (10 min)

- [ ] Take screenshots of successful tests
- [ ] Save test output:
  ```bash
  script -a test_results.log
  # Run all tests
  # Press Ctrl+D when done
  ```
- [ ] Note any issues encountered
- [ ] List what works and what doesn't

### 10. Prepare for Submission

- [ ] Clean build:
  ```bash
  make clean
  rm -f *.o *.d ctp_sender ctp_receiver
  ```
- [ ] Rebuild:
  ```bash
  g++ -std=c++17 -Wall -O2 -I. -o ctp_sender src/sender.cc -lz
  g++ -std=c++17 -Wall -O2 -I. -o ctp_receiver src/ctp_receiver.cc -lz
  ```
- [ ] Final test
- [ ] Package for submission:
  ```bash
  tar czf ctp_project_submission.tar.gz src/ include/ test/ Makefile README.md
  ```

---

## Success Criteria Summary

### Must Work (Required for Exercise 1):
- ✅ Your sender → Reference receiver (with 512-byte file)
- ✅ Reference sender → Your receiver (with 512-byte file)

### Should Work (Full credit):
- ✅ Your sender → Your receiver
- ✅ IPv4 transfers
- ✅ Files match byte-for-byte (verified with diff)

### Nice to Have (Extra confidence):
- ⭐ IPv6 transfers
- ⭐ Cross-machine transfers
- ⭐ Multiple file sizes (1 byte, 256 bytes, 512 bytes)

---

## Time Estimate

- Setup and compile: 15 min
- Reference tests: 10 min
- Your tests: 30 min
- Debug (if needed): 30-60 min
- Total: **1.5 - 2 hours**

---

## Emergency Contacts

If stuck, check:
1. Lab manual Section 6.4 Exercise 1
2. Protocol documentation (Chapter 8)
3. Beej's Guide: https://beej.us/guide/bgnet/
4. Ask TA during lab hours
5. Check course Moodle forum

---

## Notes Section

Use this space to record observations:

**What worked well:**


**What didn't work:**


**Error messages encountered:**


**Solutions found:**


**Questions for TA:**
