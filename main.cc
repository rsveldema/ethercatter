#include "ethercat.h"


int main()
{
    try
    {
        EtherCat ethercat;

        while (1)
        {
            ethercat.process_packet();
        }
    }
    catch (std::runtime_error &e)
    {
        std::cerr << "failed: " << e.what() << std::endl;
    }
    return 0;
}