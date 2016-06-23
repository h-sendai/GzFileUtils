#include <cstdio>
#include <fstream>
#include <iostream>

#include "GzFileUtils.h"

int main(int argc, char *argv[])
{
    GzFileUtils *g = new GzFileUtils::GzFileUtils();
    
    g->open_file("/tmp/abc");
    g->set_max_size_in_megaBytes(1);
    
    char buf[1025]; // 1024 + 1 to examine file branches
    for (int i = 0; i < 10000; ++i) {
        g->write_data(buf, sizeof(buf));
    }

    g->close_file();

    delete g;
    return 0;
}
