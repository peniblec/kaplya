#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>

using namespace std;

#ifdef DEBUG_ON
#define DEBUG(x) do { cerr << "DBG: " << x << endl; } while (0)
#else
#define DEBUG(x)
#endif

#define MESSAGE_SIZE 128

const string COMMAND_SEND = "send";
const string COMMAND_ADD = "add";


#endif 
