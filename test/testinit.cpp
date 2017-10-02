#include "helpers.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        KPMCoreInitializer i;
        return i.isValid() ? 0 : 1;
    }
    else
    {
        KPMCoreInitializer i(argv[1]);
        return i.isValid() ? 0 : 1;
    }
}

