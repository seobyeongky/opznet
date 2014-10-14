#include "common.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

namespace opznet
{
    
    bool GetBroadcastAddress(list<sf::IpAddress> *address_list)
    {
        struct ifaddrs *ifap, *ifa;
        struct sockaddr_in *sa;
        char *addr;
        
        getifaddrs (&ifap);
        for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr->sa_family==AF_INET) {
                sa = (struct sockaddr_in *) ifa->ifa_addr;
                addr = inet_ntoa(sa->sin_addr);
                printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
            }
        }
        
        freeifaddrs(ifap);
        return 0;
    }
    
    
}