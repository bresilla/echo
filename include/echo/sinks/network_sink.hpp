#pragma once

/**
 * @file sinks/network_sink.hpp
 * @brief Network output sink (TCP/UDP)
 *
 * Only available when compiled with -DECHO_ENABLE_NETWORK_SINK
 */

#include <echo/sinks/sink.hpp>

#include <chrono>
#include <mutex>
#include <queue>
#include <string>

// Platform-specific socket includes
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace echo {

    /**
     * @brief Network protocol type
     */
    enum class NetworkProtocol { TCP, UDP };

    /**
     * @brief Network sink - sends logs over TCP or UDP
     *
     * Features:
     * - TCP or UDP transport
     * - Automatic reconnection on failure
     * - Message buffering when disconnected
     * - Non-blocking sends
     * - ANSI code stripping
     *
     * Example:
     *   auto net = std::make_shared<NetworkSink>("localhost", 5140, NetworkProtocol::TCP);
     *   net->set_buffer_size(1000);  // Buffer up to 1000 messages
     *   echo::add_sink(net);
     */
    class NetworkSink : public Sink {
      private:
        std::string host_;
        int port_;
        NetworkProtocol protocol_;
        int socket_ = -1;
        bool connected_ = false;
        mutable std::mutex mutex_;
        std::queue<std::string> buffer_;
        size_t max_buffer_size_ = 100;
        std::chrono::steady_clock::time_point last_connect_attempt_;
        std::chrono::seconds reconnect_interval_{5};

#ifdef _WIN32
        static bool winsock_initialized_;
        static std::mutex winsock_mutex_;
#endif

        /**
         * @brief Initialize platform-specific networking
         */
        void init_networking() {
#ifdef _WIN32
            std::lock_guard<std::mutex> lock(winsock_mutex_);
            if (!winsock_initialized_) {
                WSADATA wsa_data;
                WSAStartup(MAKEWORD(2, 2), &wsa_data);
                winsock_initialized_ = true;
            }
#endif
        }

        /**
         * @brief Close socket
         */
        void close_socket() {
            if (socket_ != -1) {
#ifdef _WIN32
                closesocket(socket_);
#else
                close(socket_);
#endif
                socket_ = -1;
            }
            connected_ = false;
        }

        /**
         * @brief Attempt to connect to remote host
         * @return true if connected
         */
        bool connect() {
            // Rate limit connection attempts
            auto now = std::chrono::steady_clock::now();
            if (now - last_connect_attempt_ < reconnect_interval_) {
                return false;
            }
            last_connect_attempt_ = now;

            close_socket();

            // Resolve hostname
            struct addrinfo hints = {}, *result = nullptr;
            hints.ai_family = AF_INET;
            hints.ai_socktype = (protocol_ == NetworkProtocol::TCP) ? SOCK_STREAM : SOCK_DGRAM;

            if (getaddrinfo(host_.c_str(), std::to_string(port_).c_str(), &hints, &result) != 0) {
                return false;
            }

            // Create socket
            socket_ = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if (socket_ == -1) {
                freeaddrinfo(result);
                return false;
            }

            // Connect (TCP only)
            if (protocol_ == NetworkProtocol::TCP) {
                if (::connect(socket_, result->ai_addr, result->ai_addrlen) != 0) {
                    close_socket();
                    freeaddrinfo(result);
                    return false;
                }
            }

            freeaddrinfo(result);
            connected_ = true;
            return true;
        }

        /**
         * @brief Send message over network
         * @param message Message to send
         * @return true if sent successfully
         */
        bool send_message(const std::string &message) {
            if (!connected_ && !connect()) {
                return false;
            }

            ssize_t sent = send(socket_, message.c_str(), message.size(), 0);
            if (sent < 0) {
                close_socket();
                return false;
            }

            return true;
        }

        /**
         * @brief Strip ANSI escape codes from string
         */
        std::string strip_ansi(const std::string &str) const {
            std::string result;
            result.reserve(str.size());

            bool in_escape = false;
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[') {
                    in_escape = true;
                    ++i;
                    continue;
                }
                if (in_escape) {
                    if (str[i] == 'm') {
                        in_escape = false;
                    }
                    continue;
                }
                result += str[i];
            }
            return result;
        }

        /**
         * @brief Flush buffered messages
         */
        void flush_buffer() {
            while (!buffer_.empty() && connected_) {
                if (send_message(buffer_.front())) {
                    buffer_.pop();
                } else {
                    break;
                }
            }
        }

      public:
        /**
         * @brief Construct a network sink
         * @param host Remote hostname or IP
         * @param port Remote port
         * @param protocol TCP or UDP
         */
        NetworkSink(const std::string &host, int port, NetworkProtocol protocol = NetworkProtocol::TCP)
            : host_(host), port_(port), protocol_(protocol) {
            init_networking();
            last_connect_attempt_ = std::chrono::steady_clock::now() - reconnect_interval_;
        }

        ~NetworkSink() override { close_socket(); }

        // Prevent copying
        NetworkSink(const NetworkSink &) = delete;
        NetworkSink &operator=(const NetworkSink &) = delete;

        /**
         * @brief Write message to network
         * @param level Log level
         * @param message Formatted message
         */
        void write(Level level, const std::string &message) override {
            if (!should_log(level)) {
                return;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            std::string clean_message = strip_ansi(message);

            // Try to send immediately
            if (send_message(clean_message)) {
                // Success, try to flush any buffered messages
                flush_buffer();
            } else {
                // Failed, buffer the message
                if (buffer_.size() < max_buffer_size_) {
                    buffer_.push(clean_message);
                }
                // If buffer is full, oldest messages are dropped
            }
        }

        /**
         * @brief Flush any buffered messages
         */
        void flush() override {
            std::lock_guard<std::mutex> lock(mutex_);
            flush_buffer();
        }

        /**
         * @brief Set maximum buffer size
         * @param size Maximum number of messages to buffer
         */
        void set_buffer_size(size_t size) {
            std::lock_guard<std::mutex> lock(mutex_);
            max_buffer_size_ = size;
        }

        /**
         * @brief Set reconnection interval
         * @param seconds Seconds between reconnection attempts
         */
        void set_reconnect_interval(int seconds) {
            std::lock_guard<std::mutex> lock(mutex_);
            reconnect_interval_ = std::chrono::seconds(seconds);
        }

        /**
         * @brief Check if currently connected
         * @return true if connected
         */
        [[nodiscard]] bool is_connected() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return connected_;
        }

        /**
         * @brief Get number of buffered messages
         * @return Buffer size
         */
        [[nodiscard]] size_t get_buffer_count() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return buffer_.size();
        }
    };

#ifdef _WIN32
    bool NetworkSink::winsock_initialized_ = false;
    std::mutex NetworkSink::winsock_mutex_;
#endif

} // namespace echo
