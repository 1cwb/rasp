#include <iostream>
#include "config.h"
#include "common.h"
#include "util.h"
#include <chrono>
#include "mlog.h"

using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{
    Logger& mlog = Logger::getLogger();
    mlog.setFileName("/home/rock/12345.log");
    while(true)
    error("hello world");
    return 0;
}
