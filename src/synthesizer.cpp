#include "synthesizer.hpp"

#include "Timer.hpp"
#include "ltl_parser.hpp"
#include "spotutil.hh"
#include "game.hh"
#include "syntcomp_constants.hpp"
#include "translator.hh"
#include "utils.hpp"
#include "debug.hh"

#define BDD spotBDD
#include <spot/twaalgos/dot.hh>
#undef BDD

#include <chrono>
#include <iostream>
#include <unordered_map>
// extern "C"
// {
//     #include <aiger.h>
// }

//#include <spdlog/spdlog.h>

using namespace std;
using namespace lisa;

int lisa::run(const std::string &tlsf_file_name, uint n_ind,
              uint n_prod, bool exp_mode, bool extract_model,
              const std::string &output_file_name) {
  spot::formula formula;
  set<spot::formula> inputs, outputs;
  bool is_moore;
  tie(formula, inputs, outputs, is_moore) = sdf::parse_tlsf(tlsf_file_name);

  // TODO: what happens when tlsf does have inputs/outputs but the formula
  // doesn't mention them?
  //       (should still have it)

  // std::cout << "\n  tlsf: " << tlsf_file_name;
  // std::cout << "\n  formula: " << formula;
  // std::cout << "\n  inputs: " << sdf::join(", ", inputs);
  // std::cout << "\n  outputs: " << sdf::join(", ", outputs);
  // std::cout << "\n  is_moore: " << is_moore;

  // aiger* model;
  bool game_is_real =
      synthesize_formula(formula, inputs, outputs, n_ind, n_prod,
                               exp_mode, is_moore);

  cout << (game_is_real ? SYNTCOMP_STR_REAL: SYNTCOMP_STR_UNREAL ) << endl;

  if (extract_model) {
    // TODO
    // if (!output_file_name.empty())
    // {
    //     INF("writing a model to " << output_file_name);
    //     int res = (output_file_name == "stdout") ?
    //               aiger_write_to_file(model, aiger_ascii_mode, stdout):
    //               aiger_open_and_write_to_file(model,
    //               output_file_name.c_str());
    //     MASSERT(res, "Could not write the model to file");
    // }
    // aiger_reset(model);
  }

  return (game_is_real ? SYNTCOMP_RC_REAL: SYNTCOMP_RC_UNREAL );
}

bool lisa::synthesize_formula(const spot::formula &formula,
                              const set<spot::formula> &inputs,
                              const set<spot::formula> &outputs, uint n_ind,
                              uint n_prod, bool exp_mode, bool is_moore) {
  spot::bdd_dict_ptr dict = spot::make_bdd_dict();
  dfwa *aut = new dfwa(dict, bddtrue);
  // register formulas
  map<spot::formula, int> &var_map = dict->var_map;
  translate(aut, dict, formula, exp_mode, n_ind, n_prod);
  #ifdef DEBUG
  map<spot::formula, int>::const_iterator iter = var_map.begin();
  while (iter != var_map.end()) {
    spot::formula key = iter->first;
    int value = iter->second;
    cout << key << " -> " << value << endl;
    iter++;
  }
  
  aut->output_dfwa(std::cout);
  #endif
  // game solving
  bdd input_cube, output_cube;
  input_cube = output_cube = bddtrue;
  for (const spot::formula &f : inputs) {
    // not in var map
    if (var_map.count(f) == 0) {
      continue;
    }
    int var_index = dict->varnum(f);
    bdd p = bdd_ithvar(var_index);
    input_cube = input_cube & p;
  }
  for (const spot::formula &f : outputs) {
    // not in var map
    if (var_map.count(f) == 0) {
      continue;
    }
    int var_index = dict->varnum(f);
    bdd p = bdd_ithvar(var_index);
    output_cube = output_cube & p;
  }
  string alive_ap(ALIVE_AP);
  dict->register_proposition(formula::ap(alive_ap), aut);
  int var_index = dict->varnum(formula::ap(alive_ap));
  bdd p = bdd_ithvar(var_index);
  output_cube = output_cube & p;

  //  {
  #ifdef DEBUG
  clock_t c_start = clock();
  auto t_start = chrono::high_resolution_clock::now();
  #endif
  // aut->output_dfwa(cout);
  game_solver solv(*aut, input_cube, output_cube);
  DEBUG_STDOUT( "Starting to synthesize " << endl);
  if (!is_moore) {
    solv.env_play_first();
    DEBUG_STDOUT( "Environment will play first" << endl);
  }else {
    DEBUG_STDOUT( "System will play first" << endl);
  }

  solv.is_realizable();

  #ifdef DEBUG
  clock_t c_end = clock();
  cout << 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n";
  auto t_end = chrono::high_resolution_clock::now();
  cout << "Total CPU time used: " << 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC
       << " ms\n"
       << "Total wall clock time passed: "
       << std::chrono::duration<double, std::milli>(t_end - t_start).count()
       << " ms\n";
  #endif
  if (aut)
    delete aut;
  return solv._result == SYNTCOMP_RC_REAL;
}
