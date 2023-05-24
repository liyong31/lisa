#include <iostream>
#include <sstream>
#include <string>

#include <spdlog/spdlog.h>
#include <args.hxx>

#include "utils.hpp"
#include "synthesizer.hpp"


using namespace std;
using namespace sdf;


int main(int argc, const char *argv[])
{
    args::ArgumentParser parser("Lisa synthesizer from LTLf (TLSF format)");
    parser.helpParams.width = 110;
    parser.helpParams.helpindent = 26;

    args::Positional<std::string> tlsf_arg
        (parser, "tlsf",
         "File with TLSF specification",
         args::Options::Required);

    // args::Flag check_dual_flag
    //         (parser,
    //          "dual",
    //          "check the dualized spec (unrealizability)",
    //          {'d', "dual"});

    args::Flag check_real_only_flag
            (parser,
             "real",
             "do not extract the model (check realizability only)",
             {'r', "real"});
    args::ValueFlag<uint> ind_arg(
        parser, "ind", "switch to symbolic approach when the number of"
                            " states of an individual DFA exceeds ind"
                            " (Default: 800)", 
                             {'i'});
    args::ValueFlag<uint> prod_arg(
        parser, "pro", "Switch to symbolic approach when the number of"
                            " states of a product DFA exceeds pro"
                            " (default: 2500)", 
                             {'p'});

    args::ValueFlag<string> output_name
            (parser,
             "o",
             "file name for the synthesized model",
             {'o', "output"});

    args::Flag exp_flag
            (parser,
             "e",
             "explicit mode for DFA construction",
             {'e', "exp"});

    args::Flag silence_flag
            (parser,
             "s",
             "silent mode (the printed output adheres to SYNTCOMP)",
             {'s', "silent"});

    args::Flag verbose_flag
            (parser,
             "v",
             "verbose mode (default: informational)",
             {'v', "verbose"});

    args::HelpFlag help
        (parser,
         "help",
         "Display this help menu",
         {'h', "help"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help&)
    {
        cout << parser;
        return 0;
    }
    catch (args::ParseError& e)
    {
        cerr << e.what() << endl;
        cerr << parser;
        return 1;
    }
    catch (args::ValidationError& e)
    {
        cerr << e.what() << endl;
        cerr << parser;
        return 1;
    }

    // setup logging
    if (silence_flag)
        spdlog::set_level(spdlog::level::off);
    if (verbose_flag)
        spdlog::set_level(spdlog::level::debug);
    
    // ostringstream os;
    auto tlsf_file = args::get(tlsf_arg);
    std::string tlsf_file_name (tlsf_file);
    string output_file_name = "";
    if (output_name) {
        auto output_file = args::get(output_name);
        output_file_name = string(output_file);
    }
    
    // vector<uint> k_list(k_list_arg.Get());
    // bool check_dual_spec = false;
    // if (check_dual_flag) {
    //     check_dual_spec = true;
    // }

    bool check_real_only = false;
    if (check_real_only_flag) {
        check_real_only = true;
    }
    bool explicit_mode = false;
    if (exp_flag) {
        explicit_mode = true;
    }
    uint n_ind = 800;
    if (ind_arg) {
        n_ind = args::get(ind_arg);
    }
    uint n_prod= 2500;
    if (prod_arg) {
        n_prod = args::get(prod_arg);
    }
    // bool postprocess_atm(postprocess_atm_flag.Get());

    return lisa::run(tlsf_file_name, n_ind, n_prod, explicit_mode, !check_real_only, output_file_name);
}

