#include <cstdlib>
#include <cstring>

int main(int argc, char *argv[])
{
    if (argc != 3)
        return 1;

    char command[] = "mdadm --detail --scan ";

    strcat(command, argv[1]);

    strcat(command, " >> ");

    strcat(command, argv[2]);

    system(command);

    return 0;
}
