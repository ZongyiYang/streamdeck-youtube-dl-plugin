#ifndef __CSS_PARSER_PP_H__
#define __CSS_PARSER_PP_H__

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace htmlcxx {
namespace CSS {

extern const char *IE_CSS;
class Parser 
{

	public:
		friend class Attribute;

		enum PseudoClass { NONE_CLASS, LINK, VISITED, ACTIVE };
		enum PseudoElement { NONE_ELEMENT, FIRST_LETTER, FIRST_LINE };

		class Selector 
		{
			private:
				std::string mElement;
				std::string mId;
				std::string mEClass;
				PseudoClass mPsClass;
				PseudoElement mPsElement;

			public:
				Selector();
				Selector(const std::string& e, const std::string& i, const std::string& c, const PseudoClass& pc, const PseudoElement &pe);
				void setElement(const std::string &str);
				void setId(const std::string &str);
				void setClass(const std::string &str);
				void setPseudoClass(enum PseudoClass p);
				void setPseudoElement(enum PseudoElement p);
				bool match(const Selector& s) const;
				bool operator==(const Selector& s) const;
				bool operator<(const Selector& s) const;
				friend std::ostream& operator<<(std::ostream& out, const Selector& s);
		};

	private:
		static bool match(const std::vector<Selector>& selector, const std::vector<Selector>& path);

		class Attribute 
		{
			public:
				Attribute() {}
				Attribute(const std::string& v, bool i) : mVal(v), mImportant(i) {}
				std::string mVal;
				bool mImportant;
		};

	public:
		Parser() {}
		friend std::ostream& operator<<(std::ostream& out, const std::map<std::string, Parser::Attribute>& s);
		bool parse(const std::string& css);
		bool parse(const char *buf, int buf_len);
		void merge(const Parser& p);
		std::map<std::string, std::string> getAttributes(const std::vector<Selector>& sv) const;

		friend std::ostream& operator<<(std::ostream& out, const CSS::Parser& p);

	private:
		typedef std::map<std::vector<Selector>, std::map<std::string, Attribute> > RuleSet;
		RuleSet mRuleSets;
};

std::string pse2str(const enum Parser::PseudoElement& s);
std::string psc2str(const enum Parser::PseudoClass& s);
std::ostream& operator<<(std::ostream& out, const std::map<std::string, Parser::Attribute>& s);

}//namespace CSS
}//namespace htmlcxx
#endif
