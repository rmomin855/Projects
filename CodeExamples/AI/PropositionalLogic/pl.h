#ifndef PL4_H
#define PL4_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

class Literal {
public:
  Literal(std::string const& _name) : name(_name), negated(false) { }
  Literal() : name(""), negated(false) { } // just for map.operator[]
  ////////////////////////////////////////////////////////////////////////
  Literal& Negate() { negated = !negated; return *this; }
  bool IsPositive() const { return !negated; }
  ////////////////////////////////////////////////////////////////////////
  bool operator==(Literal const& op2) const {
    Literal const& op1 = *this;
    return (op1.negated == op2.negated) && (op1.name == op2.name);
  }
  ////////////////////////////////////////////////////////////////////////
  bool operator<(Literal const& op2) const {
    Literal const& op1 = *this;
    //negated infront
    if (op1.negated && !op2.negated) {
      return true;
    }
    if (!op1.negated && op2.negated) {
      return false;
    }
    return (op1.name < op2.name);
  }
  ////////////////////////////////////////////////////////////////////////
  Literal operator~() const {
    Literal result(*this);
    result.Negate();
    return result;
  }
  ////////////////////////////////////////////////////////////////////////
  bool Complementary(Literal const& op2) const {
    return (name == op2.name) && (negated != op2.negated);
  }
  ////////////////////////////////////////////////////////////////////////
  friend std::ostream& operator<<(std::ostream& os, Literal const& literal) {
    os << (literal.negated ? "~" : "") << literal.name;
    return os;
  }

  int GetVal() const
  {
    int sum = 0;
    size_t size = name.size();

    for (size_t i = 0; i < size; ++i)
    {
      sum += name[i];
    }

    if (negated)
    {
      sum *= -21;
    }

    return sum;
  }

private:
  std::string name;
  bool negated;
};

class Clause {
public:
  Clause(Literal const& op2)
  {
    literals.insert(op2);
  }
  // ..........
  // ..........
  // ..........
  // ..........
  ////////////////////////////////////////////////////////////////////////
  friend std::ostream& operator<<(std::ostream& os, Clause const& clause) {
    unsigned size = clause.literals.size();

    if (size == 0) {
      os << " FALSE ";
    }
    else {
      std::set< Literal >::const_iterator it = clause.literals.begin();
      os << "( " << *it;
      ++it;
      for (; it != clause.literals.end(); ++it) {
        os << " | " << *it;
      }
      os << " ) ";
    }
    return os;
  }

  friend bool operator<(Clause const& clause1, Clause const& clause2)
  {
    auto curr1 = clause1.literals.begin();
    auto end1 = clause1.literals.end();

    auto curr2 = clause2.literals.begin();
    auto end2 = clause2.literals.end();

    int sum1 = 0;

    while (curr1 != end1)
    {
      sum1 += curr1->GetVal();
      ++curr1;
    }

    int sum2 = 0;

    while (curr2 != end2)
    {
      sum2 += curr2->GetVal();
      ++curr2;
    }

    if (sum1 != sum2)
    {
      return sum1 < sum2;
    }

    return false;
  }

  const std::set< Literal >& getLiterals() const
  {
    return literals;
  }

  std::set< Literal >& getLiterals()
  {
    return literals;
  }

  friend bool operator==(const Clause& lhs, const Clause& rhs)
  {
    auto curr = lhs.literals.begin();
    auto end = lhs.literals.end();

    auto currRhs = rhs.literals.begin();
    auto endRhs = rhs.literals.end();

    while (curr != end && currRhs != endRhs)
    {
      if (!(*curr == *currRhs))
      {
        return false;
      }

      ++curr;
      ++currRhs;
    }

    if (curr != end || currRhs != endRhs)
    {
      return false;
    }

    return true;
  }

  void ORClause(const Clause& rhs)
  {
    literals.insert(rhs.literals.begin(), rhs.literals.end());
  }

  int Size() const
  {
    return literals.size();
  }

  std::set<Literal>::const_iterator Begin() const
  {
    return literals.begin();
  }

  std::set<Literal>::const_iterator End() const
  {
    return literals.end();
  }

private:
  std::set< Literal > literals;
};

class CNF
{
public:
  CNF() = default;

  CNF(Literal const& op2)
  {
    clauses.insert(Clause(op2));
  }

  ////////////////////////////////////////////////////////////////////////
  // not
  CNF const operator~() const {
    //if CNF is made of a single clause: A | B | ~C,
    //negating it gives ~A & ~B & C (3 clauses)
    //otherwise
    //CNF = clause1 & clause2 & clause3,
    //~CNF = ~clause1 | ~clause2 | ~clause3 
    //"or" is defined later 
    CNF newCNF;

    size_t CNFsize = size();
    if (CNFsize == 1)
    {
      const std::set< Literal >& literals = clauses.begin()->getLiterals();

      auto curr = literals.begin();
      auto end = literals.end();

      while (curr != end)
      {
        Literal temp = *curr;
        temp.Negate();
        newCNF.clauses.insert(temp);

        ++curr;
      }

      return newCNF;
    }

    auto curr = clauses.begin();
    auto end = clauses.end();

    newCNF = NotClause(*curr);
    ++curr;

    while (curr != end)
    {
      //NEGATE CLAUSES
      newCNF = newCNF | NotClause(*curr);
      ++curr;
    }

    return newCNF;
  }

  CNF NotClause(const Clause& temp) const
  {
    const std::set<Literal>& literals = temp.getLiterals();
    auto curr = literals.begin();
    auto end = literals.end();

    CNF newCNF;

    while (curr != end)
    {
      Literal temp = *curr;
      newCNF.clauses.insert(temp.Negate());
      ++curr;
    }

    return newCNF;
  }
  ////////////////////////////////////////////////////////////////////////
  // =>
  CNF const operator>(CNF const& op2) const {
    CNF const& op1 = *this;
    return ~(op1) | op2;
  }
  ////////////////////////////////////////////////////////////////////////
  // and
  CNF const operator&(CNF const& op2) const {
    //CNF1 = clause1 & clause2 & clause3,
    //CNF2 = clause4 & clause5 & clause6,
    //CNF1 & CNF2 = clause1 & clause2 & clause3 & clause4 & clause5 & clause6

    CNF newCNF;
    newCNF.clauses = clauses;
    newCNF.clauses.insert(op2.begin(), op2.end());

    return newCNF;
  }
  ///////////////////////////////////////////////////////////////////////
  // or
  CNF const operator|(CNF const& op2) const
  {
    //CNF1 = clause1 & clause2 & clause3,
    //CNF2 = clause4 & clause5 & clause6,
    //CNF1 | CNF2 = 
    //              c1|c4 & c1|c5 & c1|c6    &
    //              c2|c4 & c2|c5 & c2|c6    &
    //              c3|c4 & c3|c5 & c3|c6

    if (!clauses.size())
    {
      return op2;
    }
    else if (!op2.clauses.size())
    {
      return *this;
    }

    CNF newCNF;

    auto curr1 = clauses.begin();
    auto end1 = clauses.end();

    while (curr1 != end1)
    {
      auto curr2 = op2.clauses.begin();
      auto end2 = op2.clauses.end();

      while (curr2 != end2)
      {
        Clause temp = *curr1;
        temp.ORClause(*curr2);

        newCNF.clauses.insert(temp);

        ++curr2;
      }

      ++curr1;
    }

    return newCNF;
  }

  /////////////////////////////////////////////////////////////////////////////////
  CNF const operator>(Literal const& op2) const { return operator>(CNF(op2)); }
  CNF const operator&(Literal const& op2) const { return operator&(CNF(op2)); }
  CNF const operator|(Literal const& op2) const { return operator|(CNF(op2)); }

  ////////////////////////////////////////////////////////////////////////
  bool Empty() const { return clauses.size() == 0; }
  ////////////////////////////////////////////////////////////////////////
  std::set< Clause >::const_iterator begin() const { return clauses.begin(); }
  std::set< Clause >::const_iterator end()   const { return clauses.end(); }
  unsigned                           size()  const { return clauses.size(); }
  ////////////////////////////////////////////////////////////////////////
  friend std::ostream& operator<<(std::ostream& os, CNF const& cnf) {
    unsigned size = cnf.clauses.size();
    for (std::set< Clause >::const_iterator it1 = cnf.clauses.begin(); it1 != cnf.clauses.end(); ++it1) {
      os << *it1 << ", ";
    }
    return os;
  }
private:
  std::set< Clause > clauses;
};

CNF const operator|(Literal const& op1, Literal const& op2);
CNF const operator|(Literal const& op1, CNF     const& op2);
CNF const operator&(Literal const& op1, Literal const& op2);
CNF const operator&(Literal const& op1, CNF     const& op2);
CNF const operator>(Literal const& op1, Literal const& op2);
CNF const operator>(Literal const& op1, CNF     const& op2);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class KnowledgeBase {
public:
  ////////////////////////////////////////////////////////////////////////////
  KnowledgeBase();
  ////////////////////////////////////////////////////////////////////////////
  KnowledgeBase& operator+=(CNF const& cnf);
  ////////////////////////////////////////////////////////////////////////
  std::set< Clause >::const_iterator begin() const;
  std::set< Clause >::const_iterator end()   const;
  unsigned                           size()  const;
  ////////////////////////////////////////////////////////////////////////////
  bool ProveByRefutation(CNF const& alpha);
  ////////////////////////////////////////////////////////////////////////////
  friend std::ostream& operator<<(std::ostream& os, KnowledgeBase const& kb);

  bool ResolveKB();
  int CheckForCompliments(const Literal& clause1, const Clause& clause2);
  void RemoveCompliments(Clause& clause);

  std::set<Clause> GetResolved(std::set<Clause>::iterator start);

private:
  std::set< Clause > clauses;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
