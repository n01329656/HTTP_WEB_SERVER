

#include <iostream>
#include "HTTP_Server.h"


int main()
{
    try {
        Server myserver;
        myserver.Start(2500,10);
        std::this_thread::sleep_for(std::chrono::seconds(60));
        myserver.Stop();
    }
    catch (const std::system_error& er) {
        std::cout << er.what();
        return -1;
    }
    return 0;
}


