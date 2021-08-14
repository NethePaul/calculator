// calculator.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"

#include<Math.h>
#include<iostream>
#include"calculator.h"


char term[1024] = {};
bool compareStr(char*a,char*b){
	for (int i = 0;; i++) {
		if (a[i] != b[i])return 0;
		if ((!a[i]) && (!b[i]))return 1;
		else if ((!a[i]) || (!b[i]))return 0;
	}
	return 0;
}
bool compareStr(char*a, char*b,int l) {
	for (int i = 0; i < l; i++) {
		if (a[i] != b[i])return 0;
		if ((!a[i]) && (!b[i]))return 1;
		else if ((!a[i]) || (!b[i]))return 0;
	}
	return 1;
}
#define str(a)std::string(a)
int main(int argc,char*argv[]) {
	calculator calculate;
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			bool e=false;
			auto res = calculate.calc(argv[i]);
			if (e|=!!calculate.msg.size())for (const auto&a : calculate.msg)std::cout << "msg	" << a << std::endl;
			if (e|=!!calculate.errors.size()) for (const auto&a : calculate.errors)std::cout << "error	" << a << std::endl;
			if (e|=!!calculate.recorder.size()) for (const auto&a : calculate.recorder)std::cout << a << std::endl;
			if (!e)
				std::cout << res << std::endl;
		}
		return 0;
	}
	while (1) {
		std::cout << ">";
		std::cin.getline(term, 1023);
		if (str("cls") == term)
			system("cls");
		else if (str("EXIT")==term)
			return 0;
		else if (str("help") == term) {
			std::cout
				<< "addition: x+y\nsubtraction: x-y\nmultiplication x*y\ndivision: x/y" << std::endl
				<< "x to the power of y: (x)^(y)\nthe xth root of y: (y)W(x)\nlog x of base y: (x)L(y)" << std::endl
				<< "cosine: cos, sine: sin, tangens: tan\narccosiuns: acos, arcsinus: asin, atangens: atan" << std::endl
				<< "natural log of x: ln(x)" << std::endl
				<< "to get the faculty of x: x!" << std::endl
				<< "to declare a variable with the name a: def:a\nto set the value of the variable a to x: x=>a\nto define a variable with the value x and name a: x=>def:a\nto delete the variable/function/operator with the name a: undef:a" << std::endl
				<< "to declare two or more variables/functions/operators in the same line: [def:var][def:f()=5][def:a op c=a+c]" << std::endl
				<< "to declare a function add: def:add(x,y)=x+y" << std::endl
				<< "to call a function add with the parameters 5 and 6: add(5,6)" << std::endl
				<< "to declare an operator add: def:x add y=x+y" << std::endl
				<< "to call an operator: 5add6" << std::endl
				<< "functions and operators can have variables/functions: def:add(x)=[noredef:def a][f(a,b)=a+b]0+f(a,x)=>a those variables/funcitons are locale to that function" << std::endl
				<< "a function cannot directly access the locale variables/functions/operators of a diffrent function" << std::endl
				<< "to call a function of a function: function_in_scope:sub_function()" << std::endl
				<< "to call a function of a functino of a function: f_in_scope:f_of_previous_f:f()" << std::endl
				<< "to call a function of the parent scope: def:g()=parent:[f()] or def:g()=parent:(f())" << std::endl
				<< "to create a function that shares the locale scope with another function: [def:a(x)=x][def:a:b(y)=x+y]" << std::endl
				<< "recursion is possible but it is impossible to escape it: def:f()=parent:(f())" << std::endl
				<< "to clear the console: cls" << std::endl
				<< "a variable will always be perfered above functions, brackets, numbers and terms" << std::endl
				<< "a variable with a name beginning with an operator may cause unexpected behavior" << std::endl
				;
		}
		else {
			auto res=calculate.calc(term);
			if (calculate.msg.size())for (const auto&a : calculate.msg)std::cout<<"msg	" << a << std::endl;
			if (calculate.errors.size()) for (const auto&a : calculate.errors)std::cout<<"error	" << a << std::endl;
			if (calculate.recorder.size()) for (const auto&a : calculate.recorder)std::cout << a << std::endl;
			std::cout << res << std::endl;
		}
	}
}
