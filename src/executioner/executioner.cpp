#include "MVRE/executioner/executioner.hpp"

using namespace mvre_executioner;

executioner_worker* executioner::worker = nullptr;

void executioner::init() {
    worker = new executioner_worker();
}

void executioner::execute() {
    if (worker == nullptr)
        return;

    worker->execute();

    std::mutex mtx;
    std::unique_lock<std::mutex> l(mtx);
    worker->wait_room.wait(l, [&] { return !worker->executing(); });
}

void executioner::clean() {
    worker->stop();
    delete worker;
}
