#include <iostream>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Machine.hpp"

int main()
{
	std::string ex0 = "main = out[x];";
	std::string ex1 = "main = rnd<x> . out[x];";
	std::string ex2 = "main = <x> . rnd<y> . out[x] . out[y];";
	std::string ex3 = "main = rnd<x> . [out[x]] . <f> . f . f;";
	std::string ex4 = "main = [rnd<x> . out[x]] . <f> . f . f;";
	std::string ex5 = "main = rnd<x> . [out[x]] . <f> . f . [hello] . <x> . f . [x];";
	std::string ex6 = "main = rnd<x> . [out[x]] . <f> . f . [hello] . f . [x];";
	std::string ex7 = "main = [rnd<x> . out[x]] . <y> . rnd<x> . [out[x] . y] . <f> . f . [hello] . <x> . f . [x];";

	Parser parser;
	Machine machine;
	machine.execute(parser.parse(ex7));
	machine.printDebug();

	return 0;
}