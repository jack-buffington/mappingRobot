
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <string>
#include <iostream>
#include <memory.h> 
#include <array>
#include <memory>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <poll.h>   // TODO:  go through all of these and see which are still necessary


struct robotIP
{
	std::string IPaddress;
	int serialNumber;
};

// int sockfd; // Global so that I don't have to pass this to any function that involves communication with the robot 
// struct sockaddr_in     servaddr;  // Same here


// PORT is the port that UDP packets are sent to.  The port was randomly picked
//#define PORT     17980  
#define PORT     17980  // This is an incorrect port so that I can see what happens if it doesn't get a response
#define BUFFER_SIZE 1024 



int outgoingSockfd, incomingSockfd;  
struct sockaddr_in     outgoingAddr, incomingAddr;  
char buffer[BUFFER_SIZE];

int main(); 
std::string findMyIPaddress();
// std::vector<std::string> findAllRobots(std::string thisComputersIPaddress);

std::vector<robotIP> findAllRobots(std::string thisComputersIPaddress);
std::string exec(const char* cmd);
std::vector<std::string> split_string(const std::string& str, const std::string& delimiter);
void setupOutgoingUDP();
void setupIncomingUDP();
void checkForUpdates();
std::string stripNewline(std::string theString);
bool recvFromWithTimeout(int sockFD, sockaddr_in* sockfrom, void *buffer, unsigned int bufferLength, int timeoutInMicroseconds);