#pragma once
#define _USE_MATH_DEFINES
#include<math.h>
#include<string>
#include<vector>
#include<memory>

auto mod=[](auto a, auto b) {return a-floor(a / b)*b;};

class calculator {
	calculator*parent = 0;
public:
	std::vector<std::string>recorder;
	std::vector<std::string>errors;
	std::vector<std::string>msg;
	struct udfunction{//user defined functions
		//e.g.function def f(x,y,z)=x+y*z
		std::vector<std::string> paralist;//"x","y","z"
		std::string name;//"f("
		std::string function;//"x+y*z"
		std::shared_ptr<calculator> calc;
		udfunction(const udfunction&mov) = default;
		udfunction() = default;
		udfunction&operator=(const udfunction&mov) = default;
	};
	struct udoperator {//user defined operators
		//e.g.operator def x f y=x+y
		std::string paraFirst;//"x"
		std::string paraSecond;//"y"
		std::string name;//"f"
		std::string function;//"x+y"
		std::shared_ptr<calculator>calc;
		udoperator(const udoperator&mov) = default;
		udoperator&operator=(const udoperator&mov) = default;
		udoperator() = default;
	};

private:

	bool record = 0;
	bool redef = true;
	struct set_true_on_destruction { bool*p; set_true_on_destruction(bool&p) :p(&p) {} ~set_true_on_destruction() { *p = 1; } };

	void recording(const std::string&r) {
		if (record)
			recorder.push_back(r);
	}
	std::vector<udfunction>udf;
	std::vector<udoperator>udo;
	int constcount = 0;//the first Vars are constance and should not be overwritten
	static const std::vector<std::string>functions;
	std::vector<std::pair<std::string, long double>>Var;//variables local to this class
	std::vector<std::pair<std::string,std::vector<long double>>>lists;//arrays of variables local to this class

	udfunction*is_udf(const char*in) {
		if (parent&&std::string(in, in + 7) == "parent:") { return parent->is_udf(in + 7); }
		for (auto& a : udf)
			if (std::string(in, in + a.name.size()) == a.name &&(in[a.name.size()] == ':' || (in[a.name.size()] == '(')))
				return &a;
		return nullptr;
	}
	udoperator*is_udo(const char*in){
		if (parent&&std::string(in, in + 7) == "parent:") { return parent->is_udo(in + 7); }
		restart:
		for (auto& a : udo)
			if (!a.name.size()) { udo.erase(udo.begin() + (&a - udo.begin()._Ptr)); goto restart; }
			else if (std::string(in, in + a.name.size()) == a.name&&(*(in + a.name.size())==':'||is_value(in+a.name.size()))) {
				return &a;
			}
		return nullptr;
	}

	int is_Num(const char*in) {
		const char* x = in;
		for (;;in++)
			if (!(*in&&(*in=='.'||(*in >= '0'&&*in <= '9'))))
				return in-x;
	};
	std::pair<std::string, long double>*is_Var(const char*in, bool*is_const = 0) {
		if (parent&&std::string(in, in + 7) == "parent:") { return parent->is_Var(in + 7); }
		std::pair<std::string, long double>*buffer = 0;
		int i = 0;
		for (; i < Var.size(); i++)
			if (Var[i].first==std::string(in,in + Var[i].first.size())) {
				if(!buffer)buffer=&Var[i];
				else if(buffer->first.size()<Var[i].first.size())buffer = &Var[i];
				break;
			}
		if (is_const)
			*is_const = i < constcount;
		return buffer;//*/
	};

	int is_Function(const char*in) {
		for (int i = 0; i < functions.size(); i++)
			if (functions[i] == std::string(in, in + functions[i].size())) return i + 1;
		return 0;
	};
	bool is_value(const char*in) {
		if (!in)return 0;
		if (is_Num(in))return 1;
		if (is_Function(in))return 1;
		if (is_udf(in))return 1;
		if (is_Var(in))return 1;
		if (in[0] == '(')return 1;
		if (in[0] == '[')return 1;
		return 0;
	};

	long double fakultat(int in) {
		long double buff = 1;
		if (buff < 0)return NAN;
		for (int i = in; i > 0; i--) {
			buff *= i;
		}
		return buff;
	};

	long double strToNum(const char*in) {
		long double buff = 0; double point = 0;
		for (int i = 0; in[i]; i++) {
			if (!point)
				if (in[i] >= '0'&&in[i] <= '9') { buff *= 10; buff += in[i] - '0'; }
				else if(in[i]=='.') point = 1;
				else return buff;
			else {
				point *= 10;
				if(in[i]>='0'&&in[i]<='9')  buff += in[i]-'0' / point;
				else return buff;
			}
		}
		return buff;
	};

	bool not_from_child = 1;
	long double loadValue(const char*in, int&i,bool from_child=0) {
		not_from_child = !from_child; set_true_on_destruction a(not_from_child);
		if (parent&&std::string(in, in + 7) == "parent:") {return parent->loadValue(in,i+=7,true); }
		long double b = 0;
		std::pair<std::string, long double>*buffer = is_Var(in + i);
		int buff = is_Num(in + i);
		auto pudf = is_udf(in + i);

		if (buffer) {
			b = buffer->second; i += buffer->first.size();
			recording("var:"+buffer->first);
		}
		else if (buff){b = strToNum(in + i);i += buff; recording("number:"+std::string(in+i-buff,in+i));}
		else if (is_Function(in + i))b = function(in, i);
		else if (pudf)b = cudf(in, i,pudf);
		else if (in[i] == '(') { recording("("); b = add(in, ++i); if (!(in[i++] == ')'))errors.emplace_back("unexpected character before \")\" or missing \")\""); recording(")"); }
		else if (in[i] == '[') {
			if (recorder.size() && recorder.back()[0] == '[') {
				if (record)
					recorder.back().insert(0, 1, '[');
			}
			else recording("[");
			b = calc(in + (++i), false, record);
			int x = 1;
			for (; in[i] && x; i++) {
				if (in[i] == '[')x++;
				else if (in[i] == ']')x--;
			}
			if (x > 0)
				errors.emplace_back("unexpected character before \"]\" or missing \"]\"");
			if (recorder.size() && recorder.back()[0] == ']') { if (record)recorder.back().insert(recorder.back().begin(), ']'); }
			else recording("]");
		}

		for (; in[i] == '!'; i++) { recording("!"); b = fakultat(b); }
		return b;
	};
	long double function(const char*in, int&i) {
		int buffer = is_Function(in + i);
		if (!buffer)return NAN;
		i += functions[buffer-1].size();
		recording("function:"+functions[buffer-1]);
		switch (buffer) {
		case 1: return cos(loadValue(in, i) * M_PI / 180.0);
		case 2: return sin(loadValue(in, i) * M_PI / 180.0);
		case 3: return tan(loadValue(in, i) * M_PI / 180.0);
		case 4: return acos(loadValue(in, i)) * 180 / M_PI;
		case 5: return asin(loadValue(in, i)) * 180 / M_PI;
		case 6: return atan(loadValue(in, i)) * 180 / M_PI;
		case 7: return log(loadValue(in, i)) / log(M_E);
		default:return 0;
		}
	};
	long double cudf(const char*in, int &i, udfunction*p) {//calculate user defined function
	begin:
		i += p->name.size();
		//if (!p->calc) { errors.push_back("parent function deleted"); return 0; }//unreachable
		if (in[i] == ':') {
			i++;
			p = p->calc->is_udf(in + i);//idk why but my debugger complains that it cant convert "udfunction*" to "udfunction*", compiles fine though
			if(p)goto begin;
			errors.push_back("unknown function");
			return 0;
		}
		recording("udf:"+p->name);
		p->calc->errors.clear();
		if (in[i] == '(') {
			i++;
			if (p->paralist.size() == 0) { i++; goto paralist_finished;}
			for (auto&para : p->paralist) {
				p->calc->setVar(para.c_str(), add(in, i));
				if (in[i] == ',')i++;
				else if (in[i] == ')'&&&para==p->paralist.end()._Ptr-1) { i++; goto paralist_finished; }
				else break;
			}
		}
		//error
		errors.emplace_back("incorrect parameter list");
		return 0;
	paralist_finished:
		long double a=p->calc->calc(p->function.c_str(),false,record);
		errors.insert(errors.end(),p->calc->errors.begin(),p->calc->errors.end());
		recorder.insert(recorder.end(), p->calc->recorder.begin(), p->calc->recorder.end());
		return a;

	}
	long double cudo(const char*in, int &i) {//calculate user defined operator
		long double ANS = loadValue(in, i);
		udoperator*p = 0;
		while (1) {
			if (p = is_udo(in + i)) {
				recording("operator:"+p->name);
				i += p->name.size();
				p->calc->setVar(p->paraFirst.c_str(), ANS);
				p->calc->setVar(p->paraSecond.c_str(), loadValue(in, i));
				ANS = p->calc->calc(p->function.c_str(),false, record);
				errors.insert(errors.end(), p->calc->errors.begin(), p->calc->errors.end());
				recorder.insert(recorder.end(), p->calc->recorder.begin(), p->calc->recorder.end());
			}
			else return ANS;
		}
	}
	long double pot(const char*in, int&i) {
		long double ANS = cudo(in, i);
		while (1) {
			if (in[i] == '^') {
				if (!is_value(in + i + 1)) {
					ANS *= ANS;
					i++;
				}
				else {
					long double buffer = loadValue(in, ++i);
					if ((!buffer) && (!ANS))ANS = NAN;
					else ANS = pow(ANS, buffer);
				}
				recording("^");
			}
			else if (in[i] == 'W') {
				if (!is_value(in + i + 1)) {
					ANS = sqrt(ANS);
					i++;
				}
				else ANS = pow(ANS, 1 / loadValue(in, ++i));
				recording("W");
			}
			else if (in[i] == 'L') {
				if (!is_value(in + i + 1)) {
					ANS = log10(ANS);
					i++;
				}
				else ANS = log(ANS) / log(loadValue(in, ++i));
				recording("L");
			}
			else return ANS;
		}
	};
	long double mul(const char*in, int&i) {
		long double ANS = pot(in, i);
		while (1) {
			if (in[i] == '*') {
				if (!is_value(in + i + 1)) {
					ANS *= 2;
					i++;
				}
				else ANS *= pot(in, ++i);
				recording("*");
			}
			else if (is_value(in + i)) {
				ANS *= pot(in, i);
				recording("implicit *");
			}
			else if (in[i] == '/') {
				if (!is_value(in + i + 1)) {
					ANS *= 0.5;
					i++;
				}
				else {
					long double buffer = pot(in, ++i);
					if (!buffer)ANS = NAN;
					else ANS /= buffer;
				}recording("/");
			}
			else if (in[i] == '%') {
				if (is_value(in + i + 1)) {
					long double buffer = pot(in, ++i);
					if (!buffer)ANS = NAN;
					else ANS = mod(ANS, buffer);
				}recording("%");
			}
			else return ANS;
		}
	};
	long double add(const char*in, int&i) {
		long double ANS = mul(in, i);
		while (1) {
			if (in[i] == '+') {
				if (!is_value(in + i + 1)) {
					ANS += 1;
					i++;
				}
				else ANS += mul(in, ++i);
				recording("+");
			}
			else if (in[i] == '-') {
				if (!is_value(in + i + 1)) {
					ANS -= 1;
					i++;
				}
				else ANS -= mul(in, ++i);
				recording("-");
			}
			else return ANS;
		}
	};

	void define(const char*in, long double val) {
		define(in);
		Var.back().second = val;
	}
	void define(const char*in) {
		if (is_Var(in)) {
			if (redef)
				undefine(in);//avoid redefinition of same var
			else return;
		}
		Var.push_back(std::make_pair<std::string, long double>(std::string(in), 0));
	}
	void undefine(const char*in) {
		msg.clear();
		if (delFunction(in))msg.emplace_back("deleted function");
		if (delOperator(in))msg.emplace_back("deleted operator");
		for (int i = constcount; i < Var.size(); i++)
			if (Var[i].first == std::string(in)) {
				Var.erase(Var.begin() + i--);
				msg.emplace_back("deleted Variable");
			}
	}
public:
	calculator() {
		Var.push_back(std::make_pair(std::string("PI"), M_PI));
		Var.push_back(std::make_pair(std::string("e"), M_E));
		Var.push_back(std::make_pair(std::string("TAU"), 2*M_PI));
		constcount = Var.size();
	}calculator(calculator&rhs){
		*this = rhs;
	}
	calculator&operator=(calculator&rhs) {
		constcount = rhs.constcount;
		Var = rhs.Var;
		udf = rhs.udf;
		udo = rhs.udo;
		return*this;
	}
	long double operator()(const char*in) { return calc(in); }
	long double calc(const char*in, bool clear_messages = true,bool record_=false) {
		set_true_on_destruction b(redef);
		//set redef to true when this function returns
		record = record_;
		if (std::string(in, in + 8) == "noredef:") { redef = 0; in += 8; }
		if (std::string(in, in + 9) == "record:on") { record = true; return 1; }
		if (std::string(in, in + 10) == "record:off") { if (recorder.back() == "[")recorder.pop_back(); record = false; return 1; }
		if (clear_messages) {
			msg.clear(); errors.clear(); recorder.clear();
		}
		int x = 0;
		for (int i = 0; in[i]; i++)
			if (in[i] == '(')x++;
			else if (in[i] == ')')x--;
		std::string l = in;

		if (std::string(in,in+4)=="def:"&&l.size()>4) {
			in += 4;
			if (not_from_child) {
				if (addFunction(in)) { msg.emplace_back("added function"); return 0; }
				if (addOperator(in)) { msg.emplace_back("added operator"); return 0; }
			}
			if (std::string(in).find('(') == std::string::npos) {
				define(in);
				msg.emplace_back("added variable");
			}
			return 0;
		}
		if (std::string(in, in + 6)=="undef:" && l.size()>6) {
			undefine(in+6);
			return 0;
		}
		int i = 0;
		long double buffer = 0;
		if(x>=0)buffer=add(in, i);



		else buffer =add((std::string(-x, '(') + in).c_str(), i);//insert x '(' at the begining of in , e.g in=="(1))" -> in=="((1))" , actual calculation



		if (in[i])
		{
			if (in[i++] == '='&&in[i++] == '>') {
				l = in + i;bool constant;
				if (std::string(in + i, in + i + 4) == "def:"&&l.size() > 4) {
					if (std::string(in).find('(') == std::string::npos) {
						define(in + i + 4, buffer);
						msg.emplace_back("added variable");
					}
					goto ret;//return;
				}
				auto v = is_Var(in + i, &constant);
				if (v&&!constant)
					v->second = buffer;
			}
			else if(in[i-1]!=']')
				errors.emplace_back("unexpected characters");
		}
	ret:
		return buffer;
	};


	bool deleteVar(const char*name) {
		for (int i = 4; i < Var.size(); i++)
			if (Var[i].first == std::string(name)) {
				Var.erase(Var.begin() + i--);
				return true;
			}
		return false;
	}
	bool setVar(const char*name, long double value) {
		bool is_var_const;
		auto a = is_Var(name, &is_var_const);
		if (a && !is_var_const) {
			a->second = value;
			return true;
		}
		return false;
	}
	bool addFunction(const char*f) {//input "f(x,y)=x+y"
		if (*f == '[')return 0;
		int name;
		std::vector<std::pair<int, int>>para;
		int fb,fe;
		int i = 0;
		udoperator* a=0;udfunction *b=0;
		std::shared_ptr<calculator>pC;
		const char* c;
		if ((a = is_udo(f)) || (b = is_udf(f))) {
			if (a) { c = f + a->name.size(); pC = a->calc; }
			else { c = f + b->name.size(); pC = b->calc; }
			if (*c == ':')f = c+1;
			else { a = 0; b = 0; pC = 0; }
		}

		for (; f[i]; i++) {
			if (f[i] == '(') { name = i; if (!name) break; goto next1; }
		}goto error;
	next1:
		for (int first = 0; f[i]; i++) {
			if (f[i] == ',') { if (!first)continue; para.emplace_back(std::make_pair(first, i)); first = 0; }
			else if (!first&&f[i] != ' '&&f[i] != ')'&&f[i] != '('&&f[i] != ',') { first = i; }
			if (f[i] == ')') { if (first)para.emplace_back(std::make_pair(first, i)); goto next2; }
		}goto error;
	next2:
		for (; f[i]; i++)if (f[i] == '=')goto next3;
		goto error;
	next3:
		fb = i + 1;
		for (int x = 1; f[i] && x; i++)
			if (f[i] == '[')x++;
			else if (f[i] == ']')x--;
			fe = i;
		{
			auto n= std::string(f, f + name);
			for (const auto&a : n)if (a == ':')goto error;
			bool force = redef;
			if ((a&&n == a->name) || (b&&n == b->name))force = 1;
			if (is_udf((n + ':').c_str()))if (force)delFunction(n.c_str()); else goto error;
			if (is_udo((n + ':').c_str()))if (force)delOperator(n.c_str()); else goto error;
			if (!pC){
				pC = std::make_shared<calculator>();
				pC->parent = this;
			}
			if (!pC) { errors.push_back("unexpected error"); return false; }
			auto func=std::string(f + fb, f + fe);
			std::vector<std::string>paralist;
			for (auto&p : para)
				paralist.emplace_back(f + p.first, f + p.second);
			udf.emplace_back();
			udfunction&a=udf.back();
			a.calc = pC;
			a.function = func;
			a.paralist = paralist;
			for (auto&p : paralist)
				a.calc->define(p.c_str());
			a.name = n;
			return 1;
		}
	error:
		return 0;
	}
	bool addOperator(const char*f){
		if (*f == '[')return 0;
		std::pair<int, int>para1,para2,name;
		int fb,fe;
		int i = 0;

		udoperator* a = 0; udfunction *b = 0;
		std::shared_ptr<calculator>pC;
		const char* c;
		if ((a = is_udo(f)) || (b = is_udf(f))) {
			if (a) { c = f + a->name.size(); pC = a->calc; }
			else { c = f + b->name.size(); pC = b->calc; }
			if (*c == ':')f = c + 1;
			else { a = 0; b = 0; pC = 0; }
		}

		for (; f[i]; i++) {
			if (f[i] == ' ') { para1 = std::make_pair(0, i); goto next1; }
		}goto error;
	next1:
		for (; f[i] && f[i] == ' '; i++);
		for (int p=i; f[i]; i++) {
			if (f[i] == ' ') { name = std::make_pair(p, i); goto next2; }
		}goto error;
	next2:
		for (; f[i] && f[i] == ' '; i++);
		for (int p=i; f[i]; i++) {
			if (f[i] == ' '||f[i]=='=') { para2 = std::make_pair(p, i); goto next3; }
		}goto error;
	next3:
		for (; f[i]; i++)if (f[i] == '=')goto next4;
		goto error;
	next4:
		fb = i + 1;
		for (int x=1; f[i]&&x; i++)
			if(f[i]=='[')x++;
			else if (f[i] == ']')x--;
		fe = i;
		{
			if (!pC) {
				pC = std::make_shared<calculator>();
				pC->parent = this;
			}
			if (!pC) { errors.push_back("unexpected error"); return false; }
			auto n=std::string(f+name.first,f+name.second);
			for (const auto&a : n)if (a == ':')goto error;
			bool force = redef;
			if ((a&&n == a->name) || (b&&n == b->name))force = 1;
			if (is_udo((n + ':').c_str()))if (force)delOperator(n.c_str()); else goto error;
			if (is_udf((n + ':').c_str()))if (force)delFunction(n.c_str()); else goto error;
			udo.push_back({});
			udoperator&a=udo.back();
			a.calc = pC;
			a.name = n;
			a.paraFirst =std::string( f+para1.first,f+para1.second);
			a.paraSecond = std::string(f + para2.first, f + para2.second);
			a.function = std::string(f + fb, f + fe);
			a.calc->define(a.paraFirst.c_str(), 0);
			a.calc->define(a.paraSecond.c_str(), 0);
			return 1;
		}
	error:;
		return 0;
	}
	bool delFunction(const char*in) {
		for (auto& a : udf)
			if (std::string(in) == a.name) {
				//delete a.calc.ptr();
				udf.erase(udf.begin() + (&a - udf.begin()._Ptr));
				return true;
			}
		return false;
	}
	bool delOperator(const char*in) {
		for (auto& a : udo)
			if (std::string(in) == a.name) {
				//delete a.calc.ptr();
				udo.erase(udo.begin() + (&a - udo.begin()._Ptr));
				return true;
			}
		return false;
	}
};
const std::vector<std::string> calculator::functions{
	"cos", "sin", "tan", "acos", "asin", "atan", "ln"
};