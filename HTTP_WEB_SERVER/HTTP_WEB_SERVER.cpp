

#include <iostream>
#include "HTTP_Server.h"
#include "HTTP_Client.h"

int main()
{

    std::thread m([]()
        {
            try {
                Client c;
                std::string ip = "127.0.0.1";
                std::string res = "\\index.html";
                std::this_thread::sleep_for(std::chrono::seconds(5));
                std::cout << "starting session...\n";
                for (int i = 0; i < 10; i++)  c.StartSession(i,2500,ip,res);
                c.finish();
            }
            catch (const std::exception & er) {
                std::cout <<"Exception has been thrown: " << er.what() << "\n";
            }
        });


    try {
        Server myserver;
        std::cout << "starting the server...\n";
        myserver.Start(2500,10);
        std::this_thread::sleep_for(std::chrono::seconds(60));
        myserver.Stop();
    }
    catch (const std::system_error& er) {
        std::cout << er.what();
        return -1;
    }

   
    m.join();

    return 0;
}


