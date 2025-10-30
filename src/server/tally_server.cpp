#include <thread>

#include "gms/log.h"
#include "gms/cuda_util.h"
#include "gms/generated/server.h"

int main(int argc, char ** argv) {

    register_cuda_specs();

    // Main server thread, will spawn one server thread for each client
    std::thread main_server_t(&GmsServer::start_main_server, GmsServer::server);

    // Kernel scheduler, scheduling kernel launches from all clients
    std::thread scheduler_t(&GmsServer::start_scheduler, GmsServer::server);

    main_server_t.join();
    scheduler_t.join();

    GMS_SPD_LOG_ALWAYS("Gms server shutting down ...");

    return 0;
}