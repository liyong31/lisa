
#define BDD spotBDD
    #include <spot/parseaut/public.hh>
    #include <spot/twaalgos/hoa.hh>
    #include <spot/twa/bddprint.hh>
    #include <spot/twaalgos/translate.hh>
#undef BDD

#include <queue>

#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/isdet.hh>
#include <spot/twaalgos/product.hh>
#include <spot/twaalgos/word.hh>
#include <spot/twaalgos/degen.hh>
#include <spot/twaalgos/remprop.hh>
#include <spot/twaalgos/minimize.hh>

#include <spot/twa/twagraph.hh>

#include <spot/tl/formula.hh>
#include <spot/tl/print.hh>
#include <spot/tl/parse.hh>
#include <spot/tl/relabel.hh>
#include <spot/tl/ltlf.hh>
#include <spot/tl/simplify.hh>

#include <spot/misc/optionmap.hh>
#include <spot/misc/timer.hh>

#include "dfwa.hh"
#include "translator.hh"
#include "debug.hh"


using namespace spot;
using namespace std;
// using namespace lisa;

void lisa::get_formulas(vector<formula> &lst, formula f)
{
	DEBUG_STDOUT( "Breaking formula into small pieces..." << endl);
	if (f.kind() == op::And)
	{
		// needs to limit the number of conjunctions if no minimization is used before producting two FAs
		for (formula child : f)
		{
			lst.push_back(child);
		}
	}
	else
	{
		lst.push_back(f);
	}
}

bool lisa::translate(dfwa* res, spot::bdd_dict_ptr dict, const spot::formula& input_f,
                        bool exp_mode, uint n_ind, uint n_prod)
{
    priority_queue<dfwa_pair, std::vector<dfwa_pair>, GreaterThanByDfwaSize> autlist;
	clock_t c_start = clock();
	// spot::bdd_dict_ptr dict = spot::make_bdd_dict();
	DEBUG_STDOUT(  "Starting the decomposition phase" << endl);
	vector<formula> lst;
	//cout << "parsed: " << pf1.f << endl;
	get_formulas(lst, input_f);
	//reorganize_formulas(lst);
	while (lst.size() > 0)
	{
		// translating automata
		formula f = lst.back();
		lst.pop_back();
		// cout << str_psl(f, true) << endl;
		twa_graph_ptr aut = trans_formula(f, dict, 7);
		dict->register_all_variables_of(aut, res);
		// cout << aut->num_states() << endl;
		dfwa_pair pair(aut, aut->num_states(), true, f);
		pair._num_propduct = 0;
		// cout << "st = " << aut->num_states() << endl;
		autlist.push(pair);
	}

	//cout << "splited formulas" << endl;
	// do products
	//bdd_autoreorder(BDD_REORDER_WIN2ITE);
	DEBUG_STDOUT(  "Starting the composition phase" << endl);

	set<twa_graph_ptr> optimized;
	while (autlist.size() > 1)
	{
		DEBUG_STDOUT( "Number of DFAs in the set: " << autlist.size() << endl);
		dfwa_pair first = autlist.top();
		autlist.pop();
		dfwa_pair second = autlist.top();
		autlist.pop();
		DEBUG_STDOUT(  "Number of states or nodes in M1 and M2: " << first._num_states
			 << ",  " << second._num_states << endl);
		spot::formula result_formula = formula::And({first._formula, second._formula});
		//cout << result_formula << endl;
		bool must_symbolic = false;//opt_->num_of_remaining_dfas > 0 && autlist.size() + 2 <= opt_->num_of_remaining_dfas;
		if (first._is_explicit && second._is_explicit)
		{
			twa_graph_ptr A = first._twa;
			twa_graph_ptr B = second._twa;
			if (optimized.find(A) == optimized.end())
			{
				A = minimize_explicit(A);
				optimized.insert(A);
			}
			if (optimized.find(B) == optimized.end())
			{
				B = minimize_explicit(B);
				optimized.insert(B);
			}

			if (exp_mode || (!must_symbolic && (A->num_states() < n_ind && B->num_states() < n_ind 
			&& (A->num_states() * B->num_states() < n_prod))))
			{
				// explict representation used
				twa_graph_ptr P = spot::product(A, B);
				//cout << "explicit minimization starts..." << endl;
				P = spot::minimize_wdba(P);
				optimized.insert(P);
				dfwa_pair pair(P, P->num_states(), true, result_formula);
				pair._num_propduct = 1;
				DEBUG_STDOUT(  "Number of states in explicit product is: " << P->num_states() << endl);
				autlist.push(pair);
			}
			else
			{
				dfwa *fst = symbolize_twa(dict, A);
				dfwa *snd = symbolize_twa(dict, B);
				tuple<dfwa *, unsigned, bool> result = make_product(dict, fst, snd, 2);
				dfwa_pair pair(get<0>(result), get<1>(result), false, result_formula);
				if (get<2>(result))
				{
					pair._num_propduct = 1;
				}
				else
				{
					pair._num_propduct = 2;
				}
				DEBUG_STDOUT( "Number of nodes in symbolic product is: " << get<1>(result) << endl);
				autlist.push(pair);
				delete fst;
				delete snd;
			}
		}
		else if (first._is_explicit || second._is_explicit)
		{
			// needs symbolic automata
			dfwa *A = nullptr;
			dfwa *B = nullptr;

			if (first._is_explicit)
			{
				twa_graph_ptr aut = first._twa;
				B = second._dfa;
				// make sure it is weak DBA
				if (optimized.find(aut) == optimized.end())
				{
					aut = minimize_explicit(aut);
				}
				// now compute dfwa
				A = symbolize_twa(dict, aut);
			}
			else
			{
				twa_graph_ptr aut = second._twa;
				B = first._dfa;
				if (optimized.find(aut) == optimized.end())
				{
					aut = minimize_explicit(aut);
				}
				// now compute dfwa
				A = symbolize_twa(dict, aut);
			}
			unsigned num = first._num_propduct + second._num_propduct + 1;
			tuple<dfwa *, unsigned, bool> result = make_product(dict, A, B, num);
			dfwa_pair pair(get<0>(result), get<1>(result), false, result_formula);
			if (get<2>(result))
			{
				pair._num_propduct = 1;
			}
			else
			{
				pair._num_propduct = num;
			}
			DEBUG_STDOUT(  "Number of nodes in symbolic product is: " << get<1>(result) << endl);
			autlist.push(pair);
			delete B;
			// delete A;
		}
		else
		{
			// two symbolic automata
			dfwa *A = first._dfa;
			dfwa *B = second._dfa;
			unsigned num = first._num_propduct + second._num_propduct;
			tuple<dfwa *, unsigned, bool> result = make_product(dict, A, B, num);
			dfwa_pair pair(get<0>(result), get<1>(result), false, result_formula);
			DEBUG_STDOUT(  "Number of nodes in symbolic product is: " << get<1>(result) << endl);
			autlist.push(pair);
			if (get<2>(result))
			{
				pair._num_propduct = 1;
			}
			else
			{
				pair._num_propduct = num;
			}

			delete A;
			delete B;
		}
	}
	clock_t c_end = clock();
	DEBUG_STDOUT(  "Finished constructing minimal dfa in "
		 << 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << "ms ..." << endl);
	dfwa_pair pair = autlist.top();
	DEBUG_STDOUT(  "Number of states (or nodes) is: " << pair._num_states << endl);
	DEBUG_STDOUT( "Final result (or number of nodes): " << pair._num_states << endl);
	// map<spot::formula, int>::const_iterator iter = dict->var_map.begin();
	// while (iter !=  dict->var_map.end())
	// {
	// 	spot::formula key = iter->first;
	// 	int value = iter->second;
	// 	cout << key << " -> " << value << endl;
	// 	// now register it with 
	// 	iter++;
	// }
	if (!pair._is_explicit)
	{
		res->copy(*pair._dfa);
		delete pair._dfa;
	}
	else
	{
		dfwa* sym = symbolize_twa(dict, pair._twa);
		res->copy(*sym);
		// sym->output_dfwa(std::cout);
		delete sym;
        // std::cout << "finished DFA" << endl;
		// res->output_dfwa(std::cout);
	}

	return true;
	// res->_state_vars._dict = dict;
	// res->get_dict()
}
