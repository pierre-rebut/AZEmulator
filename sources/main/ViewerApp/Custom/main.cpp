//
// Created by pierr on 15/03/2023.
//

#include "ViewerApplication.h"

#include "Commons/Log.h"
#include "Commons/Profiling.h"
#include "Commons/utils/RandomGenerator.h"

int main() {
    LOG_INIT();

    try {
        Astra::RandomGenerator rng;

        Astra::UI::App::ViewerApplication app;
        app.run();
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
        return EXIT_FAILURE;
    }

    LOG_INFO("Exit");
    return EXIT_SUCCESS;
}
