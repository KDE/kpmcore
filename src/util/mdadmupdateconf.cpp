#include <cstdlib>
#include <string>

using std::string;

int main(int argc, char *argv[])
{
    if (argc != 3)
        return 1;

    string command = "mdadm --detail --scan " + string(argv[1]) + " >> " + string(argv[2]);

    system(command.c_str());

    return 0;
}
