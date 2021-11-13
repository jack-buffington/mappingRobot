 /*	This file contains some useful functions that I include in many of my programs. 
	Written by Jack Buffington 

	Note that if you are finding this in an online repository, not all of this code 
	is used in all of my programs.  I keep this code as a general-purpose library but
	may only be using parts of it. 

	Last updated November 2021
 
 */
 
 
 
 #include "commonlyUsedFunctions.hpp"

struct termios old, current;
struct timeval start, end;
 
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


// ##################################
// Things dealing with keyboard input
// ##################################

// These keyboard functions were found in a comment on this page: 
// https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux

void initTermios(int echo) 
{
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	current = old; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	if (echo) 
	{
		current.c_lflag |= ECHO; /* set echo mode */
	} 
	else 
	{
		current.c_lflag &= ~ECHO; /* set no echo mode */
	}
	tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

void turnOffLocalEcho()
{
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	current = old; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set no echo mode */
	tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}


void turnOnLocalEcho()
{
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	current = old; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag |= ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

void resetTermios(void) 
{ // Restore old terminal i/o settings 
	tcsetattr(0, TCSANOW, &old);
}



char getch_(int echo) 
{ // Read 1 character - echo defines echo mode 
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}



char getch(void) 
{ // Read 1 character without echo
	return getch_(0);
}



char getche(void) 
{ // Read 1 character with echo
	return getch_(1);
}

int kbhit() 
{
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) 
    {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

void startTiming()
{  	// Sets a start time to use as a reference later
	gettimeofday(&start, NULL);
}

int getElapsedTimeInMilliseconds()
{	// Returns the number of milliseconds since startTiming was called.
	gettimeofday(&end, NULL);
    long seconds  = end.tv_sec  - start.tv_sec;
    long useconds = end.tv_usec - start.tv_usec;
    return ((seconds) * 1000 + useconds/1000.0) + 0.5;
}

int getElapsedTimeInMicroseconds()
{	// Returns the number of microseconds since startTiming was called.
	gettimeofday(&end, NULL);
    long seconds  = end.tv_sec  - start.tv_sec;
    long useconds = end.tv_usec - start.tv_usec;
    return ((seconds) * 1000000 + useconds) + 0.5;
}

void printElapsedTimeInMilliseconds()
{	// Prints how much time has passed since startTiming was called in milliseconds.
	std::cout << "ElapsedTime: " << getElapsedTimeInMilliseconds() << "ms.\n";
}

void printElapsedTimeInMicroseconds()
{	// Prints how much time has passed since startTiming was called in microseconds.
	std::cout << "ElapsedTime: " << getElapsedTimeInMicroseconds() << "us.\n";
}