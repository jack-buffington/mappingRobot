 /*	This file contains some useful functions that I include in many of my programs. 
	Written by Jack Buffington 
	Last updated May 2020
 
 */
 
 
 
 #include "commonlyUsedFunctions.hpp"
 //std::ofstream logFile;
 //std::string logFileName;


// void initLogFile()
// {	// This function creates a file name that will be used during logging based on the 
// 	// current time.  

// 	// Make sure that the directory exists.  This won't blow away anything that is already there.
// 	exec("mkdir -p /data/log/videoRecorder", true,true);

// 	logFileName = "/data/log/videoRecorder/videoRecorder-" + exec("date +%G-%m-%d_%H_%M_%S", true,true) + ".000";
// 	logFileName += ".log";
// 	printInGreen("The log file is:");
// 	printInGreen(logFileName);
// }


// void logAndPrint(std::string theString)
// {	// This function replaces anywhere that I have used std::cout.  It passes its input to 
// 	// std::cout but also saves this output to a log file.  


// 	// Start each entry with the time. 
// 	theString = "\n\033[30;47m" + getTimeStringForLogging() + "\033[0m  " + theString;
// 	//std::cout << theString;
// 	logFile.open(logFileName, std::ios_base::app); // append instead of overwrite
//   	logFile << theString; 
// 	logFile.close();
// }

 
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
	//logAndPrint(theString);
	std::cout << theString;
}

void printInRed(std::string theText)
{ // Prints a message to the console in red.

	std::string theString = "\033[1;31m" + theText + "\033[0m\n";
	//logAndPrint(theString);
	std::cout << theString;
}


// std::string getTimeStringForLogging() 
// {
// 	auto now(std::chrono::system_clock::now());
// 	auto seconds_since_epoch(
// 	std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()));

// 	// Construct time_t using 'seconds_since_epoch' rather than 'now' since it is
// 	// implementation-defined whether the value is rounded or truncated.
// 	std::time_t now_t(
// 	std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(seconds_since_epoch)));

// 	char temp[10];
// 	if (!std::strftime(temp, 10, "%H:%M:%S.", std::localtime(&now_t)))
// 	return "";

// 	std::string nanoseconds = std::to_string(
// 	(std::chrono::duration<long long, std::nano>(now.time_since_epoch() - seconds_since_epoch)).count());

// 	std::string timeString = std::string(temp) + std::string(9-nanoseconds.length(),'0') + nanoseconds;
// 	timeString = timeString.substr(0,12); // Omit this line if you want more digits past the decimal point.

// 	return timeString;
// }