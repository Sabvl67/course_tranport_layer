// src/simple_sender.cc
// Simple UDP sender that sends a message and receives echo
// Lab N5 - Exercise 1

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " destination_host destination_port\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* port = argv[2];

    // Set up hints for getaddrinfo
    struct addrinfo hints;
    struct addrinfo* results = nullptr;
    struct addrinfo* ptr = nullptr;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // Get address info for destination
    int rval = getaddrinfo(host, port, &hints, &results);
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

    // Read message from user
    std::cout << "Enter a message: ";
    std::string message;
    std::getline(std::cin, message);

    // Send message to receiver
    ssize_t sent = sendto(sock, message.c_str(), message.size(), 0,
                          ptr->ai_addr, ptr->ai_addrlen);

    if (sent < 0) {
        std::perror("sendto");
        freeaddrinfo(results);
        close(sock);
        return 4;
    }

    // Receive echo response
    char buf[512];
    struct sockaddr_storage fromaddr;
    socklen_t fromlen = sizeof(fromaddr);

    ssize_t byte_count = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                                   (struct sockaddr*)&fromaddr, &fromlen);

    if (byte_count < 0) {
        std::perror("recvfrom");
        freeaddrinfo(results);
        close(sock);
        return 5;
    }

    // Null-terminate and print received message
    buf[byte_count] = '\0';
    std::cout << "Received: " << buf << std::endl;

    // Clean up
    freeaddrinfo(results);
    close(sock);

    return 0;
}
