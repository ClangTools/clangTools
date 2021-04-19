//
// Created by Caesar on 2019/11/11.
//

#include <string>
#include <Logging.h>

using namespace std;

int main(int argc, char **argv) {
    Logging logging("logger_demo", "logger_demo.log", Logging::log_rank_t::log_rank_DEBUG, 10 * 1024, 5);

    for (int i = 0; i < 10000; i++) {
        logging.debug("%03d:%s", i, "DEBUG");
        logging.info("%03d:%s", i, "INFO");
        logging.warn("%03d:%s", i, "WARNING");
        logging.error("%03d:%s", i, "ERROR");
        logging.fatal("%03d:%s", i, "FATAL");
    }
    return 0;
}