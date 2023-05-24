#pragma once

#include <set>
#include <spdlog/spdlog.h>

#define BDD spotBDD
    #include <spot/parseaut/public.hh>
    #include <spot/twaalgos/hoa.hh>
    #include <spot/twa/bddprint.hh>
    #include <spot/twaalgos/translate.hh>
#undef BDD

// extern "C"
// {
//     #include <aiger.h>
// }
using namespace std;


namespace lisa
{

/**
 * @return code according to SYNTCOMP (unreal_rc if unreal, real_rc if real, else unknown_rc)
 */
int run(const std::string& tlsf_file_name,
        uint n_ind, uint n_prod,
        bool exp_mode = false,
        bool extract_model=false,
        const std::string& output_file_name="");

bool synthesize_formula(const spot::formula& formula,
                        const set<spot::formula>& inputs,
                        const set<spot::formula>& outputs,
                        uint n_ind, uint n_prod,
                        bool exp_mode,
                        bool is_moore);

}
