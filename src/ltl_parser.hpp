#pragma once

#include <string>
#include <tuple>
#include <set>

#define BDD spotBDD
    #include <spot/tl/formula.hh>
#undef BDD


namespace sdf
{

/**
 * Convert `ltl_file_name` as TLSF (if the name ends with `.tlsf`),
 * and return <formula, inputs, outputs, is_moore>.
 */
std::tuple<spot::formula, std::set<spot::formula>, std::set<spot::formula>, bool>
parse_tlsf(const std::string &tlsf_file_name);

} //namespace sdf
