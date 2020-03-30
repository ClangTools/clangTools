#include <iostream> /* cout */
#include <unistd.h>/* gethostname */
#include <netdb.h> /* struct hostent */
#include <arpa/inet.h> /* inet_ntop */

bool GetHostInfo(std::string& hostName, std::string& Ip) {
    char name[256];
    gethostname(name, sizeof(name));
    hostName = name;

    struct hostent* host = gethostbyname(name);
    char ipStr[32];
    const char* ret = inet_ntop(host->h_addrtype, host->h_addr_list[0], ipStr, sizeof(ipStr));
    if (NULL==ret) {
        std::cout << "hostname transform to ip failed";
        return false;
    }
    Ip = ipStr;
    return true;
}
//以下测试
int main(int argc, char *argv[]) {
    std::string hostName;
    std::string Ip;

    bool ret = GetHostInfo(hostName, Ip);
    if (true == ret) {
        std::cout << "hostname: " << hostName << std::endl;
        std::cout << "Ip: " << Ip << std::endl;
    }
    return 0;
}
