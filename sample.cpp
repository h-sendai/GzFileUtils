#include <cstdio>
#include <fstream>
#include <iostream>

#include "GzFileUtils.h"

int main(int argc, char *argv[])
{
    GzFileUtils *g = new GzFileUtils::GzFileUtils();
    
    g->open_file("/tmp/abc");
    
    char buf[1024];
    g->write_data(buf, sizeof(buf));


    g->close_file();

    delete g;
    return 0;
}
