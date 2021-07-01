/*-------------------------------------------------------------------*/
// Yong Li (liyong@ios.ac.cn)
/*-------------------------------------------------------------------*/

#pragma once

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>      // std::setw

#include <spot/tl/formula.hh>
#include <spot/tl/print.hh>
#include <spot/tl/parse.hh>

// for convenience
using json = nlohmann::json;
using namespace std;
using namespace spot;

// description for the specification
extern const char* DESCR;
// assumption for the specification
extern const char* ASSUMPTIONS;
// guarantees for the specification
extern const char* GUARANTEES;
// inputs for the specification
extern const char* INPUTS;
// outputs for the specification
extern const char* OUTPUTS;
// types for the synthesized strategies
extern const char* TYPE ;
// semantics for the synthesized strategies
extern const char* SEMANTICS;
// unobservable inputs 
extern const char* UNOBSERVS;
// 
//const char* WINNING = "winning";
//
extern const char* GOOD_ENOUGH;

extern const char* MEALY;


enum class strategy_type { WINNING, GE };
enum class strategy_semantics { MOORE, MEALY };

struct spec
{
    string descr;

    strategy_type start_type;
    strategy_semantics start_semantics;

    vector<string> input_aps;
    vector<string> output_aps;
    vector<string> unobservable_aps;

    vector<formula> assumptions;
    vector<formula> guarantees;


    friend std::ostream& operator<<(std::ostream& os, const spec& obj);
    friend std::istream& operator>>(std::istream& is, spec& obj);

};

typedef spec* spec_ptr;

spec_ptr
parse_spec(const char* file_name);





