/*-------------------------------------------------------------------*/
// Yong Li (liyong@ios.ac.cn)
/*-------------------------------------------------------------------*/

#pragma once

#include <stdlib.h>
#include <error.h>
#include <iostream>
#include <argp.h>


using namespace std;

#define info(o) cout << "[INFO] " << ( o ) << endl
#define erro(o) cerr << "[ERRO] " << ( o ) << endl

// This file is adapted from Argp example #4 at https://www.gnu.org/software/libc/manual/html_node/Argp-Example-4.html

//const char *argp_program_version ;

//const char *argp_program_bug_address ;

/* Program documentation. */
//static const char doc[] ;
/*
  \
options\
\vThis part of the documentation comes *after* the options;\
 note that the text is automatically filled, but it's possible\
 to force a line-break, e.g.\n<-- here.";
*/

/* A description of the arguments we accept. */
//static char args_doc[] ;

/* Keys for options without short-options. */
//#define OPT_ABORT  1            /* --abort */

/* The options we understand. */
//static struct argp_option options[] ;

enum class dd_package 
{
    BUDDY, CUDD, SYLVAN
};
std::ostream& operator<<(std::ostream& os, const dd_package& obj);

// command line parser
struct opt
{
    // json file for specification
	string spec_file;
    
  string formula_string;

    // 
	bool explicit_;
	bool minimize ;
    
    //
	unsigned num_of_product;
	unsigned num_states_single;
	unsigned num_states_product;
	int num_of_remaining_dfas;

	bool synthesize;
	bool output;
  string output_file;
    
  bool verbose;

	dd_package bdd_pkg;
    
    
  opt();

  ~opt();
    
};

std::ostream& operator<<(std::ostream& os, const opt& obj);


//static bool 
//verify_options(const opt& obj);

//static error_t
//parse_opt(int key, char *arg, struct argp_state *state);
 
void get_opt(int argc, char** args, opt* ops);


