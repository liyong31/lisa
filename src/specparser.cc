/*-------------------------------------------------------------------*/
// Yong Li (liyong@ios.ac.cn)
/*-------------------------------------------------------------------*/

#include "specparser.hh"

// description for the specification
const char *DESCR = "description";
// assumption for the specification
const char *ASSUMPTIONS = "assumptions";
// guarantees for the specification
const char *GUARANTEES = "guarantees";
// inputs for the specification
const char *INPUTS = "inputs";
// outputs for the specification
const char *OUTPUTS = "outputs";
// types for the synthesized strategies
const char *TYPE = "type";
// semantics for the synthesized strategies
const char *SEMANTICS = "semantics";
// unobservable inputs
const char *UNOBSERVS = "unobservables";
//
//const char* WINNING = "winning";
//
const char *GOOD_ENOUGH = "good-enough";

const char *MEALY = "mealy";

spec_ptr
parse_spec(const char *file_name)
{
    spec_ptr p = new spec;

    json j;
    ifstream ispecfile(file_name);
    ispecfile >> j;
    //cout << j << endl;

    if (j[DESCR] != nullptr)
    {
        p->descr = j[DESCR];
    }
    vector<string> assumps = j[ASSUMPTIONS];
    //cout << "Assumptions: " << endl;
    for (string assump : assumps)
    {
        auto pf1 = spot::parse_infix_psl(assump.c_str());
        formula conjunct = pf1.f;
        p->assumptions.push_back(conjunct);
        //cout << pf1.f << endl;
    }
    //cout << "Guarantees: " << endl;
    vector<string> guarantees = j[GUARANTEES];
    for (string guarantee : guarantees)
    {
        auto pf1 = spot::parse_infix_psl(guarantee.c_str());
        formula conjunct = pf1.f;
        p->guarantees.push_back(conjunct);
        //cout << pf1.f << endl;
    }

    for (string input_ap : j[INPUTS])
    {
        p->input_aps.push_back(input_ap);
        //cout << "in: " << input_ap << endl;
    }

    for (string output_ap : j[OUTPUTS])
    {
        p->output_aps.push_back(output_ap);
        //cout << "out: " << output_ap << endl;
    }

    if (j[UNOBSERVS] != nullptr)
    {
        for (string ubobs_ap : j[UNOBSERVS])
        {
            p->unobservable_aps.push_back(ubobs_ap);
            //cout << "unobservables: " << ubobs_ap << endl;
        }
    }

    if(j[TYPE] != nullptr && j[TYPE] == GOOD_ENOUGH)
    {
        p->start_type = strategy_type::GE;
    }
    else
    {
        p->start_type = strategy_type::WINNING;
        //cout << "Winning" << endl;
    }

    if (j[SEMANTICS] == MEALY)
    {
        p->start_semantics = strategy_semantics::MEALY;
    }
    else
    {
        p->start_semantics = strategy_semantics::MOORE;
        //cout << "Moore" << endl;
    }

    return p;
}

std::ostream &
operator<<(std::ostream &os, const spec &obj)
{
    // write obj to stream
    os << "[" << endl;
    os << setw(4) << "\"" << DESCR << "\": \"" << obj.descr << "\"," << endl;
    os << setw(4) << "\"" << SEMANTICS << "\": \"" << (obj.start_semantics == strategy_semantics::MEALY ? "mealy" : "moore") << "\"," << endl;
    os << setw(4) << "\"" << TYPE << "\": \"" << (obj.start_type == strategy_type::WINNING ? "winning" : "good-enough") << "\"," << endl;
    os << setw(4) << "\"" << INPUTS << "\": [";
    bool first = true;
    for (string in : obj.input_aps)
    {
        if (first)
        {
            os << "\"" << in << "\"";
            first = false;
        }
        else
        {
            os << ", \"" << in << "\"";
        }
    }
    os << "]," << endl;
    os << setw(4) << "\"" << OUTPUTS << "\": [";
    first = true;
    for (string out : obj.output_aps)
    {
        if (first)
        {
            os << "\"" << out << "\"";
            first = false;
        }
        else
        {
            os << ", \"" << out << "\"";
        }
    }
    os << "]," << endl;
    os << setw(4) << "\"" << UNOBSERVS << "\": [";
    first = true;
    for (string in : obj.unobservable_aps)
    {
        if (first)
        {
            os << "\"" << in << "\"";
            first = false;
        }
        else
        {
            os << ", \"" << in << "\"";
        }
    }
    os << "]," << endl;

    os << setw(4) << "\"" << ASSUMPTIONS << "\": [";
    if (obj.assumptions.size() == 0)
    {
        os << "]," << endl;
    }
    else
    {
        first = true;
        os << endl;
        for (formula f : obj.assumptions)
        {
            if (first)
            {
                os << setw(6) << "\"" << str_psl(f, true) << "\"" << endl;
                first = false;
            }
            else
            {
                os << setw(6) << ", \"" << str_psl(f, true) << "\"" << endl;
            }
        }
        os << setw(4) << "]," << endl;
    }

    os << setw(4) << "\"" << GUARANTEES << "\": [";
    if (obj.guarantees.size() == 0)
    {
        os << "]," << endl;
    }
    else
    {
        first = true;
        os << endl;
        for (formula f : obj.guarantees)
        {
            if (first)
            {
                os << setw(6) << "\"" << str_psl(f, true) << "\"" << endl;
                first = false;
            }
            else
            {
                os << setw(6) << ", \"" << str_psl(f, true) << "\"" << endl;
            }
        }
        os << setw(4) << "]" << endl;
    }
    os << "]" << endl;
    return os;
}

std::istream &
operator>>(std::istream &is, spec &obj)
{
    json j;
    is >> j;
    //cout << "json parsing" << endl;
    if (j[DESCR] != nullptr)
        obj.descr = j[DESCR];
    vector<string> assumps = j[ASSUMPTIONS];
    for (string assump : assumps)
    {
        auto pf1 = spot::parse_infix_psl(assump.c_str());
        formula conjunct = pf1.f;
        obj.assumptions.push_back(conjunct);
    }
    vector<string> guarantees = j[GUARANTEES];
    for (string guarantee : guarantees)
    {
        auto pf1 = spot::parse_infix_psl(guarantee.c_str());
        formula conjunct = pf1.f;
        obj.guarantees.push_back(conjunct);
    }

    for (string input_ap : j[INPUTS])
    {
        obj.input_aps.push_back(input_ap);
    }

    for (string output_ap : j[OUTPUTS])
    {
        obj.output_aps.push_back(output_ap);
    }

    if (j[UNOBSERVS] != nullptr)
    {
        for (string ubobs_ap : j[UNOBSERVS])
        {
            obj.unobservable_aps.push_back(ubobs_ap);
        }
    }

    if (j[TYPE] != nullptr)
    {
        if (j[TYPE] == GOOD_ENOUGH)
        {
            obj.start_type = strategy_type::GE;
        }
        else
        {
            obj.start_type = strategy_type::WINNING;
        }
    }

    if (j[SEMANTICS] == MEALY)
    {
        obj.start_semantics = strategy_semantics::MEALY;
    }
    else
    {
        obj.start_semantics = strategy_semantics::MOORE;
    }
}

/*
int main(int argc, char **args)
{
    char *name = args[1];
    
    spec_ptr sp = parse_spec(name);
    cout << *sp << endl;
    delete sp;
    
    string fname = name;
    ifstream is(fname);
    spec p;
    is >> p;
    cout << p << endl;
    return 0;
}*/
