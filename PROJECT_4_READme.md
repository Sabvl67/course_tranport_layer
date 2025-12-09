## Command
# BUILD COMMANDS
- make
- make clean 

### PROJECT 4 COMMANDS

### 1. Receiver correctness with reference sender (IPv4 + IPv6)

## Test 1A — Receiver IPv4 receiving large file (>512 bytes)

# Terminal 1

./ctp_receiver 5001 out_large_ipv4.bin

# Terminal 2

./reference-implementation/sender -f test_files/test_large.bin localhost 5001

# Verification

diff test_files/test_large.bin out_large_ipv4.bin && echo "PASS"


## Test 1B — Receiver IPv6 receiving large file (>512 bytes)

# Terminal 1

./ctp_receiver -6 5002 out_large_ipv6.bin

# Terminal 2

./reference-implementation/sender -f test_files/test_large.bin ::1 5002

# Verification

diff test_files/test_large.bin out_large_ipv6.bin && echo "PASS"

### 2. Sender correctness with reference receiver (IPv4 + IPv6)

## Test 2A — Sender IPv4 sending large file (>512 bytes)

# Terminal 1

./reference-implementation/receiver -f out_sender_large_ipv4.bin localhost 5003

# Terminal 2

./ctp_sender -f test_files/test_large.bin localhost 5003

# Verification

diff test_files/test_large.bin out_sender_large_ipv4.bin && echo "PASS"

## Test 2B — Sender IPv6 sending large file (>512 bytes)

# Terminal 1

./reference-implementation/receiver -f out_sender_large_ipv6.bin ::1 5004

# Terminal 2

./ctp_sender -f test_files/test_large.bin ::1 5004

# Verification

diff test_files/test_large.bin out_sender_large_ipv6.bin && echo "PASS"

### 3. Sender Honors Window Size

# Terminal 1 

./reference-implementation/receiver -b 1 -f out_window.bin localhost 5005

# Terminal 2

./ctp_sender -f test_files/test_large.bin localhost 5005

# Verification 

diff test_files/test_large.bin out_window.bin && echo "PASS"

### 4. Sender + Receiver Work Together (IPv4 + IPv6)

## Test 4A — Sender → Receiver (IPv4)

# Terminal 1

./ctp_receiver 5006 out_self_ipv4.bin

# Terminal 2

./ctp_sender -f test_files/test_large.bin localhost 5006

# Verification

diff test_files/test_large.bin out_self_ipv4.bin && echo "PASS"

## Test 4B — Sender → Receiver (IPv6)

# Terminal 1

./ctp_receiver -6 5007 out_self_ipv6.bin

# Terminal 2

./ctp_sender -f test_files/test_large.bin ::1 5007

# Verification

diff test_files/test_large.bin out_self_ipv6.bin && echo "PASS"







