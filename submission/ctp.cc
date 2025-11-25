// src/ctp.cpp
#include "../include/ctp.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>

#include <zlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// ----- helpers for big-endian -----

static inline void put_u16_be(uint8_t* p, uint16_t v) {
    p[0] = uint8_t(v >> 8);
    p[1] = uint8_t(v & 0xFF);
}

static inline void put_u32_be(uint8_t* p, uint32_t v) {
    p[0] = uint8_t(v >> 24);
    p[1] = uint8_t(v >> 16);
    p[2] = uint8_t(v >> 8);
    p[3] = uint8_t(v & 0xFF);
}

static inline uint32_t read_u32_be(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) |
           (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8)  |
           (uint32_t(p[3]));
}

static inline uint16_t read_u16_be(const uint8_t* p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

// ----- build DATA packet -----

std::size_t build_ctp_data_packet(
    uint8_t* out_buffer,
    std::size_t out_capacity,
    const uint8_t* payload,
    std::size_t payload_len,
    uint8_t seqnum,
    uint8_t window
) {
    if (payload_len > CTP_MAX_PAYLOAD) {
        std::cerr << "Payload too large (>512 bytes).\n";
        return 0;
    }

    // Header (8 bytes) + payload + optional CRC2 (4 bytes if payload_len > 0)
    std::size_t total_len = CTP_HEADER_LEN + payload_len + (payload_len > 0 ? 4 : 0);
    if (out_capacity < total_len) {
        std::cerr << "Output buffer too small for CTP packet.\n";
        return 0;
    }

    uint8_t* buf = out_buffer;

    // First byte: Type (bits 7-6), TR (bit 5), Window (bits 4-0)
    uint8_t type   = CTP_PTYPE_DATA;
    uint8_t tr     = 0;
    window         = uint8_t(window & 0x1F); // 5 bits

    uint8_t first =
         uint8_t(((type & 0x03u) << 6) |
                 ((tr   & 0x01u) << 5) |
                 (window & 0x1Fu));
    
    buf[0] = first;
    buf[1] = seqnum;
    put_u16_be(buf + 2, uint16_t(payload_len));

    // timestamp @ 4bytes
    uint32_t ts =  uint32_t(std::time(nullptr));
    put_u32_be(buf + 4, ts);

    // CRC1 init
    buf[8]  = 0;
    buf[9]  = 0;
    buf[10] = 0;
    buf[11] = 0;

    // CRC over first 8 bytes
    uint8_t header_for_crc[8];
    std::memcpy(header_for_crc, buf, 8);

    // Ensure TR copy bit is 0
    header_for_crc[0] &= uint8_t(~(1u << 5));

    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, header_for_crc, 8);
    put_u32_be(buf + 8,  uint32_t(crc));

    // Payload
    if (payload_len > 0 && payload != nullptr) {
        std::memcpy(buf + CTP_HEADER_LEN, payload, payload_len);

        // CRC2 over payload
        uLong crc_payload = crc32(0L, Z_NULL, 0);
        crc_payload = crc32(crc_payload, payload, payload_len);
        put_u32_be(buf + CTP_HEADER_LEN + payload_len,
                    uint32_t(crc_payload));
    }

    return total_len;
}

// ----- parse + verify ACK -----

bool parse_and_verify_ack(
    const uint8_t* buffer,
    std::size_t length,
    uint8_t& out_window,
    uint8_t& out_seqnum
) {
    if (length < CTP_HEADER_LEN) {
        std::cerr << "ACK too short.\n";
        return false;
    }

    const uint8_t* buf = buffer;

    uint8_t first = buf[0];
    uint8_t type   = uint8_t((first >> 6) & 0x03u);
    uint8_t tr     = uint8_t((first >> 5) & 0x01u);
    uint8_t window = uint8_t(first & 0x1Fu);

    if (type != CTP_PTYPE_ACK) {
        std::cerr << "Unexpected packet type (expected ACK).\n";
        return false;
    }
    if (tr != 0) {
        std::cerr << "ACK has TR=1, invalid.\n";
        return false;
    }

    // Check CRC1
    uint32_t stored_crc = read_u32_be(buf + 8);

    // Build copy of first 8 bytes, force TR=0
    uint8_t header_for_crc[8];
    std::memcpy(header_for_crc, buf, 8);
    header_for_crc[0] &= uint8_t(~(1u << 5));

    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, header_for_crc, 8);

    if (stored_crc != uint32_t(crc)) {
        std::cerr << "ACK CRC1 mismatch.\n";
        return false;
    }

    out_window = window;
    out_seqnum = buf[1]; // Seqnum field
    return true;
}

// ----- parse + verify DATA packet -----

bool parse_and_verify_data(
    const uint8_t* buffer,
    std::size_t length,
    uint8_t& out_type,
    uint8_t& out_window,
    uint8_t& out_seqnum,
    uint16_t& out_payload_len,
    const uint8_t*& out_payload
) {
    if (length < CTP_HEADER_LEN) {
        std::cerr << "DATA packet too short.\n";
        return false;
    }

    const uint8_t* buf = buffer;

    uint8_t first = buf[0];
    uint8_t type   = uint8_t((first >> 6) & 0x03u);
    uint8_t tr     = uint8_t((first >> 5) & 0x01u);
    uint8_t window = uint8_t(first & 0x1Fu);

    if (type != CTP_PTYPE_DATA) {
        std::cerr << "Unexpected packet type (expected DATA).\n";
        return false;
    }

    // Check CRC1
    uint32_t stored_crc1 = read_u32_be(buf + 8);

    // Build copy of first 8 bytes, force TR=0
    uint8_t header_for_crc[8];
    std::memcpy(header_for_crc, buf, 8);
    header_for_crc[0] &= uint8_t(~(1u << 5));

    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, header_for_crc, 8);

    if (stored_crc1 != uint32_t(crc)) {
        std::cerr << "DATA CRC1 mismatch.\n";
        return false;
    }

    // Extract payload length
    uint16_t payload_len = read_u16_be(buf + 2);

    // Check if we have enough data for payload + CRC2
    if (payload_len > 0) {
        if (length < CTP_HEADER_LEN + payload_len + 4) {
            std::cerr << "DATA packet too short for payload + CRC2.\n";
            return false;
        }

        // Verify CRC2 over payload
        const uint8_t* payload = buf + CTP_HEADER_LEN;
        uint32_t stored_crc2 = read_u32_be(buf + CTP_HEADER_LEN + payload_len);

        uLong crc_payload = crc32(0L, Z_NULL, 0);
        crc_payload = crc32(crc_payload, payload, payload_len);

        if (stored_crc2 != uint32_t(crc_payload)) {
            std::cerr << "DATA CRC2 mismatch.\n";
            return false;
        }

        out_payload = payload;
    } else {
        out_payload = nullptr;
    }

    out_type = type;
    out_window = window;
    out_seqnum = buf[1];
    out_payload_len = payload_len;
    return true;
}

// ----- build ACK packet -----

std::size_t build_ctp_ack_packet(
    uint8_t* out_buffer,
    std::size_t out_capacity,
    uint8_t seqnum,
    uint8_t window
) {
    if (out_capacity < CTP_HEADER_LEN) {
        std::cerr << "Output buffer too small for ACK packet.\n";
        return 0;
    }

    uint8_t* buf = out_buffer;

    // First byte: Type (bits 7-6), TR (bit 5), Window (bits 4-0)
    uint8_t type = CTP_PTYPE_ACK;
    uint8_t tr   = 0;
    window       = uint8_t(window & 0x1F);

    uint8_t first =
         uint8_t(((type & 0x03u) << 6) |
                 ((tr   & 0x01u) << 5) |
                 (window & 0x1Fu));

    buf[0] = first;
    buf[1] = seqnum;
    put_u16_be(buf + 2, 0); // Length = 0 for ACK

    // Timestamp
    uint32_t ts = uint32_t(std::time(nullptr));
    put_u32_be(buf + 4, ts);

    // CRC1 init
    buf[8]  = 0;
    buf[9]  = 0;
    buf[10] = 0;
    buf[11] = 0;

    // CRC over first 8 bytes
    uint8_t header_for_crc[8];
    std::memcpy(header_for_crc, buf, 8);
    header_for_crc[0] &= uint8_t(~(1u << 5));

    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, header_for_crc, 8);
    put_u32_be(buf + 8, uint32_t(crc));

    return CTP_HEADER_LEN;
}

// ----- receiver main routine -----

static void print_receiver_usage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " [-4|-6] port output_file\n";
}

int run_receiver(int argc, char* argv[]) {
    int ip_version = AF_UNSPEC;
    std::string port;
    std::string output_file;

    int arg_index = 1;

    // Parse optional -4 or -6 flag
    if (arg_index < argc) {
        if (std::strcmp(argv[arg_index], "-4") == 0) {
            ip_version = AF_INET;
            arg_index++;
        } else if (std::strcmp(argv[arg_index], "-6") == 0) {
            ip_version = AF_INET6;
            arg_index++;
        }
    }

    // Need port and output_file
    if (argc - arg_index != 2) {
        print_receiver_usage(argv[0]);
        return 1;
    }

    port = argv[arg_index];
    output_file = argv[arg_index + 1];

    // --- Set up socket and bind ---

    struct addrinfo hints;
    struct addrinfo* results = nullptr;
    struct addrinfo* ptr = nullptr;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = ip_version;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0x01;  // Passive - for binding

    int rval = getaddrinfo(nullptr, port.c_str(), &hints, &results);
    if (rval != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rval) << "\n";
        return 2;
    }

    int sock = -1;
    for (ptr = results; ptr != nullptr; ptr = ptr->ai_next) {
        sock = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1) {
            break;
        }
    }

    if (sock == -1 || ptr == nullptr) {
        std::cerr << "Failed to open socket.\n";
        freeaddrinfo(results);
        return 3;
    }

    if (bind(sock, ptr->ai_addr, ptr->ai_addrlen) == -1) {
        std::cerr << "Cannot bind socket.\n";
        freeaddrinfo(results);
        close(sock);
        return 4;
    }

    std::cout << "Receiver listening on port " << port << std::endl;

    // --- Open output file ---

    std::ofstream outfile(output_file, std::ios::binary);
    if (!outfile) {
        std::cerr << "Cannot open output file: " << output_file << "\n";
        freeaddrinfo(results);
        close(sock);
        return 5;
    }

    // --- Receive loop ---

    uint8_t recv_buf[CTP_HEADER_LEN + CTP_MAX_PAYLOAD + 4];
    uint8_t send_buf[CTP_HEADER_LEN];
    struct sockaddr_storage from_addr;
    socklen_t from_len;

    uint8_t expected_seq = 0;
    bool transfer_complete = false;

    while (!transfer_complete) {
        from_len = sizeof(from_addr);
        ssize_t recvd = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
                                  (struct sockaddr*)&from_addr, &from_len);

        if (recvd < 0) {
            std::perror("recvfrom");
            break;
        }

        // Parse DATA packet
        uint8_t pkt_type = 0;
        uint8_t pkt_window = 0;
        uint8_t pkt_seqnum = 0;
        uint16_t payload_len = 0;
        const uint8_t* payload = nullptr;

        if (!parse_and_verify_data(recv_buf, std::size_t(recvd),
                                    pkt_type, pkt_window, pkt_seqnum,
                                    payload_len, payload)) {
            std::cerr << "Failed to parse/verify DATA packet.\n";
            continue;  // Wait for valid packet
        }

        std::cout << "Received DATA: seq=" << int(pkt_seqnum)
                  << " len=" << payload_len << std::endl;

        // Check for end-of-transfer (zero-length payload)
        if (payload_len == 0) {
            std::cout << "End of transfer received." << std::endl;
            transfer_complete = true;
        } else {
            // Write payload to file
            outfile.write(reinterpret_cast<const char*>(payload), payload_len);
        }

        // Build and send ACK
        uint8_t next_seq = uint8_t((pkt_seqnum + 1) % 256);
        std::size_t ack_len = build_ctp_ack_packet(send_buf, sizeof(send_buf),
                                                    next_seq, 1);

        if (ack_len == 0) {
            std::cerr << "Failed to build ACK packet.\n";
            continue;
        }

        ssize_t sent = sendto(sock, send_buf, ack_len, 0,
                               (struct sockaddr*)&from_addr, from_len);

        if (sent < 0) {
            std::perror("sendto");
        } else {
            std::cout << "Sent ACK: seq=" << int(next_seq) << std::endl;
        }

        expected_seq = next_seq;
    }

    outfile.close();
    freeaddrinfo(results);
    close(sock);

    std::cout << "File saved to: " << output_file << std::endl;
    return 0;
}

// ----- sender main routine -----

static void print_usage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " [-f data_file] host port\n";
}

int run_sender(int c, char* v[]) {
    std::string filename;
    std::string host;
    std::string port;

    int x = 1;
    if (x < c && std::string(v[x]) == "-f") {
        if (x + 1 >= c) {
            print_usage(v[0]);
            return 1;
        }
        filename = v[x + 1];
        x += 2;
    }

    if (c - x != 2) {
        print_usage(v[0]);
        return 1;
    }

    host = v[x];
    port = v[x + 1];

    // --- Read data (≤512 bytes) ---

    std::vector<uint8_t> data;

    if (!filename.empty()) {
       std::ifstream in(filename, std::ios::binary);
        if (!in) {
            std::cerr << "Cannot open file: " << filename << "\n";
            return 1;
        }
        data.assign(std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>());
    } else {
        // read from stdin
        std::istreambuf_iterator<char> it(std::cin);
        std::istreambuf_iterator<char> end;
        for (; it != end; ++it) {
            data.push_back( uint8_t(*it));
        }
    }

    if (data.size() > CTP_MAX_PAYLOAD) {
        std::cerr << "Input larger than 512 bytes; not supported in Prototype 2.\n";
        return 1;
    }

    // --- Resolve host/port and open UDP socket (IPv4 or IPv6) ---

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;  // UDP

    struct addrinfo* results = nullptr;
    int rval = getaddrinfo(host.c_str(), port.c_str(), &hints, &results);
    if (rval != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rval) << "\n";
        return 2;
    }

    int sock = -1;
    struct addrinfo* ptr;
    for (ptr = results; ptr != nullptr; ptr = ptr->ai_next) {
        sock = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1) {
            break; // success
        }
    }

    if (sock == -1 || ptr == nullptr) {
        std::cerr << "Failed to open UDP socket.\n";
        freeaddrinfo(results);
        return 3;
    }

    // --- Build and send DATA packet with file contents ---

    uint8_t send_buf[CTP_HEADER_LEN + CTP_MAX_PAYLOAD + 4]; // enough for payload + CRC2
    uint8_t seqnum = 0;

    std::size_t data_pkt_len = build_ctp_data_packet(
        send_buf,
        sizeof(send_buf),
        data.empty() ? nullptr : data.data(),
        data.size(),
        seqnum,
        1 // window
    );

    if (data_pkt_len == 0) {
        std::cerr << "Failed to build DATA packet.\n";
        freeaddrinfo(results);
        close(sock);
        return 4;
    }

    ssize_t sent = sendto(sock,
                          send_buf,
                          data_pkt_len,
                          0,
                          ptr->ai_addr,
                          ptr->ai_addrlen);

    if (sent < 0 ||  std::size_t(sent) != data_pkt_len) {
        std::perror("sendto (data)");
        freeaddrinfo(results);
        close(sock);
        return 5;
    }

    // --- Receive ACK ---

    uint8_t recv_buf[1024];
    struct sockaddr_storage from;
    socklen_t fromlen = sizeof(from);

    ssize_t recvd = recvfrom(sock,
                             recv_buf,
                             sizeof(recv_buf),
                             0,
                             reinterpret_cast<struct sockaddr*>(&from),
                             &fromlen);
    if (recvd < 0) {
        std::perror("recvfrom");
        freeaddrinfo(results);
        close(sock);
        return 6;
    }

    uint8_t ack_window = 0;
    uint8_t ack_seqnum = 0;
    if (!parse_and_verify_ack(recv_buf,
                               std::size_t(recvd),
                              ack_window,
                              ack_seqnum)) {
        std::cerr << "Failed to parse/verify ACK.\n";
        freeaddrinfo(results);
        close(sock);
        return 7;
    }

    // --- Send final zero-length DATA packet to signal end-of-transfer ---

    std::size_t fin_len = build_ctp_data_packet(
        send_buf,
        sizeof(send_buf),
        nullptr,
        0,            // Length
        ack_seqnum,
        1
    );

    if (fin_len == 0) {
        std::cerr << "Failed to build final DATA packet.\n";
        freeaddrinfo(results);
        close(sock);
        return 8;
    }

    sent = sendto(sock,
                  send_buf,
                  fin_len,
                  0,
                  ptr->ai_addr,
                  ptr->ai_addrlen);
    if (sent < 0 ||  std::size_t(sent) != fin_len) {
        std::perror("sendto (final)");
        freeaddrinfo(results);
        close(sock);
        return 9;
    }

    // Receive final ACK
    recvd = recvfrom(sock,
                     recv_buf,
                     sizeof(recv_buf),
                     0,
                     reinterpret_cast<struct sockaddr*>(&from),
                     &fromlen);

    if (recvd < 0) {
        std::perror("recvfrom (final ACK)");
        // we'll still close and exit; at worst receiver complains
    } else {
        uint8_t final_window = 0;
        uint8_t final_seq = 0;
        // optional: ignore failure, we just want to keep the port open
        parse_and_verify_ack(recv_buf,
                             std::size_t(recvd),
                             final_window,
                             final_seq);
    }

    freeaddrinfo(results);
    close(sock);

    return 0;
}