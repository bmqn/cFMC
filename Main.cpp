#include <iostream>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Machine.hpp"

int main()
{
	std::string ex0 = "main S= [x] . print";
	std::string ex1 = "main S= rnd<x> . [x] . print";
	std::string ex2 = "main S= <z> . rnd<y> . out[z] . [y] . print";
	std::string ex3 = "main S= rnd<z> . [[z] . print] . <w> . w . w . rnd<p> . [p] . print";
	std::string ex4 = "main S= [rnd<z> . [z] . print] . <w> . w . w";
	std::string ex6 = "main S= [[f] . print] . [[t] . print] . [true] . <p> . <a> . <b> . [b] . [a] . p";
	std::string ex7 = "main S= [*] . [[hello] . print . <binder> . out[binder]] . [true] . if";
	std::string ex8 = "main S= [*] . [new[hello]] . [true] . if . new<x> . new[x] . [x] . print";

	// (\x.(\x.x) y) N
	// ->  (\x.x) y
	// -> y

	// [N] . <x> . [y] . <x> . x

	// (\x.(\x.x) x y) (\x.x)
	// -> (\x.x) (\x.x) y
	// -> (\x.x) y
	// -> y

	// If capturing
	// [<x> . [x]] . <x> . [y] . [x] . <x> . x
	// -> [y] . [<x> . [x]] . <x> . [<x> . [x]]
	// -> <x> . [<x> . [x]]
	// -> [<x> . [y]]

	std::string ex9 = "main S= [N] . <x> . [y] . <x> . x";
	std::string ex10 = "main S= [<x> . [x]] . <x> . [y] . [x] . <x> . x . print";
	std::string ex11 = "main S= [<x> . x] . print";
 
	// <y> . [x] . y . [x]
	// [<x> . x] . <y> . [x] . y . [x] -> [x] . [<x> . x] . [x]
	// [<z> . z] . <y> . [x] . y -> [x] . [x] . [<z> . z] . [x]

	Parser parser;
	Machine machine;
	machine.printDebug();
	machine.execute(parser.parse(ex10));
	machine.printDebug();

	return 0;
}