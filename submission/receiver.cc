// src/receiver.cc
// UDP receiver that echoes messages back to sender
// Lab N5 - Exercise 1

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [-4|-6] port\n"
              << "  -4    Use IPv4 only\n"
              << "  -6    Use IPv6 only\n"
              << "  port  Port number to listen on\n";
}

int main(int argc, char* argv[]) {
    int ip_version = AF_UNSPEC;  // default: both IPv4 and IPv6
    std::string port;

    // Parse command line arguments
    int arg_index = 1;

    if (arg_index < argc) {
        if (std::strcmp(argv[arg_index], "-4") == 0) {
            ip_version = AF_INET;
            std::cout << "[DEBUG] Using IPv4 only\n";
            arg_index++;
        } else if (std::strcmp(argv[arg_index], "-6") == 0) {
            ip_version = AF_INET6;
            std::cout << "[DEBUG] Using IPv6 only\n";
            arg_index++;
        }
    }

    if (arg_index >= argc) {
        print_usage(argv[0]);
        return 1;
    }

    port = argv[arg_index];
    std::cout << "[DEBUG] Receiver starting on port: " << port << std::endl;

    // Set up hints for getaddrinfo
    struct addrinfo hints;
    struct addrinfo* results = nullptr;
    struct addrinfo* rp = nullptr;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = ip_version;
    hints.ai_socktype = SOCK_DGRAM;  // UDP
    hints.ai_flags = AI_PASSIVE;

    std::cout << "[DEBUG] Calling getaddrinfo...\n";
    int rval = getaddrinfo(nullptr, port.c_str(), &hints, &results);
    if (rval != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(rval) << std::endl;
        return 2;
}

    std::cout << "[DEBUG] getaddrinfo returned address list\n";
    int sock = -1;

    // Try all results until one socket+bind succeeds
    for (rp = results; rp != nullptr; rp = rp->ai_next) {
    std::cout << "[DEBUG] Attempting socket()...\n";
    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sock == -1) 
    std::cerr << "[DEBUG] socket() failed, trying next...\n";
    continue;

    std::cout << "[DEBUG] Attempting bind()...\n";
    if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
         std::cout << "[DEBUG] bind() successful!\n";
         break;
    }

    std::cerr << "[DEBUG] bind() failed, trying next...\n";
    close(sock);
    sock = -1; // reset
}

freeaddrinfo(results);

if (sock == -1) {
    std::cerr << "[ERROR] Could not bind to ANY address\n";
    return 4;
}

std::cout << "Receiver listening on port " << port << std::endl;


    // Prepare buffer and address storage for receiving
    char buf[512];
    struct sockaddr_storage fromaddr;
    socklen_t fromlen = sizeof(fromaddr);
    ssize_t byte_count;


    std::cout << "[DEBUG] Waiting for a packet...\n";

    // Receive one packet
    byte_count = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                          (struct sockaddr*)&fromaddr, &fromlen);

    if (byte_count < 0) {
        std::perror("[ERROR] recvfrom");

        close(sock);
        return 5;
    }

    std::cout << "[DEBUG] Received a packet of " << byte_count << " bytes\n";


    // Null-terminate the received data (assuming it's a string)
    buf[byte_count] = '\0';
    std::cout << "Received: " << buf << std::endl;

    // Echo the message back to the sender
    std::cout << "[DEBUG] Echoing data back to sender...\n";
    ssize_t sent = sendto(sock, buf, byte_count, 0,
                          (struct sockaddr*)&fromaddr, fromlen);

    if (sent < 0) {
        std::perror("[ERROR] sendto");
        close(sock);
        return 6;
    }

    std::cout << "[DEBUG] Echo complete, exiting normally.\n";

    // Clean up
    close(sock);

    return 0;
}
