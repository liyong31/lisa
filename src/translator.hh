#pragma once

#define BDD spotBDD
    #include <spot/parseaut/public.hh>
    #include <spot/twaalgos/hoa.hh>
    #include <spot/twa/bddprint.hh>
    #include <spot/twaalgos/translate.hh>
    #include <spot/twaalgos/hoa.hh>
    #include <spot/twaalgos/translate.hh>
    #include <spot/twaalgos/isdet.hh>
    #include <spot/twaalgos/product.hh>
    #include <spot/twaalgos/word.hh>
    #include <spot/twaalgos/degen.hh>
    #include <spot/twaalgos/remprop.hh>
    #include <spot/twaalgos/minimize.hh>
#undef BDD

#include "dfwa.hh"

namespace lisa
{
    bool translate(dfwa* res, spot::bdd_dict_ptr dict, const spot::formula& formula,
                        bool exp_mode, uint n_ind, uint n_prod);

    void get_formulas(vector<formula> &lst, formula f);

    class dfwa_pair
    {
    public:
        unsigned _num_states;
        bool _is_explicit;
        twa_graph_ptr _twa;
        dfwa *_dfa = nullptr;
        unsigned _num_propduct = 0;

        formula _formula;

        dfwa_pair(twa_graph_ptr aut, unsigned num_states, bool is_explicit, formula &f)
            : _num_states(num_states), _is_explicit(is_explicit), _formula(f)
        {
            _twa = aut;
        }
        dfwa_pair(dfwa *aut, unsigned num_states, bool is_explicit, formula &f)
            : _num_states(num_states), _is_explicit(is_explicit), _formula(f)
        {
            _dfa = aut;
        }
    };

    struct GreaterThanByDfwaSize
    {
        bool operator()(dfwa_pair &p1, dfwa_pair &p2) const
        {
            if (p1._num_states < p2._num_states)
            {
                return false;
            }
            else if (p1._num_states == p2._num_states)
            {
                return !p1._is_explicit;
            }
            return p1._num_states >= p2._num_states;
        }
    };

    inline bool compare_aut_size(twa_graph_ptr p1, twa_graph_ptr p2)
    {
        if (p1->num_states() == p2->num_states())
        {
            return false;
        }
        return p1->num_states() > p2->num_states();
    }

    inline twa_graph_ptr minimize_explicit(twa_graph_ptr A)
    {
        twa_graph_ptr C = spot::minimize_wdba(A);
        //A = spot::minimize_obligation(A);
        // check equivalence of two automata
    #ifdef DEBUG
        string word = is_twa_equivalent(A, C);
        if (word.size() == 0)
        {
            cout << "A: equivalent two automata" << endl;
        }
    #endif
        return C;
    }

    inline tuple<dfwa *, unsigned, bool>
    make_product(bdd_dict_ptr dict, dfwa *A, dfwa *B, unsigned num_prod)
    {
        unsigned num_states;

        dfwa_ptr P = product_dfwa_and(*A, *B);
        {
            num_states = bdd_nodecount(P._trans);
            return make_tuple<>(&P, num_states, false);
        }
    }

    inline dfwa *
    symbolize_twa(bdd_dict_ptr dict, twa_graph_ptr aut)
    {
        // there is alive states
        bdd label_cube = bddtrue;
        for (auto f : aut->ap())
        {
            bdd f_var = bdd_ithvar(aut->register_ap(f));
            label_cube = label_cube & f_var;
        }

        set<unsigned> finals_aut;
        compute_final_states(aut, finals_aut);
        // now compute dfwa
        dfwa *A = new dfwa(aut, label_cube, finals_aut);
        return A;
    }



}
