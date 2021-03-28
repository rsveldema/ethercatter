#include "ethercat.h"


int main(int argc, char **argv)
{
    bool master = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-master") == 0) {
            master = true;
        } else {
            fprintf(stderr, "unknown arg: %s\n", argv[i]);
            exit(1);
        }
    }

    try
    {
        EtherCat ethercat;
        if (master)
        {
            ethercat.master();
        } else {
            ethercat.slave();
        }
    }
    catch (std::runtime_error &e)
    {
        std::cerr << "failed: " << e.what() << std::endl;
    }
    return 0;
}