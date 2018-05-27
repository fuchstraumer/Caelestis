#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest/doctest.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv) {
    doctest::Context context;
    context.setOption("abort-after", 5);
    context.applyCommandLine(argc, argv);
    context.run();
}