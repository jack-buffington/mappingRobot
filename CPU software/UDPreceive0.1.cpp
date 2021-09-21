 
// Code is based on the reference at  https://www.geeksforgeeks.org/udp-server-client-implementation-c/

// compile with    g++ UDPreceive.cpp -o UDPreceive -std=c++11

// I'm not seeing these packets with wireshark!



  


#include "UDPreceive.h"

#define SERIAL_NUMBER   1



int main() { 
    char buffer[BUFFER_SIZE]; 

    setupIncomingUDP();
      
     

    /*
    ssize_t recv(int incomingSockfd, void *buf, size_t len, int flags);
    ssize_t recvfrom(int incomingSockfd, void *buf, size_t len,   int flags,   struct sockaddr *src_addr,   socklen_t *addrlen); <--- Using this one
    ssize_t recvmsg(int incomingSockfd, struct msghdr *msg, int flags);
    */
  unsigned int len, n;
  while(1)
   {
    n = recvfrom(incomingSockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &incomingAddr, &len);
    // n = recvfrom(incomingSockfd, (char *)buffer, BUFFER_SIZE,  MSG_WAITALL, (struct sockaddr *) &cliaddr, &len); 
    buffer[n] = '\0'; 
    printf("Client : %s\n", buffer); 


    if(buffer[0] == 1)
    {
      std::cout << "This appears to be a hello message." << std::endl;
      std::string remoteIP = buffer;
      remoteIP = remoteIP.substr(1,remoteIP.length());
      std::cout << "The remote IP address is " << remoteIP << std::endl;

      // Send a return message to the remote computer
      setupOutgoingUDP(remoteIP);
      char hello[2] = {1, SERIAL_NUMBER};
      //sendto(outgoingSockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &outgoingAddr,  sizeof(outgoingAddr));
      sendto(outgoingSockfd, (const char *)hello, 2, MSG_CONFIRM, (const struct sockaddr *) &outgoingAddr,  sizeof(outgoingAddr));

    }


    std::cout << "Data that is in incomingAddr:" << std::endl;
    std::cout << "  sin_family:" << incomingAddr.sin_family << std::endl;  // 0
    std::cout << "  sin_port:" << incomingAddr.sin_port << std::endl;      // 0

    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(incomingAddr.sin_addr), str, INET_ADDRSTRLEN);
    std::cout << "  sin_addr:" << str << std::endl;                   // 0.0.0.0


    std::string address = inet_ntoa(incomingAddr.sin_addr);
    std::cout <<  "Packet was received from: " << address << std::endl;
   }







    // sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 



/*
    std::cout << "Enter something to send a message." << std::endl;
    std::string tempString;

    while(1)
    {
      // pause for a bit 
      sleep(.5);
      sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr)); 
      printf("Hello message sent to client.\n");     
    }
*/
      
    return 0; 
} 




void setupOutgoingUDP(std::string controlComputerIPaddress)
{
    // Creating socket file descriptor 
    if ( (outgoingSockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) // IP4, UDP, IP protocol
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    
    memset(&outgoingAddr, 0, sizeof(outgoingAddr)); 
    outgoingAddr.sin_family = AF_INET;  // This is always SF_INET
    outgoingAddr.sin_port = htons(PORT); 
    //servaddr.sin_addr.s_addr = INADDR_ANY; // "any address for binding"??? // Uncomment to make this work on the local machine again
    inet_aton(controlComputerIPaddress.c_str(), &outgoingAddr.sin_addr);
}


void setupIncomingUDP()
{
  if ( (incomingSockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
  { 
      perror("socket creation failed"); 
      exit(EXIT_FAILURE); 
  } 
    
  memset(&incomingAddr, 0, sizeof(incomingAddr)); 
  //memset(&outgoingAddr, 0, sizeof(outgoingAddr)); 

  incomingAddr.sin_family    = AF_INET; // IPv4 
  incomingAddr.sin_addr.s_addr = INADDR_ANY; 
  incomingAddr.sin_port = htons(PORT); 

  if ( bind(incomingSockfd, (const struct sockaddr *)&incomingAddr, sizeof(incomingAddr)) < 0 ) 
  { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
  } 

}