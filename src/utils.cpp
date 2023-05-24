#include "utils.hpp"

#include <string>
#include <iostream>

#include <pstream.h>
#include <tuple>
#include <cassert>

using namespace std;


tuple<int, string, string> sdf::execute(const string& cmd)
{
    return sdf::execute(cmd.c_str());
}


tuple<int, string, string> sdf::execute(const char* cmd)
{   // TODO:AK: what happens when cmd crashes?
    redi::ipstream proc(cmd, redi::pstreams::pstdout | redi::pstreams::pstderr);

    string line;

    stringstream out;
    while (getline(proc.out(), line))
        out << line << '\n';
    proc.clear();  // TODO: read redi docs --- why need to call this?

    stringstream err;
    while (getline(proc.err(), line))
        err << line << '\n';

    proc.close();
    if (proc.rdbuf()->exited())
        return tuple<int,string, string>(proc.rdbuf()->status(), out.str(), err.str());

    // this should be unreachable, the code is to silence the warnings
    assert(0);
    return tuple<int,string, string>(-1, "", "");
}

