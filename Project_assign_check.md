# Project Assigment Checklist

## ✅

### 1. Code Implementation ✅ COMPLETE
- ✅ CTP sender ([src/ctp.cc])
- ✅ CTP receiver ([src/ctp.cc])
- ✅ IPv4/IPv6 support via `getaddrinfo()`
- ✅ CRC1 and CRC2 calculation with zlib

### 2. Reference Implementation Analysis ✅ COMPLETE
- ✅ Located reference binaries ([reference-implementation/])
- ✅ Analyzed packet capture ([short.pcap])
- ✅ Verified packet format matches your implementation
- ✅ Confirmed protocol flow (DATA → ACK → FIN → ACK)

### 3. Test Files Created ✅ COMPLETE
- ✅ `test_files/test_1byte.bin` (1 byte)
- ✅ `test_files/test_256bytes.bin` (256 bytes)
- ✅ `test_files/test_512bytes.bin` (512 bytes)

### 4. Testing Script ✅ READY
- ✅ [test_with_reference.sh]
  - Tests IPv4: 1, 256, 512 byte files
  - Tests IPv6: 1, 512 byte files
  - Compares outputs automatically
  - Saves logs for debugging

---

### Use Lab Machine 

# Test
cd ~/course_tranport_layer/
./test_with_reference.sh


---


### Step 3: Review Results
```bash
# Check test results
ls -lh test_results/

# If all tests pass:
✅ Ready to submit!

# If any test fails:
cat test_results/ipv4_1byte_sender.log      # Your sender output
cat test_results/ipv4_1byte_receiver.log    # Reference receiver output
diff -y test_files/test_1byte.bin test_results/ipv4_1byte_output.bin | head
```

---

## 🔍 What the Test Script Does

```
For each test:
1. Start reference receiver on unique port
2. Run your sender to connect to it
3. Wait for transfer to complete
4. Compare input file vs output file byte-by-byte
5. Report PASS/FAIL
6. Save all logs

Test Coverage:
├─ IPv4 tests (ports 5001, 5002, 5003)
│  ├─ 1 byte file
│  ├─ 256 byte file
│  └─ 512 byte file
└─ IPv6 tests (ports 5004, 5005)
   ├─ 1 byte file
   └─ 512 byte file
```

---

### Code Structure
```
course_tranport_layer/
├── src/
│   ├── ctp.cc           ✅ Protocol implementation
│   ├── sender.cc        ✅ Sender main
│   ├── ctp_receiver.cc  ✅ Receiver main (bonus)
│   └── header.cc        ✅ Packet headers
├── include/
│   └── ctp.h            ✅ Function declarations
├── test/                
├── Makefile             
└── README.md            ✅ Documentation
```


## 🎓 Final Checklist

Before submission:
- [ ] Run `./test_with_reference.sh` on Linux
- [ ] All 5 tests pass (IPv4 + IPv6)
- [ ] Code compiles without warnings
- [ ] Command line arguments work: `-f filename host port`
- [ ] Files in src/ and include/ only
- [ ] Code is commented appropriately
- [ ] README explains your implementation

Once all checked:
- [ ] Create tarball: `tar czf submission.tar.gz src/ include/`
- [ ] Test extraction: `tar xzf submission.tar.gz && ls`
- [ ] Submit to course portal


