#include <iostream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <cstring>
#include "common.hpp"
#include "lognum.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "data.hpp"
#include "stacksubset.hpp"
#include "scores.hpp"
#include "bestdagdp.hpp"
#include "order.hpp"
#include <boost/program_options.hpp>
using std::list;
using std::vector;
using namespace std;
namespace opts = boost::program_options;

int main(int argc, char **argv)
{
    string inputfile;
    int burn_in;
    int max_parent_size;
    int swap_n;


    opts::options_description desc("Options");
    opts::variables_map vm;
    desc.add_options()
    ("input-file,i", opts::value<string>(&inputfile)->default_value("./eg.dat"), "set input file of data")
    ("burn-size,b", opts::value<int>(&burn_in)->default_value(1000), "number of burn-in")
    ("max-parent-size,m", opts::value<int>(&max_parent_size)->default_value(3), "maximum size of parent set")
    ("swap-n,s",opts::value<int>(&swap_n)->default_value(5),"set num of variables for swaping every loop")
    ("help,h", "produce help message");
    opts::positional_options_description pdesc;
    pdesc.add("input-file", 1);
    try
    {
		opts::store(opts::command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vm);
        opts::notify(vm);

    }
    catch (opts::error &err)
    {
        cout << "Error:" << err.what() << endl;
        cout << "Aborting." << endl;
        return 1;
    }
    if (vm.count("help"))
    {
        cout << desc << endl;
        return 1;
    }

    Data data;
    istream inStream(0);
    ifstream inFile;
    inFile.open(inputfile);
    inStream.rdbuf(inFile.rdbuf());
    data.read(inStream);
    if (inFile.is_open())
        inFile.close();
    int nVariables = data.nVariables;
    vector<int> targets;
    for (int i = 0; i < nVariables; ++i)
    {
        targets.push_back(i);
    }
    myMCMC(data, targets, burn_in, max_parent_size, swap_n);
    return 0;
}