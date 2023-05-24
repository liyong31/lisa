/*-------------------------------------------------------------------*/
// Yong Li (liyong@ios.ac.cn)
/*-------------------------------------------------------------------*/

#include "game.hh"
#include "syntcomp_constants.hpp"

/*-------------------------------------------------------------------*/
// prepare for realizability checking
/*-------------------------------------------------------------------*/
void
game_solver::prepare()
{
    // first add finals
    _tseq.push_back(_dfa.get_finals());
    _wseq.push_back(_dfa.get_finals());
    //remove input and output variables
    //_rest_cube = bdd_exist(_dfa._label_cube, _in_cube & _out_cube);
    _result = SYNTCOMP_RC_UNKNOWN;
    _env_first = false;
}

game_solver::~game_solver()
{
}

/*-------------------------------------------------------------------*/
// compute pre image
/*-------------------------------------------------------------------*/
bdd
game_solver::pre_image(bdd& curr)
{
    bdd next = bdd_replace(curr, _dfa._curr_to_next_pairs);
    // has label_cube
	bdd pre = bdd_relprod(_dfa._trans, next, _dfa._next_cube);
    // only allow reachable states
    //pre = pre & _dfa._reach;
    return pre;
}

/*-------------------------------------------------------------------*/
// fixed point computation for checking realizability
/*-------------------------------------------------------------------*/
void
game_solver::env_play_first()
{
	_env_first = true;
}

// evaluate assign in dd
bool
game_solver::eval(bdd& dd, bdd& assign)
{
	bdd overlap = dd & assign;
	return overlap != bddfalse;
}

void 
game_solver::is_realizable()
{
    unsigned curr = 0;
    while(true)
    {
        DEBUG_STDOUT( "The number of nodes in T("<< curr << ") is " << bdd_nodecount(_tseq[curr]) << endl);
        DEBUG_STDOUT("The number of nodes in W("<< curr << ") is " << bdd_nodecount(_wseq[curr]) << endl);

        // pre_image (X, Y, Z)
        bdd pre_w = pre_image(_wseq[curr]);
        DEBUG_STDOUT( "The number of nodes in pre_image("<< curr << ") is " << bdd_nodecount(pre_w) << endl);
        /*
        cout << "pre image of curr" << endl;
        bdd_print_set(cout, _dfa._state_vars.get_dict(), pre_w);
        cout << endl;

        cout << "in_cube " << endl;
        bdd_print_set(cout, _dfa._state_vars.get_dict(), _in_cube);
        cout << endl;
        */

        if(_env_first)
        {
        	// first quantify out all the outputs
        	// w_{i + 1}(Z) = w_{i}(Z) | (!w_{i}(Z) &  ∀ X.∃Y. (tr(X, Y, (w_{i})(Z))))
        	bdd tmp = bdd_exist(pre_w, _out_cube);
        	/*
        	cout << "bdd_exist pre_w " << endl;
			bdd_print_set(cout, _dfa._state_vars.get_dict(), tmp);
			cout << endl;
			*/
			tmp = bdd_forall(tmp, _in_cube);
			bdd wp = _wseq[curr] | tmp;
			_wseq.push_back(wp);

        	// retain only valid states
			bdd state_io = pre_w & tmp;
			bdd tp = _tseq[curr] | ((!_wseq[curr]) & state_io);
			_tseq.push_back(tp);

        }else
        {
        	// t_{i + 1}(Z, Y) = t_{i}(Z, Y) | (!w_{i}(Z) &  ∀ X. (tr(X, Y, (w_{i})(Z))))
        	pre_w = bdd_forall(pre_w, _in_cube);
        	/*
			cout << "bdd_foall pre_w " << endl;
			bdd_print_set(cout, _dfa._state_vars.get_dict(), pre_w);
			cout << endl;
			*/
			// now pre_w has only Z and Y
			bdd tp = _tseq[curr] | ((!_wseq[curr]) & pre_w);
			/*
			cout << "bdd_foall tp " << endl;
			bdd_print_set(cout, _dfa._state_vars.get_dict(), tp);
			cout << endl;

			cout << "out_cube " << endl;
			bdd_print_set(cout, _dfa._state_vars.get_dict(), _out_cube);
			cout << endl;
			*/
			_tseq.push_back(tp);
			// w_{i+1}(Z) = ∃Y.ti+1(Z, Y)
			bdd wp = bdd_exist(tp, _out_cube);
			/*
			cout << "bdd_foall wp " << endl;
			bdd_print_set(cout, _dfa._state_vars.get_dict(), wp);
			cout << endl;
			*/
			_wseq.push_back(wp);
        }

        ++ curr;
        // fixed point or see the initial state
        if(_wseq[curr].id() == _wseq[curr - 1].id()
        || eval(_wseq[curr], _dfa._init))
        {
            break;
        }
    }
    DEBUG_STDOUT( "Finished synthesis after " << curr  << " iterations" << endl);
    DEBUG_STDOUT("The number of nodes in T(i+1) is " << bdd_nodecount(_tseq.back()) << endl);

    if(eval(_wseq[curr], _dfa._init))
    {
        _result = SYNTCOMP_RC_REAL;
        DEBUG_STDOUT(SYNTCOMP_STR_REAL << endl);
    }else
    {
        _result = SYNTCOMP_RC_UNREAL;
        DEBUG_STDOUT( SYNTCOMP_STR_UNREAL << endl);
    }
}
/*-------------------------------------------------------------------*/
// synthesize winning strategy
/*-------------------------------------------------------------------*/        
void 
game_solver::synthesize()
{
    // with the set t(Z, Y), we can construct a mealy machine
    // by construct T(X, Y, Z, Z') & t(Z, Y)
	if(_result == SYNTCOMP_RC_REAL)
	{
		_mealy_init = _dfa._init;
		// construct the transition relation of a mealy machine

		_mealy_trans = _dfa._trans;
		// relate one state with only one output
		#ifdef DEBUG
		clock_t c_start = clock();
		#endif // DEBUG
		
		if(_env_first)
		{
			strategy strat(_tseq.back(), _out_cube, _dfa._curr_cube & _in_cube);
			_mealy_out = strat.synthesize_strategy();
		}else
		{
			strategy strat(_tseq.back(), _out_cube, _dfa._curr_cube);
			_mealy_out = strat.synthesize_strategy();
		}
		//cout << "mealy output: " << endl;
		//bdd_print_set(cout, _dfa.get_dict(), _mealy_out);
		//cout << endl;
		#ifdef DEBUG
		clock_t c_end = clock();
	    std::cout << "Strategy exaction done in "<< 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n";
	    std::cout << "The number of nodes in strategy is " << bdd_nodecount(_mealy_out) << endl;
		#endif
	}

}
