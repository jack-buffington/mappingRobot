// Client side implementation of UDP client-server model 
// Code is from https://www.geeksforgeeks.org/udp-server-client-implementation-c/

// compile with g++ UDPsend3.cpp -o UDPsend3 -std=c++11 


#include "UDPsend3.h"
  


int main() 
{ 

     
  
    setupOutgoingUDP();
    setupIncomingUDP();

    checkForUpdates();

    std::string thisComputersIPaddress = findMyIPaddress();
    std::vector<std::string> IPaddressesOfTheRobots = findAllRobots(thisComputersIPaddress);
      
    
    while(1)
    {
        char hello[] = "Sent from control program";
        sendto(outgoingSockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &outgoingAddr,  sizeof(outgoingAddr));  
        sendto(outgoingSockfd, thisComputersIPaddress.c_str(), thisComputersIPaddress.length(), MSG_CONFIRM, (const struct sockaddr *) &outgoingAddr,  sizeof(outgoingAddr) );  

        printf("message sent.\n"); 
        sleep(1);
    }
} 


void setupOutgoingUDP()
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
    inet_aton("192.168.1.143", &outgoingAddr.sin_addr);
}



std::string findMyIPaddress()
{ // Finds this computer's IP address through a system call
    std::string thisIP = exec("hostname -I");
    std::cout << "This computer's IP address is " << thisIP << "." << std::endl;
    return thisIP;
}




std::vector<std::string> findAllRobots(std::string thisComputersIPaddress)
{ // Returns the IP address of all robots that it finds within the IP range hard coded into this function
    int startAddress = 2;
    int endAddress = 255;

    std::vector<std::string> returnVector;

    // Modify the IP address so that it can be part of a range.
    std::size_t trimPoint = thisComputersIPaddress.find_last_of(".");
    std::string IPrange = thisComputersIPaddress.substr(0,trimPoint);
    IPrange.append(".");
    IPrange.append(std::to_string(startAddress));
    IPrange.append("-");
    IPrange.append(std::to_string(endAddress));

    //std::cout << "IPrange: " << IPrange << std::endl;


    std::string commandString = "nmap -sP " + IPrange + " | grep for | awk '{print $NF}'";

    std::cout << "command string: " << commandString << std::endl;  


    std::cout << "Searching for the robot on this network"  << std::endl;
  //  nmap -sP 192.168.1.140-150 | grep for | awk '{print $5}'

    // std::string computersInRange = exec("nmap -sP 192.168.1.1-150 | grep for | awk '{print $5}'");

    std::string computersInRange = exec(commandString.c_str());

    // Split the string into a vector of IP addresses
    std::vector<std::string> IPaddresses = split_string(computersInRange, "\n");


    // Optionally strip away the parentheses
    for(unsigned int I = 0; I < IPaddresses.size() - 1; I++)
    {
      if(IPaddresses[I].at(0) == '(' )
        IPaddresses[I].erase(0,1);
      if(IPaddresses[I].at(IPaddresses[I].length()-1) == ')' )
        IPaddresses[I].erase(IPaddresses[I].length()-1,1);
    }

    // Print out all of the IP addresses that were found
    std::cout << "Found the following computers on the network: " << std::endl;
    for(unsigned int I = 0; I < IPaddresses.size() - 1; I++)
    {
        std::cout << "#" << I << ": " << IPaddresses[I] << std::endl; 
    }

    // Contact each IP address and see if it receives a reply from a robot

    // std::string helloString = "hello\x00\x01\x02\x03\x04\x05\xFF ";  // \x<hex code> allows me to enter byte values
    // helloString += thisComputersIPaddress;


    // The method below seems to be the best way to construct these messages, or at least the ones that aren't pure binary.
    // I can't explicitly put a 0x00 in there because the compiler will complain so I can sneak it in there later.  As long as I tell 
    // sendto the correct number of characters, everything will work.   strlen will see the 0 and assume that it is a terminating character.

    char test2[100];
    int n = sprintf(test2, " %s", thisComputersIPaddress.c_str()); // n contains how many characters were printed out
    test2[0] = 1;
    unsigned int len;

    for(unsigned int I = 0; I < IPaddresses.size() - 1; I++)
    {
        std::cout << "Querying IP address #" << I << ": " << IPaddresses[I] << std::endl;
        inet_aton(IPaddresses[I].c_str(), &outgoingAddr.sin_addr);
        //sendto(sockfd, (const char *)helloString.c_str(), helloString.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
        sendto(outgoingSockfd, (const char *)test2, n, MSG_CONFIRM, (const struct sockaddr *) &outgoingAddr,  sizeof(outgoingAddr));

        n = recvfrom(incomingSockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &incomingAddr, &len);
        std::cout << "Bytes received: " << n << std::endl;

        if(bytesReceived != -1 && buffer[0] == 1)
            std::cout << "This is a hello packet for serial number " << buffer[1] + 0 << std::endl;
        else
            std::cout << "This wasn't a hello packet." <<  std::endl;

    }
    return returnVector;
}




std::string exec(const char* cmd) 
{ // returns the result of a system call

   #ifdef CAMERA_CLASS_DEBUGGING_ON
   std::cout << "starting " << __func__ << std::endl;
   #endif
   
   std::array<char, 128> buffer;
   std::string result;
   std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
   if (!pipe) 
   {
      throw std::runtime_error("popen() failed!");
   }
   while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) 
   {
      result += buffer.data();
   }
   return result;
}





std::vector<std::string> split_string(const std::string& str, const std::string& delimiter)
{ // splits a string into multiple strings based on a delimiter
   #ifdef CAMERA_CLASS_DEBUGGING_ON
   std::cout << "starting " << __func__ << std::endl;
   #endif
   
   std::vector<std::string> strings;

   std::string::size_type pos = 0;
   std::string::size_type prev = 0;
   while ((pos = str.find(delimiter, prev)) != std::string::npos)
   {
     strings.push_back(str.substr(prev, pos - prev));
     prev = pos + 1;
   }

   // To get the last substring (or only, if delimiter is not found)
   strings.push_back(str.substr(prev));

   return strings;
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


  // Set the timeout for not receiving a packet at 500 milliseconds.  
  //unsigned int timeoutInMilliseconds = 500;

  struct timeval timeoutDuration;
  timeoutDuration.tv_sec=0;
  timeoutDuration.tv_usec=500000; // 1/2 second


  //socklen_t timeoutSize;

  //int a = setsockopt(incomingSockfd, SOL_SOCKET, SO_RCVTIMEO, &timeoutInMilliseconds, sizeof(timeoutInMilliseconds));
  int a = setsockopt(incomingSockfd, SOL_SOCKET, SO_RCVTIMEO, &timeoutDuration, sizeof(timeoutDuration));

  std::cout << "return value when setting the timeout: " << a << std::endl;

  if(a == -1)
  {
    std::cout << "There was an error: " << errno << std::endl;
      switch(errno)
        {
          case(EBADF):
            std::cout << "The argument sockfd is not a valid descriptor." << std::endl;
            break;
          case(EFAULT):
            std::cout << "The address pointed to by optval is not in a valid part of the process address space." << std::endl;
            break; 
          case(EINVAL):
            std::cout << "optlen invalid in setsockopt(). " << std::endl;   // <-----   This is the one that I am having an error with
            break; 
          case(ENOPROTOOPT):
            std::cout << "The option is unknown at the level indicated." << std::endl;
            break; 
          case(ENOTSOCK):
            std::cout << "The argument sockfd is a file, not a socket." << std::endl;
            break; 
        }
  }
}




void checkForUpdates()
{ // This function looks for a USB drive named ROBOHELPER and then looks for a file called wifi_config.  It will then add any 
  // networks that it finds to  /etc/wpa_supplicant/wpa_supplicant.conf so that the robot can connect to the network and be found.

    std::string username = exec("whoami");
    std::cout << "The username is " << username << std::endl;

    // remove the new line character from the username
    username = stripNewline(username);
    std::cout << "The username is " << username << std::endl;

    std::string thePath = "/media/";
    thePath += username;

    std::string commandString = "ls ";
    commandString += thePath;
    commandString += " | grep ROBOHELPER | wc -l";

    std::cout << "The command string is: " << commandString << std::endl;

    std::string isItPresent = exec(commandString.c_str());
    isItPresent = stripNewline(isItPresent);

    std::cout << "Is it present: " << isItPresent << std::endl;

    if(isItPresent == "1")
    {
        std::cout << "Found ROBOHELPER!" << std::endl;
        thePath += "/ROBOHELPER";
        commandString = "ls ";
        commandString += thePath;
        commandString += " | grep wifi_config | wc -l";


        std::cout << "The command string is: " << commandString << std::endl;
        isItPresent = exec(commandString.c_str());
        isItPresent = stripNewline(isItPresent);

        if(isItPresent == "1")
        {
            std::cout << "Found wifi_config!" << std::endl;
            thePath += "/wifi_config";

            std::cout << "The path: " << thePath << std::endl;
            std::fstream wifiConfig;
            wifiConfig.open(thePath.c_str()); // Opens in overwrite mode
            //wifiConfig.open(thePath.c_str(), std::ios_base::app); // Opens in append mode - Don't use this if you are reading.  It puts the file pointer at the end of the file.


            commandString = "cat /etc/wpa_supplicant/wpa_supplicant.conf | grep ssid=\\\"";
                
            std::string networkNames = exec(commandString.c_str()); // 
            networkNames = stripNewline(networkNames);
            std::cout << "Network names: " << std::endl << networkNames << std::endl;

            // Split networkNames because there could be more than one network
            std::vector<std::string> networkNamesVector = split_string(networkNames, "\n");

            std::string networkPair;
            while(getline(wifiConfig, networkPair))
            {
                std::cout << "Pair: " << networkPair << std::endl;
                // Split the pair into a network and a password
                std::vector<std::string> splitPair = split_string(networkPair, ",");

                if(splitPair.size() == 2)
                {
                    // Now figure out if that network is already in wpa_supplicant.conf
                    // Convert the network that I want to add to the form that is in wpa_supplicant
                    std::string temp = " ssid=\"" + splitPair[0] + "\"";


                    bool networkWasFound = false;

                    for(std::string theName : networkNamesVector)
                    {
                        if(temp == theName)
                            networkWasFound = true;
                    }

                    if(networkWasFound)
                        std::cout << "this network is present already" << std::endl;
                    else
                    {
                        std::cout << "This network isn't in wpa_supplicant.conf." << std::endl;

                        // TODO: Add it! - this isn't currently working because I don't have permissions
                        std::fstream wpaSupplicant;
                        //wpaSupplicant.open("/etc/wpa_supplicant/wpa_supplicant.conf", std::ios_base::app);
                        wpaSupplicant.open("/home/jack/wpa_supplicant.conf", std::ios_base::app);

                        wpaSupplicant << "\nnetwork={\n scan_ssid=1\n ssid=\"";
                        wpaSupplicant << splitPair[0];
                        wpaSupplicant << "\"\n psk=\""; 
                        wpaSupplicant << splitPair[1];
                        wpaSupplicant <<"\"\n}\n";

                        wpaSupplicant.close();


// network={
//  scan_ssid=1
//  ssid="JACK"
//  psk="jackPassword"
// }

                        
                    }


                    
                    
                }   

                // Since there is something here, go through 
            }

            wifiConfig.close();
        }
        else
            std::cout << "didn't find wifi_config!" << std::endl;   
    }
    else
        std::cout << "Didn't find ROBOHELPER." << std::endl;


    // If ROBOHELPER is found, look for a file called wifi_config

    // If that network isn't present in /etc/wpa_supplicant/wpa_supplicant.conf, add it.
}


std::string stripNewline(std::string theString)
{
    theString.erase(theString.length()-1);
    return theString;
}