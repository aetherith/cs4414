#include <iostream>
#include <fstream>
#include <vector>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>


using namespace std;

// for keeping some stats
int num_whales, num_males, num_females, 
  num_matchmakers, num_matches, num_mated;
float total_time_to_mate;
vector<float> whale_times;

void printStats()
{
  cout << endl << endl;
  cout << "Total Whales: " << num_whales << endl;
  cout << "Total Males: " << num_males << endl;
  cout << "Total Females: " << num_females << endl;
  cout << "Total MatchMakers: " << num_matchmakers << endl;
  cout << "Total Matches: " << (num_matches / 2) << endl;
  cout << "Total # Copulations: " << (num_mated / 2) << endl;
  
  cout << "Average Time To Mate: " 
       << (total_time_to_mate / (2.0*num_mated)) << endl;
}

/**
 * a handler for SIGINT so that you can stop 
 * a program with CTRL+C and still get the results.
 */
void handle_signal(int signum) 
{
  switch(signum) {
  case SIGINT:
    printStats();
    exit(0);
    break;
  }
}

/**
 * reads all the input either from a file or
 * from stdin
 */
void readStream(istream& fin, bool showOutput)
{
  // read the input
  char ch;
  
  float t;
  int whale;
  string action;


  string line;
  int rParenIdx, colonIdx, actionIdx;

  while (!fin.eof()) {
    getline(fin, line);
    
    if (showOutput) {
      // print the line back to stdout
      cout << line << endl;
    }

    // yeah, not the best error detection...
    if (line.size() <= 0 || line[0] != '(')
      continue;

    rParenIdx = line.find(')');
    colonIdx = line.find(':');
    actionIdx = colonIdx + 2;

    if (rParenIdx <= 0 || rParenIdx == string::npos ||
	colonIdx <= 0 || colonIdx == string::npos ||
	actionIdx <= 0 || actionIdx == string::npos) {
      cerr << "\nError: couldn't pare line (" << line << ")\n";
      continue;
    }

    t = atof(line.substr(3, rParenIdx-1).c_str());
    whale = atoi(line.substr(rParenIdx+1, colonIdx-rParenIdx-1).c_str());

    action = line.substr(actionIdx, line.size() - actionIdx);

    while (whale >= whale_times.size()) {
      whale_times.push_back(0.0);
    }


    if (action == "Male Created") {
      num_males++;
      num_whales++;
      whale_times[whale] = t;
    } else if (action == "Female Created") {
      num_females++;
      num_whales++;
      whale_times[whale] = t;
    } else if (action == "Found Mate") {
      num_matches++;
    } else if (action == "Mated") {
      num_mated++;
      total_time_to_mate += (t-whale_times[whale]);
    } else if (action == "Became MatchMaker") {
      num_matchmakers++;
    }

  }
}


int main(int argc, char* argv[])
{
  signal(SIGINT, handle_signal);

  bool showOutput = false;
  bool haveFile = false;
  int fileArg = -1;

  // go through all args and get options / filename
  for (int i = 1; i < argc; i++) {
    if ( (strcmp(argv[i], "-o") == 0) ||
	 (strcmp(argv[i], "-O") == 0))
      showOutput = true;
    else if (argv[i][0] != '-') {
      // this must be the filename
      
      if (haveFile) {
	cerr << "Too many arguments." << endl
	     << "Usage: " << argv[0] << " [-o] [filename]" << endl;
	exit(1);
      }

      fileArg = i;
      haveFile = true;
    }
  }


  if (haveFile) {
    ifstream fin(argv[1]);
    if (!fin) {
      cerr << "Bad filename: " << argv[1] << endl;
      exit(0);
    }
    readStream(fin, showOutput);
  } else {
    readStream(cin, showOutput);
  }


  printStats();
}
