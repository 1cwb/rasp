#include <iostream>
#include "config.h"
#include "common.h"

using namespace std;
using namespace rasp;
int main(int argc, char** argv)
{

    Config mconfig;
    if(mconfig.parse("test.ini") < 0)
    {
        cout << "open file fail"<< endl;
    }
    mconfig.dump();
    cout << "======================" << endl;
    return 0;
}
