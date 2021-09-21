#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::milliseconds
#include <iostream>
  
#define PORT     17980 
#define BUFFER_SIZE 1024 

int outgoingSockfd, incomingSockfd;  
struct sockaddr_in     outgoingAddr, incomingAddr;  
  

int main();
void setupOutgoingUDP(std::string controlComputerIPaddress);
void setupIncomingUDP();