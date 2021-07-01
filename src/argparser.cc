/*-------------------------------------------------------------------*/
// Yong Li (liyong@ios.ac.cn)
/*-------------------------------------------------------------------*/

#include "argparser.hh"

const char *argp_program_version = "lisa v0.1";;

const char *argp_program_bug_address = "<liyong@ios.ac.cn>";

/* Program documentation. */
static char doc[] = 
  "Translate an LTLf formula to a DFA and perform synthesis if necessary";
/*
  \
options\
\vThis part of the documentation comes *after* the options;\
 note that the text is automatically filled, but it's possible\
 to force a line-break, e.g.\n<-- here.";
*/

/* A description of the arguments we accept. */
static char args_doc[] = "[SPECIFICATION...]";

/* Keys for options without short-options. */
//#define OPT_ABORT  1            /* --abort */

/* The options we understand. */
static struct argp_option options[] = {
    
  // second argument is the key for parsing arguments
  /*-------------------------------------------*/
  { 0,0,0,0, "Input:", 1 },
  {"formula",     'f', "STRING" , 0, "Process the specification STRING"},
  // use json format
  {"file",     'F', "FILE" , 0, "Process the specification in FILE"},
  {"verbose",  'v', 0,       0, "Produce verbose output" },
//  {"quiet",    'q', 0,       0, "Don't produce any output" },
  //{"silent",   's', 0,       OPTION_ALIAS },
  {"output",   'o', "FILE",  0, "Output to FILE instead of standard output" },

  /*-------------------------------------------*/
  {0,0,0,0, "DFA construction:", 2},
  {"explicit",   'e', 0, 0, "Only explicit approach for DFA construction\n(Default: false)"},
  {"individual",  'i', "INT", 0, "Switch to symbolic approach when the number of states of an individual DFA exceeds INT\n(Default: 800)"},
  {"product",    'p', "INT", 0, "Switch to symbolic approach when the number of states of a product DFA exceeds INT\n(Default: 2500)"},
  
  /*-------------------------------------------*/
  {0,0,0,0, "Synthesis:", 3 },
  {"synthesize",   's', 0, 0,
   "Synthesize a strategy from the specification"},
   
  /*-------------------------------------------*/
  {0,0,0,0, "DFA minimization:", 4 },
  {"minimize",   'm', 0, 0,
   "Minimize the DFA for the specification"},
   
  /*-------------------------------------------*/
  {0,0,0,0, "BDD choice:", 5},
   {"cudd",   'c', 0, 0,
   "Apply CUDD for DFA minimization"},
   {"buddy",   'b', 0, 0,
   "Apply BuDDy for DFA minimization"},
   {"sylvan",   'y', 0, 0,
   "Apply Sylvan for DFA minimization"},
  
  /*-------------------------------------------*/
  {0,0,0,0, "Miscellaneous options:", -1},
  { nullptr, 0, nullptr, 0, nullptr, 0 }
  //{"help",  'h', 0,       0, "print this help page" },
  //{0}
};


std::ostream &
operator<<(std::ostream &os, const dd_package &obj)
{
  switch (obj)
  {
  case dd_package::BUDDY:
    os << "BuDDy";
    break;
  case dd_package::CUDD:
    os << "Cudd";
    break;
  default:
    os << "Sylvan";
    break;
  }
  return os;
}

opt::opt()
{
  spec_file = "";

  formula_string = "";

  explicit_ = false;
  minimize = false;

  //
  num_of_product = 6;
  num_states_single = 800;
  num_states_product = 2500;
  num_of_remaining_dfas = -1;

  synthesize = false;
  output = false;
  output_file = "";

  verbose = false;

  bdd_pkg = dd_package::SYLVAN;
}

opt::~opt()
{
  
}

std::ostream &
operator<<(std::ostream &os, const opt &obj)
{

  os << "spec_file: " << obj.spec_file << endl;
  os << "formula_string: " << obj.formula_string << endl;

  os << "explicit_: " << (obj.explicit_ ? "true" : "false") << endl;
  os << "minimize: " << (obj.minimize ? "true" : "false") << endl;

  //
  os << "num_of_product: " << obj.num_of_product << endl;
  os << "num_states_single: " << obj.num_states_single << endl;
  os << "num_states_product: " << obj.num_states_product << endl;
  os << "num_of_remaining_dfas: " << obj.num_of_remaining_dfas << endl;

  os << "synthesize: " << (obj.synthesize ? "true" : "false") << endl;
  os << "output: " << (obj.output ? "true" : "false") << endl;
  os << "output_file: " << obj.output_file  << endl;

  os << "verbose: " << (obj.verbose ? "true" : "false") << endl;
  os << "bdd_pkg: " << obj.bdd_pkg << endl;

  return os;
}

/* Parse a single option. */
static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct opt *input_opt = (struct opt *)state->input;
  //cout << "key: " << key << " arg: "<< (arg? arg: "") << endl;
  switch (key)
  {
  case 'v':
    input_opt->verbose = true;
    break;
  case 'f':
    if (input_opt->spec_file != "")
    {
      cerr << "Cannot input a formula string when already given a spec file: " << input_opt->spec_file << endl;
      exit(-1);
    }
    input_opt->formula_string = arg;
    break;
  case 'F':
    if (input_opt->formula_string != "")
    {
      cerr << "Cannot input a spec file when already given a formula: " << input_opt->formula_string << endl;
      exit(-1);
    }
    input_opt->spec_file = arg;
    //arg ? atoi (arg) : 10;
    break;
  case 'o':
    input_opt->output_file = arg;
    input_opt->output = true;
    break;
  case 'e':
    input_opt->explicit_ = true;
    break;
  case 'i':
    if (arg)
    {
      input_opt->num_states_single = atoi(arg);
    }
    break;
  case 'p':
    if (arg)
    {
      input_opt->num_states_product = atoi(arg);
    }
    break;
  case 's':
    input_opt->synthesize = true;
    break;
  case 'm':
    input_opt->minimize = true;
    break;
  case 'c':
    input_opt->bdd_pkg = dd_package::CUDD;
    break;
  case 'b':
    input_opt->bdd_pkg = dd_package::BUDDY;
    break;
  case 'y':
    input_opt->bdd_pkg = dd_package::SYLVAN;
    break;
  //case ARGP_KEY_NO_ARGS:
  //
  //    argp_usage (state);
  //    break;
    /*
    case ARGP_KEY_ARG:
      /* Here we know that state->arg_num == 0, since we
         force argument parsing to end before any more arguments can
         get here. */
    //  arguments->arg1 = arg;

    /* Now we consume all the rest of the arguments.
         state->next is the index in state->argv of the
         next argument to be parsed, which is the first string
         we're interested in, so we can just use
         &state->argv[state->next] as the value for
         arguments->strings.

         In addition, by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. 
      arguments->strings = &state->argv[state->next];
      state->next = state->argc;

      break;
    */
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static bool 
verify_options(const opt& obj)
{
  if(obj.formula_string == "" && obj.spec_file == "")
  {
    cerr << "No formulas or specification files given in the arguments. Run 'lisa --help' for help" << endl;
    exit(-1);
  }
}

static struct argp argps = {options, parse_opt, args_doc, doc};

/* Our argp parser. */
void
get_opt(int argc, char** argv, opt* op_t)
{
  argp_parse(&argps, argc, argv, 0, 0, op_t);
  verify_options(*op_t);
}



/*
int main(int argc, char **argv)
{
  struct opt ops;

  // Parse our arguments; every option seen by parse_opt will be
  //   reflected in arguments. 
  argp_parse(&argps, argc, argv, 0, 0, &ops);
  verify_options(ops);
  cout << ops << endl;

  exit(0);
}*/
//Go to the first, previous, next, last section, table of
