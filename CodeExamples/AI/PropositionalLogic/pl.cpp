#include "pl.h"

CNF const operator|(Literal const& op1, Literal const& op2) { return CNF(op1) | CNF(op2); }
CNF const operator|(Literal const& op1, CNF     const& op2) { return CNF(op1) | op2; }
CNF const operator&(Literal const& op1, Literal const& op2) { return CNF(op1) & CNF(op2); }
CNF const operator&(Literal const& op1, CNF     const& op2) { return CNF(op1) & op2; }
CNF const operator>(Literal const& op1, Literal const& op2) { return CNF(op1) > CNF(op2); }
CNF const operator>(Literal const& op1, CNF     const& op2) { return CNF(op1) > op2; }


KnowledgeBase::KnowledgeBase() : clauses() {}
////////////////////////////////////////////////////////////////////////////
KnowledgeBase& KnowledgeBase::operator+=(CNF const& cnf) {
  for (std::set< Clause >::const_iterator it = cnf.begin(); it != cnf.end(); ++it) {
    std::pair<std::set<Clause>::iterator, bool> temp = clauses.insert(*it);
  }
  return *this;
}
////////////////////////////////////////////////////////////////////////
std::set< Clause >::const_iterator KnowledgeBase::begin() const { return clauses.begin(); }
std::set< Clause >::const_iterator KnowledgeBase::end()   const { return clauses.end(); }
unsigned                           KnowledgeBase::size()  const { return clauses.size(); }
////////////////////////////////////////////////////////////////////////////
bool KnowledgeBase::ProveByRefutation(CNF const& alpha)
{
  KnowledgeBase KB = *this;

  auto curr = alpha.begin();
  auto end = alpha.end();

  while (curr != end)
  {
    CNF notAlpha = alpha.NotClause(*curr);
    KB += notAlpha;

    ++curr;
  }

  return KB.ResolveKB();
}

std::set<Clause> KnowledgeBase::GetResolved(std::set<Clause>::iterator start)
{
  std::set<Clause> resolved;

  if (!clauses.size())
  {
    return resolved;
  }

  auto next = start; ++next;
  auto end = clauses.end();

  auto currL = start->Begin();
  auto endL = start->End();

  while (currL != endL)
  {
    while (next != end)
    {
      if (CheckForCompliments(*currL, *next))
      {
        Clause newClause = *next;
        Literal temp = *currL;
        temp.Negate();
        newClause.getLiterals().erase(temp);

        resolved.insert(newClause);
      }

      ++next;
    }

    ++currL;
  }

  return resolved;
}

bool KnowledgeBase::ResolveKB()
{
  std::set<Clause> considered;
  auto start = clauses.begin();
  auto end = clauses.end();

  while (start != end)
  {
    std::set<Clause> resolved = GetResolved(start);

    if (resolved.size())
    {
      considered.insert(*start);
      clauses.erase(start);

      auto curr = resolved.begin();
      auto end = resolved.end();

      while (curr != end)
      {
        if (!curr->Size())
        {
          return true;
        }

        if (considered.find(*curr) == considered.end())
        {
          std::pair<std::set<Clause>::iterator, bool> temp = clauses.insert(*curr);

          if (!temp.second)
          {
            //std::cout << "FAILED" << std::endl;
          }
        }

        ++curr;
      }

      start = clauses.begin();
      end = clauses.end();
    }
    else
    {
      ++start;
    }
  }

  return false;
}

int KnowledgeBase::CheckForCompliments(const Literal& clause1, const Clause& clause2)
{
  auto curr2 = clause2.Begin();
  auto end2 = clause2.End();

  while (curr2 != end2)
  {
    if (clause1.Complementary(*curr2))
    {
      return 1;
    }

    ++curr2;
  }

  return 0;
}

void KnowledgeBase::RemoveCompliments(Clause& clause)
{
  auto curr = clause.Begin();
  auto next = curr; ++next;
  auto end = clause.End();

  std::set< Literal >& literals = clause.getLiterals();

  while (next != end)
  {
    if (curr->Complementary(*next))
    {
      literals.erase(*curr);
      literals.erase(*next);

      curr = clause.Begin();
      end = clause.End();

      if (curr == end)
      {
        return;
      }

      next = curr; ++next;
      continue;
    }

    ++next;

    if (next == end)
    {
      ++curr;
      next = curr; ++next;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, KnowledgeBase const& kb) {
  unsigned size = kb.clauses.size();
  for (std::set< Clause >::const_iterator it1 = kb.clauses.begin(); it1 != kb.clauses.end(); ++it1) {
    os << *it1 << ", ";
  }
  return os;
}
