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
            arg_index++;
        } else if (std::strcmp(argv[arg_index], "-6") == 0) {
            ip_version = AF_INET6;
            arg_index++;
        }
    }

    if (arg_index >= argc) {
        print_usage(argv[0]);
        return 1;
    }

    port = argv[arg_index];

    // Set up hints for getaddrinfo
    struct addrinfo hints;
    struct addrinfo* results = nullptr;
    struct addrinfo* ptr = nullptr;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = ip_version;
    hints.ai_socktype = SOCK_DGRAM;  // UDP
    hints.ai_flags = 0x01;  // Passive flag - get local address for binding

    // Get address info for binding (NULL hostname = local host)
    int rval = getaddrinfo(nullptr, port.c_str(), &hints, &results);
    if (rval != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(rval) << std::endl;
        return 2;
    }

    // Try to open socket from the results
    int sock = -1;
    for (ptr = results; ptr != nullptr; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1) {
            break;  // success
        }
    }

    if (sock == -1 || ptr == nullptr) {
        std::cerr << "Failed to open socket" << std::endl;
        freeaddrinfo(results);
        return 3;
    }

    // Bind the socket to the local address and port
    if (bind(sock, ptr->ai_addr, ptr->ai_addrlen) == -1) {
        std::cerr << "Cannot bind the socket" << std::endl;
        freeaddrinfo(results);
        close(sock);
        return 4;
    }

    std::cout << "Receiver listening on port " << port << std::endl;

    // Prepare buffer and address storage for receiving
    char buf[512];
    struct sockaddr_storage fromaddr;
    socklen_t fromlen;
    ssize_t byte_count;

    // Receive one packet
    fromlen = sizeof(fromaddr);
    byte_count = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                          (struct sockaddr*)&fromaddr, &fromlen);

    if (byte_count < 0) {
        std::perror("recvfrom");
        freeaddrinfo(results);
        close(sock);
        return 5;
    }

    // Null-terminate the received data (assuming it's a string)
    buf[byte_count] = '\0';
    std::cout << "Received: " << buf << std::endl;

    // Echo the message back to the sender
    ssize_t sent = sendto(sock, buf, byte_count, 0,
                          (struct sockaddr*)&fromaddr, fromlen);

    if (sent < 0) {
        std::perror("sendto");
        freeaddrinfo(results);
        close(sock);
        return 6;
    }

    // Clean up
    freeaddrinfo(results);
    close(sock);

    return 0;
}
