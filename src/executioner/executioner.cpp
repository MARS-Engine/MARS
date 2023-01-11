#include "MARS/executioner/executioner.hpp"

using namespace mars_executioner;

executioner_worker* executioner::worker = nullptr;

void executioner::init() {
    worker = new executioner_worker();
}

void executioner::execute() {
    if (worker == nullptr)
        return;

    worker->execute();
}

void executioner::clean() {
    worker->stop();
    delete worker;
}
