#include <cstdlib>
#include <string>
#include <iostream>

using namespace std;

using std::string;

int main(int argc, char *argv[])
{
    if (argc != 4)
        return 1;

    if (string(argv[1]) == "--append") {
        string command = "mdadm --detail --scan " + string(argv[2]) + " >> " + string(argv[3]);

        system(command.c_str());
    }
    else if (string(argv[1]) == "--write") {
        string command = "echo " + string(argv[2]) + " > " + string(argv[3]);

        system(command.c_str());
    }

    return 0;
}
