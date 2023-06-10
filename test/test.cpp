#include <windows.h>

extern "C" PVOID __stdcall QueryInterface();
int main()
{
    QueryInterface();
    return 0;
}
