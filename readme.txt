Overview
=======

Lisa-Syntcomp is an LTLf realizability tool that built on top of 
	(1) [Lisa] (https://github.com/vardigroup/lisa) for LTLf realizability checking, and
	(2) [sdf-hoa] (https://github.com/5nizza/sdf-hoa) modified for processing TLSF input files. 

It is publicly available under the license GNU GPL v3 (souce from sdf-hoa is released under MIT license).


Requirements
-----------------------------------

Lisa requires a C++14-compliant compiler.  G++ 5.x or later should work.

Third-party dependencies
-----------------------------------

* [Spot model checker](https://spot.lrde.epita.fr/) -- placed into third_parties/ folder

* [MONA](https://github.com/liyong31/MONA) -- placed into third_parties/ folder

* [args] (https://github.com/Taywee/args) -- placed into third_parties folder (see CMakeLists.txt)

* [pstreams-1.0.3] (https://pstreams.sourceforge.net/download/) -- placed into third_parties/ folder

* [syfco] (https://github.com/reactive-systems/syfco) -- placed into third_parties/ folder

Compilation steps
-----------------------------------
The easiest way is to use build.sh script
	
	build.sh
	
Then all executables are placed in binary/.
Make sure that the executable syfco is also placed in binary/.

Now you can run lisa-syntcomp with run_lisa.sh.
Note that mona and syfco are invoked via command line, so we need to make sure binary/ is part of PATH.
