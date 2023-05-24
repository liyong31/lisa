/*-------------------------------------------------------------------*/
// Yong Li (liyong@ios.ac.cn)
/*-------------------------------------------------------------------*/

#include "dfwa.hh"
#include "debug.hh"

/*-------------------------------------------------------------------*/
// initialize dd representation of a product DFA
/*-------------------------------------------------------------------*/
dfwa::dfwa(bdd_dict_ptr dict, bdd label_cube)
:_state_vars(dict), _label_cube(label_cube)
{
    // _aut = nullptr;
}

struct GreaterThanByBddSize
{
  bool operator()(bdd& f1, bdd& f2) const
  {
    int size_1 = bdd_nodecount(f1);
    int size_2 = bdd_nodecount(f2);
    if(size_1 < size_2)
    {
        return false;
    }
    return size_1 >= size_2;
  }
};

/*-------------------------------------------------------------------*/
// binary operation construction for computing the disjunction of BDDs
/*-------------------------------------------------------------------*/
bdd
binary_disjunct_rec(vector<bdd> & trans, vector<bdd> & curr_st_bdd, unsigned left, unsigned right)
{
    if(left > right)
    {
        return bddfalse;
    }else
    if(left == right)
    // return (s, a, t)
    {
        return curr_st_bdd[left] & trans[left];
    }else
    // left < right
    {
        unsigned mid = (left + right) / 2;
        bdd op1 = binary_disjunct_rec(trans, curr_st_bdd, left, mid);
        bdd op2 = binary_disjunct_rec(trans, curr_st_bdd, mid + 1, right);
        return op1 | op2;
    }
}
/*-------------------------------------------------------------------*/
// initialize dd representation of a DFA
// make complete dfa
/*-------------------------------------------------------------------*/
dfwa::dfwa(twa_graph_ptr aut, bdd label_cube, set<unsigned>& finals, const char* name)
:_state_vars(aut->get_dict(), aut, 2, name, 0, aut->num_states() - 1), _label_cube(label_cube)
{
    // _aut = aut;
    // label cube
    //cout << "construct state bdd #S = " << aut->num_states() << endl;
    vector<bdd> curr_st_bdd;
    vector<bdd> next_st_bdd;
    for(unsigned i = 0; i < aut->num_states(); i ++)
    {
        bdd dd = _state_vars.new_value(0, i);
        #ifdef DEBUG
        cout << "state i = " << i << endl;
        bdd_print_sat(cout, aut->get_dict(), dd);
        cout << endl;
        #endif
        curr_st_bdd.push_back(dd);
        dd = _state_vars.new_value(1, i);
        #ifdef DEBUG
        cout << "state i' = " << i << endl;
        bdd_print_sat(cout, aut->get_dict(), dd);
        cout << endl;
        #endif
        next_st_bdd.push_back(dd);
    }
    //cout << " curr_size = " << curr_st_bdd.size() << endl;
    _init = curr_st_bdd[aut->get_init_state_number()];
    #ifdef DEBUG
    //cout << "init  = " << aut->get_init_state_number() << endl;
    bdd_print_sat(cout, aut->get_dict(), _init);
    cout << endl;
    #endif
    _trans = bddfalse;
    _finals = bddfalse;
    // later if not needed, remove _reach
    _reach = bddfalse;
    DEBUG_STDOUT( "Computing the transition relation..." << endl);
    // needs to be improved by huffman ?
    //priority_queue<bdd, std::vector<bdd>, GreaterThanByBddSize> pq;
    //vector<bdd> map2tr;
    for(unsigned s = 0; s < aut->num_states(); s ++)
    {
        _reach = _reach | curr_st_bdd[s];
        if(finals.find(s) != finals.end()) 
        {
			_finals = _finals | curr_st_bdd[s];
        }
        bdd sdd = curr_st_bdd[s];
        bdd tdd = bddfalse;
        //bdd outs = bddfalse;
        for(auto& tr : aut->out(s)) 
        {
            tdd = tdd | (tr.cond & next_st_bdd[tr.dst]); 
            //outs = outs | tr.cond;
        }
        sdd = sdd & tdd;
        _trans = _trans | sdd;

    }
    // add a sink state
    DEBUG_STDOUT( "Finished computing the transition relation..." << endl);
    _curr_cube = _state_vars.get_cube(0);
    _next_cube = _state_vars.get_cube(1);
    // make pairs
    _curr_to_next_pairs = _state_vars.make_pair(0, 1);
    _next_to_curr_pairs = _state_vars.make_pair(1, 0);
    
    //bdd all = bdd_replace(_reach, _curr_to_next_pairs);
    //all = all & _reach;
    //_trans = _trans & all;
    #ifdef DEBUG
    cout << "trans = " << endl;
    bdd_print_sat(cout, aut->get_dict(), _trans);
    cout << endl;
    
    cout << "finals = " << endl;
    bdd_print_sat(cout, aut->get_dict(), _finals);
    cout << endl;
    #endif
}

dfwa::~dfwa()
{
    //cout << "HELLO start" << _curr_to_next_pairs << endl;
    if(_curr_to_next_pairs != nullptr)
    {
        bdd_freepair(_curr_to_next_pairs);
        _curr_to_next_pairs = nullptr;
    }
    //cout << "HELLO second" << _next_to_curr_pairs << endl;
    if(_next_to_curr_pairs != nullptr)
    {
        bdd_freepair(_next_to_curr_pairs);
        _next_to_curr_pairs = nullptr;
    }
    
    //cout << "HELLO end" << endl;
}

/*-------------------------------------------------------------------*/
// compute next step image of curr 
// FIXED (note the returned image contains propositions)
/*-------------------------------------------------------------------*/
bdd
dfwa::next_image(bdd curr)
{
    bdd next = bdd_relprod(_trans, curr, _curr_cube & _label_cube);
    next = bdd_replace(next, _next_to_curr_pairs);
    return next;
}

/*-------------------------------------------------------------------*/
// compute previous image of curr 
// FIXED (note the returned image contains propositions)
/*-------------------------------------------------------------------*/
bdd 
dfwa::pre_image(bdd curr)
{
    bdd next = bdd_replace(curr, _curr_to_next_pairs);
	bdd pre = bdd_relprod(_trans, next, _next_cube  & _label_cube);
    return pre;
}
/*-------------------------------------------------------------------*/
// compute reachable state space
/*-------------------------------------------------------------------*/
bdd
dfwa::explore()
{
    bdd s = _init;
    bdd sp = bddfalse;
    unsigned count = 1;
    while(sp != s)
    {
    	DEBUG_STDOUT( "Iteration number = " << count << endl);
        // record the states reached within last step
        sp = s;
        // compute image of next step
#ifdef DEBUG

        cout << "reachable states in product: " << endl;
        bdd_print_set(cout, _state_vars.get_dict(), sp);
        cout << endl;
#endif
        s = sp | next_image(sp);
        ++ count;
    }
    return s;
}

bdd
dfwa::back_explore()
{
    bdd s = _finals;
    bdd sp = bddfalse;
    unsigned count = 1;
    while(sp != s)
    {
    	DEBUG_STDOUT( "Iteration number = " << count << endl);
        // record the states reached within last step
        sp = s;
        // compute image of next step
#ifdef DEBUG
        cout << "reachable states in product: " << endl;
        bdd_print_set(cout, _state_vars.get_dict(), sp);
        cout << endl;
#endif
        s = sp | pre_image(sp);
        DEBUG_STDOUT( "The number of node in reverse R(" << count << ") is " << bdd_nodecount(s) << endl);
        ++ count;
    }
    return s;
}

bool
dfwa::is_empty()
{
	_reach = explore();
	return (_reach & _finals) == bddfalse;
}
/*-------------------------------------------------------------------*/
// get data from dfwa
/*-------------------------------------------------------------------*/
bdd
dfwa::get_init()
{
    return _init;
}

bdd
dfwa::get_trans()
{
    return _trans;
}

bdd
dfwa::get_finals()
{
    return _finals;
}

void
dfwa::output(ostream& os)
{
    os << "dfwa: " << endl;
    os << "init: " << endl;
    bdd_print_set(os, _state_vars.get_dict(), _init);
    os << endl;
    
    os << "trans: " << endl;
    bdd_print_set(os, _state_vars.get_dict(), _trans);
    os << endl;
    
    os << "finals: " << endl;
    bdd_print_set(os, _state_vars.get_dict(), _finals);
    os << endl;
    
}

void
dfwa::output_dfwa(ostream& os)
{
	os << "LISA DFA: " << endl;
	//os << "//ID VAR HIGH LOW " << endl;

	string alive_ap(ALIVE_AP);
	bdd_dict_ptr dict = get_dict();
	int index_alive = dict->varnum(formula::ap(alive_ap));
	bdd dd_alive = bdd_ithvar(index_alive);
	bdd label_cube = bdd_exist(_label_cube, dd_alive);

	vector<int> vars;
	get_list_var_indices(vars, label_cube);
	os << "LABEL VARS:";
	for(unsigned i = 0; i < vars.size(); i ++)
	{
		os << " " << vars[i];
	}
	os << endl;

	vars.clear();
	get_list_var_indices(vars, _curr_cube);
	os << "CURR STATE VARS:";
	for(unsigned i = 0; i < vars.size(); i ++)
	{
		os << " " << vars[i];
	}
	os << endl;

	vars.clear();
	get_list_var_indices(vars, _next_cube);
	os << "NEXT STATE VARS:";
	for(unsigned i = 0; i < vars.size(); i ++)
	{
		os << " " << vars[i] ;
	}
	os << endl;

	os << "INIT: " << _init.id() << endl;
	output_bdd(os, _init);

	os << "FINAL: " << _finals.id() << endl;
	output_bdd(os, _finals);

	bdd tr = bdd_exist(_trans, dd_alive);

	os << "TRANS: " << tr.id() << endl;
	output_bdd(os, tr);

}

void
dfwa::make_complete()
{
	bdd trans = bddtrue;
	for(unsigned i = 0; i < _state_vars._dd_vars[0].size(); i ++)
	{
		bdd var_0 = _state_vars._dd_vars[0][i];
		bdd var_1 = _state_vars._dd_vars[1][i];

		trans = trans & bdd_biimp(var_0, var_1);
	}

}
/*-------------------------------------------------------------------*/
// intersection dfwa:
// result is an empty dfwa
/*-------------------------------------------------------------------*/
void 
intersect_dfwa(dfwa_ptr result, dfwa_ptr op1, dfwa_ptr op2)
{
    // set the copies of state variables
    result._state_vars._copies = 2;
    // now we add variables from op1 and op2
    DEBUG_STDOUT( "Computing the intersection product..." << endl);
    // check whether this part can be improved
    result._state_vars.add_bdd_vars(op1._state_vars);
    result._state_vars.add_bdd_vars(op2._state_vars);
    DEBUG_STDOUT( "#AP1 = " << op1._state_vars._dd_vars[0].size() << " #AP2 = " <<  op2._state_vars._dd_vars[0].size() << endl);
    DEBUG_STDOUT( "#PRO = " << result._state_vars._dd_vars[0].size() << endl);
    // 
    result._init = op1.get_init() & op2.get_init();
    // especially for the transition relation
    DEBUG_STDOUT( "Computing transition relation in the intersection product..." << endl);
    result._trans = op1.get_trans() & op2.get_trans();
    DEBUG_STDOUT( "Finished computing transition relation in the intersection product..." << endl);
    result._finals = op1.get_finals() & op2.get_finals();
    //result._reach = bddfalse;
    result._curr_cube = result._state_vars.get_cube(0);
    result._next_cube = result._state_vars.get_cube(1);
    // make pairs
    result._curr_to_next_pairs = result._state_vars.make_pair(0, 1);
    result._next_to_curr_pairs = result._state_vars.make_pair(1, 0);
    // compute reachable state space
    DEBUG_STDOUT( "Computing reachable state space in the product..." << endl);
    result._reach = bddtrue;//result.explore();
    DEBUG_STDOUT( "Finished computing reachable state space in the product..." << endl);
    DEBUG_STDOUT("Finished computing the intersection product..." << endl);
}

void dfwa::copy(dfwa& d)
{
    // set the copies of state variables
    this->_state_vars._copies = 2;
    // now we add variables from op1 and op2
    // this->_state_vars.clear();
    this->_state_vars.add_bdd_vars(d._state_vars);
    this->_label_cube = bddtrue & d._label_cube;
    // 
    this->_init = bddtrue & d.get_init();
    DEBUG_STDOUT( "copy init: " << (_init) << endl);
    // especially for the transition relation
    this->_trans = bddtrue & d.get_trans();
    DEBUG_STDOUT("copy trans: " << (_trans) << endl);
    DEBUG_STDOUT( "Finished computing transition relation in the intersection product..." << endl);
    this->_finals = bddtrue & d.get_finals();
    DEBUG_STDOUT( "copy finals: " << (_finals) << endl);
    //result._reach = bddfalse;
    this->_curr_cube = this->_state_vars.get_cube(0);
    this->_next_cube = this->_state_vars.get_cube(1);
    // make pairs
    this->_curr_to_next_pairs = this->_state_vars.make_pair(0, 1);
    this->_next_to_curr_pairs = this->_state_vars.make_pair(1, 0);
    // compute reachable state space
    this->_reach = bddtrue;//result.explore();
    DEBUG_STDOUT( "Finished deep copy..." << endl);
}

/*-------------------------------------------------------------------*/
// product for dfwa: func is passed for computing final states
// ASSUMPTION:
//  1. labels of op1 and op2 are the same
//  2. the state variables of op1 and op2 are different
/*-------------------------------------------------------------------*/
void
check_assumption(dfwa_ptr op1, dfwa_ptr op2)
{
	/*
	if(op1._label_cube != op2._label_cube)
	{
		cerr << "product: The propositions are not the same" << endl;
		cerr << "op1: ";
		bdd_print_set(cerr, op1._state_vars.get_dict(), op1._label_cube);
		cerr << endl << "op2: ";
		bdd_print_set(cerr, op1._state_vars.get_dict(), op2._label_cube);
		cerr << endl;
		exit(-1);
	}
	*/

	vector<bdd>& vars_1 = op1._state_vars.get_bdd_vars(0);
	vector<bdd>& vars_2 = op2._state_vars.get_bdd_vars(0);

	// check whether there are common variables
	set<int> vars;
	for(unsigned i = 0; i < vars_1.size(); i ++)
	{
		vars.insert(bdd_var(vars_1[i]));
	}
	for(unsigned i = 0; i < vars_2.size(); i ++)
	{
		if(vars.find(bdd_var(vars_2[i])) != vars.end())
		{
			cerr << "Product: The state variables are not different -> ";
			bdd_print_set(cerr, op1._state_vars.get_dict(), vars_2[i]);
			cerr << endl;
			for(unsigned j = 0; j < vars_1.size(); j ++)
			{
				bdd_print_set(cerr, op1._state_vars.get_dict(), vars_1[j]);
			    cerr << endl;
			}
			exit(-1);
		}
	}
}
// keep the variables in increasing order
dfwa_ptr
product_dfwa(dfwa_ptr op1, dfwa_ptr op2, function<bdd(bdd&, bdd&)> func)
{
	check_assumption(op1, op2);

	bdd label_cube = op1._label_cube & op2._label_cube;
	/*
    cout << "labels in op1: " << endl;
	bdd_print_set(cout, op1.get_dict(), op1._label_cube);
	cout << endl;
	cout << "labels in op2: " << endl;
	bdd_print_set(cout, op1.get_dict(), op2._label_cube);
	cout << endl;
	*/
    dfwa* result = new dfwa(op1.get_dict(), label_cube);
	//cout << "labels in product: " << endl;
	//bdd_print_set(cout, op1.get_dict(), result->_label_cube);
	//cout << endl;
    // set the copies of state variables
    result->_state_vars._copies = 2;
    // now we add variables from op1 and op2
    DEBUG_STDOUT( "Computing the product of two DFAs..." << endl);
    // check whether this part can be improved
    result->_state_vars.add_bdd_vars(op1._state_vars);
    result->_state_vars.add_bdd_vars(op2._state_vars);

    DEBUG_STDOUT("Number of current state variables in the two DFAs are respectively: " << op1._state_vars._dd_vars[0].size() << ", " <<  op2._state_vars._dd_vars[0].size() << endl);
    DEBUG_STDOUT("Number of current state variables in the product is: " << result->_state_vars._dd_vars[0].size() << endl);

    result->_init = op1.get_init() & op2.get_init();
    DEBUG_STDOUT( "Computing transition relation in the intersection product..." << endl);
    result->_trans = op1.get_trans() & op2.get_trans();
    //cout << "trans in product: " << endl;
    //bdd_print_set(cout, op1.get_dict(), result->_trans);
    //cout << endl;
    DEBUG_STDOUT("Finished computing transition relation in the intersection product..." << endl);
    DEBUG_STDOUT("Number of nodes in the transition BDD of the product is: " << bdd_nodecount(result->_trans)  << endl);

    bdd finals_1 = op1.get_finals();
    bdd finals_2 = op2.get_finals();

    result->_finals = func(finals_1, finals_2);
    //cout << "finals in product: " << endl;
    //bdd_print_set(cout, op1.get_dict(), result->_finals);
    //cout << endl;
    result->_reach = bddtrue;
    //result->_label_cube = op1._label_cube & op2._label_cube;
    result->_curr_cube = result->_state_vars.get_cube(0);
    //cout << "state vars_0 in product: " << endl;
    //bdd_print_set(cout, op1.get_dict(), result->_curr_cube);
    //cout << endl;
    result->_next_cube = result->_state_vars.get_cube(1);
    //cout << "state vars_1 in product: " << endl;
    //bdd_print_set(cout, op1.get_dict(), result->_next_cube);
    //cout << endl;
    // make pairs
    result->_curr_to_next_pairs = result->_state_vars.make_pair(0, 1);
    result->_next_to_curr_pairs = result->_state_vars.make_pair(1, 0);
    // compute reachable state space

    DEBUG_STDOUT( "Finished computing the product..." << endl);
    return *result;
}

dfwa_ptr
product_dfwa_and(dfwa_ptr op1, dfwa_ptr op2)
{
	return product_dfwa(op1, op2, local_bdd_and);
}

dfwa_ptr
product_dfwa_or(dfwa_ptr op1, dfwa_ptr op2)
{
	return product_dfwa(op1, op2, local_bdd_or);
}

dfwa_ptr
product_dfwa_minus(dfwa_ptr op1, dfwa_ptr op2)
{
	// first make sure op2 is complete
	return product_dfwa(op1, op2, local_bdd_not_and);
}

