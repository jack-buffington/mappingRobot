 /*	This file contains some useful functions that I include in many of my programs. 
	Written by Jack Buffington 
	Last updated October 2021
 
 */
 
 
 
 #include "commonlyUsedFunctions.hpp"


 
std::vector<std::string> splitString(const std::string& str, const std::string& delimiter)
{ 	// splits a string into multiple strings based on a delimiter
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



std::string exec(const char* cmd, bool removeWhitespace, bool removeCR_LF) 
{
    char buffer[1000];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) 
    {
		throw std::runtime_error("popen() failed!");
    }
    try 
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) 
        {
            result += buffer;
        }
    } 
    catch (...) 
    {
        pclose(pipe);
        throw;
    }
    
    pclose(pipe);
    if(removeWhitespace)
    {
		result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
	}
	else if(removeCR_LF)
	{ // Just remove carriage returns/new lines
		result.erase(std::remove_if(result.begin(), result.end(), ::iscntrl), result.end());
	}
    return result;
}





// For color code info, see this page: 
// https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
void printInGreen(std::string theText)
{ // Prints a message to the console in green.
	std::string theString = "\033[1;32m" + theText + "\033[0m\n";
	std::cout << theString;
}

void printInRed(std::string theText)
{ // Prints a message to the console in red.

	std::string theString = "\033[1;31m" + theText + "\033[0m\n";
	std::cout << theString;
}

