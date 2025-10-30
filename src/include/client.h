#ifndef GMS_CLIENT_Htally
#define GMS_CLIENT_H

#include <signal.h>
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <functional>
#include <iostream>
#include <cassert>
#include <sstream>
#include <unistd.h>

#include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/detail/unique_id.hpp"

#include "gms/msg_struct.h"

#include <dlfcn.h>

extern cudaError_t LAST_CUDA_ERR;
extern bool REPLACE_CUBLAS;

class GmsClient {

public:

    static GmsClient *client;
    int32_t client_id;

    //add3
    int mapped_id;
    std::string m_id_str = "";
    
    bool has_connected = false;

    std::recursive_mutex iox_mtx;

    std::map<const void *, std::string> host_func_to_demangled_kernel_name_map;
    std::map<std::string, std::vector<uint32_t>> _kernel_name_to_args;

    std::unordered_map<const void *, std::vector<uint32_t>> _kernel_addr_to_args;
    std::unordered_map<CUfunction, std::vector<uint32_t>> _jit_kernel_addr_to_args;

    iox::popo::UntypedClient *iox_client;

    GmsClient() :
        client_id(getpid())
    {
        mapped_id = -1;
    }

    ~GmsClient(){}

    


    void setM_ID(const std::string& full_file_path) {
        size_t start_pos = full_file_path.find("ggml-org_gemma-3-");

        if (start_pos == std::string::npos) {
            std::cerr << "Error: Could not find 'ggml-org_gemma-3-' prefix in path: " << full_file_path << std::endl;
            m_id_str = "";
            mapped_id = -1;
            return;
        }
        
        start_pos += std::string("ggml-org_gemma-3-").length();


        size_t end_pos = full_file_path.find("b-it-GGUF_", start_pos);
        if (end_pos == std::string::npos) {
            std::cerr << "Error: Could not find 'b-it-GGUF' suffix in path: " << full_file_path << std::endl;
            m_id_str = "";
            mapped_id = -1;
            return;
        }

        // Extract the substring between "file_" and ".log"
        m_id_str = full_file_path.substr(start_pos, end_pos - start_pos);

        try {
            // Attempt to convert the string MID to an integer
            mapped_id = std::stoi(m_id_str); // ggml-org_gemma-3-1
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid MID format '" << m_id_str << "'. Not a valid integer. " << e.what() << std::endl;
            mapped_id = -1; // Indicate error
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: MID '" << m_id_str << "' out of integer range. " << e.what() << std::endl;
            mapped_id = -1; // Indicate error
        }

        std::cout << "set client model id to: " << mapped_id << std::endl;
        // exit(1);
    }

    // void process_args(int argc, char** argv) {
    //     std::cerr << "GmsClient: Processing arguments from __libc_start_main interception." << std::endl;
    //     for (int i = 1; i < argc; ++i) {
    //         std::string arg = argv[i];
    //         if (arg == "-m") {
    //             if (i + 1 < argc) {
    //                 std::string path_mid_arg = argv[++i];
    //                 this->setM_ID(path_mid_arg);
    //                 // Perform your parsing logic here (e.g., extractAndSetM_ID)
    //                 // For simplicity, just store the path for now
    //                 std::cerr << "GmsClient: Found -m with path: " << path_mid_arg << std::endl;
    //             }
    //         }
    //     }
    //     std::cout << "GmsClient: set model id " << mapped_id << std::endl;
    //     exit(1);
    // }


    void connect_to_server()
    {
        if (!has_connected) {
            int32_t priority = std::getenv("PRIORITY") ? std::stoi(std::getenv("PRIORITY")) : 1;

            auto app_name_str_base = std::string("gms-client-app");
            auto app_name_str = app_name_str_base + std::to_string(client_id);

            char APP_NAME[100];
            strcpy(APP_NAME, app_name_str.c_str()); 

            iox::runtime::PoshRuntime::initRuntime(APP_NAME);

            iox::popo::UntypedClient client_handshake({"Gms", "handshake", "event"});

            // Send handshake to server
            client_handshake.loan(sizeof(HandshakeMessgae), alignof(HandshakeMessgae))
                .and_then([&](auto& requestPayload) {

                    auto request = static_cast<HandshakeMessgae*>(requestPayload);
                    request->header.client_id = client_id;
                    request->client_id = client_id;
                    request->mapped_id = mapped_id;
                    request->priority = priority;

                    client_handshake.send(request).or_else(
                        [&](auto& error) { std::cout << "Could not send Request! Error: " << error << std::endl; });
                })
                .or_else([](auto& error) { std::cout << "Could not allocate Request! Error: " << error << std::endl; });

            while (!client_handshake.take().and_then([&](const auto& responsePayload) {

                auto response = static_cast<const HandshakeResponse*>(responsePayload);
                
                bool success = response->success;
                if (!success) {
                    std::cout << "Handshake with gms server failed. Exiting ..." << std::endl;
                    exit(1);
                }

                client_handshake.releaseResponse(responsePayload);

            })) {};

            // auto channel_desc_str = std::string("Gms-Communication") + std::to_string(client_id);
            auto channel_desc_str = std::string("Gms-Main") + std::to_string(mapped_id);
            char channel_desc[100];
            strcpy(channel_desc, channel_desc_str.c_str()); 
            iox_client = new iox::popo::UntypedClient({channel_desc, "gms", "gms"});
        }

        has_connected = true;
    }
};

#endif // GMS_CLIENT_H