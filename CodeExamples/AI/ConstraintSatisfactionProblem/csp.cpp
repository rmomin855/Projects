#include "csp.h"
#include <limits.h>

#ifdef INLINE_CSP
//#warning "INFO - inlining CSP methods"
#define INLINE inline
#else   
//#warning "INFO - NOT inlining CSP methods"
#define INLINE 
#endif

////////////////////////////////////////////////////////////
//CSP constructor
template <typename T>
CSP<T>::CSP(T& cg) :
  arc_consistency(),
  cg(cg),
  solution_counter(0),
  recursive_call_counter(0),
  iteration_counter(0)
{
}

////////////////////////////////////////////////////////////
//CSP solver, brute force - no forward checking
template <typename T>
bool CSP<T>::SolveDFS(unsigned level)
{
  ++recursive_call_counter;

  if (cg.AllVariablesAssigned())
  {
    return true;
  }

  //choose a variable by MRV
  Variable* var_to_assign = MaxDegreeHeuristic();

  const std::set<Value>& domain = var_to_assign->GetDomain();
  auto curr = domain.begin();
  auto end = domain.end();

  while (curr != end)
  {
    ++iteration_counter;

    var_to_assign->Assign(*curr);
    const typename std::vector<const Constraint*>& constraints = cg.GetConstraints(var_to_assign);
    size_t constraintsSize = constraints.size();
    bool next = true;

    for (size_t j = 0; j < constraintsSize; ++j)
    {
      if (!constraints[j]->Satisfiable())
      {
        next = false;
      }
    }

    if (next)
    {
      if (SolveDFS(level + 1))
      {
        return true;
      }
    }

    var_to_assign->UnAssign();
    ++curr;
  }

  return false;
}


////////////////////////////////////////////////////////////
//CSP solver, uses forward checking
template <typename T>
bool CSP<T>::SolveFC(unsigned level)
{
  ++recursive_call_counter;

  if (cg.AllVariablesAssigned())
  {
    return true;
  }

  //choose a variable by MRV
  Variable* var_to_assign = MinRemVal();

  const std::set<Value>& domain = var_to_assign->GetDomain();
  auto curr = domain.begin();
  auto end = domain.end();

  while (curr != end)
  {
    ++iteration_counter;

    var_to_assign->Assign(*curr);

    auto savedVals = SaveState(var_to_assign);

    if (ForwardChecking(var_to_assign))
    {
      if (SolveFC(level + 1))
      {
        return true;
      }
    }

    LoadState(savedVals);

    var_to_assign->UnAssign();
    ++curr;
  }

  return false;
}
////////////////////////////////////////////////////////////
//CSP solver, uses arc consistency
template <typename T>
bool CSP<T>::SolveARC(unsigned level)
{
  level = level;
  return false;
}


template <typename T>
INLINE
bool CSP<T>::ForwardChecking(Variable* x)
{
  const typename std::set<Variable*>& variables = cg.GetNeighbors(x);
  auto neighbourCurr = variables.begin();
  auto neighbourEnd = variables.end();

  while (neighbourCurr != neighbourEnd)
  {
    if ((*neighbourCurr)->IsAssigned() || (*neighbourCurr) == x)
    {
      ++neighbourCurr;
      continue;
    }

    const typename std::set<const Constraint*>& connecting = cg.GetConnectingConstraints(x, *neighbourCurr);

    const std::set<Value>& domain = (*neighbourCurr)->GetDomain();
    auto curr = domain.begin();
    auto end = domain.end();

    std::vector<Value> toRemove;

    //for all current values in domain
    while (curr != end)
    {
      (*neighbourCurr)->Assign(*curr);
      auto connectingCurr = connecting.begin();
      auto connectingEnd = connecting.end();
      //for all constraints
      while (connectingCurr != connectingEnd)
      {
        //if constraint is not Satisfiable with current values, add to remove value vector
        if (!(*connectingCurr)->Satisfiable())
        {
          toRemove.push_back(*curr);
          break;
        }

        ++connectingCurr;
      }

      (*neighbourCurr)->UnAssign();
      ++curr;
    }


    //remove all values that are in vector from domain so only valid values remain
    size_t toRemoveSize = toRemove.size();
    for (size_t j = 0; j < toRemoveSize; ++j)
    {
      (*neighbourCurr)->RemoveValue(toRemove[j]);
    }

    //if variable domain is empty, solution cannot be found
    if ((*neighbourCurr)->IsImpossible())
    {
      return false;
    }

    ++neighbourCurr;
  }

  return true;
}
////////////////////////////////////////////////////////////
//load states (available values) of all unassigned variables 
template <typename T>
void CSP<T>::LoadState(
  std::map<Variable*,
  std::set<typename CSP<T>::Variable::Value> >& saved) const
{
  typename std::map<Variable*, std::set<typename Variable::Value> >::iterator
    b_result = saved.begin();
  typename std::map<Variable*, std::set<typename Variable::Value> >::iterator
    e_result = saved.end();

  for (; b_result != e_result; ++b_result) {
    //std::cout << "loading state for " 
    //<< b_result->first->Name() << std::endl;
    (*b_result).first->SetDomain((*b_result).second);
  }
}


////////////////////////////////////////////////////////////
//save states (available values) of all unassigned variables 
//except the current
template <typename T>
INLINE
std::map< typename CSP<T>::Variable*, std::set<typename CSP<T>::Variable::Value> >
CSP<T>::SaveState(typename CSP<T>::Variable* x) const {
  std::map<Variable*, std::set<typename Variable::Value> > result;

  const std::set<Variable*>& all_vars = cg.GetNeighbors(x);
  typename std::set<Variable*>::const_iterator
    b_all_vars = all_vars.begin();
  typename std::set<Variable*>::const_iterator
    e_all_vars = all_vars.end();
  for (; b_all_vars != e_all_vars; ++b_all_vars) {
    if (!(*b_all_vars)->IsAssigned() && *b_all_vars != x) {
      //std::cout << "saving state for " 
      //<< (*b_all_vars)->Name() << std::endl;
      result[*b_all_vars] = (*b_all_vars)->GetDomain();
    }
  }
  return result;
}
////////////////////////////////////////////////////////////
//check the current (incomplete) assignment for satisfiability
template <typename T>
INLINE
bool CSP<T>::AssignmentIsConsistent(Variable* p_var) const
{
  return false;
}
////////////////////////////////////////////////////////////
//insert pair 
//(neighbors of the current variable, the current variable)
//current variable is th variable that just lost some values
// for all y~x insert (y,x)
//into arc-consistency queue
template <typename T>
INLINE
void CSP<T>::InsertAllArcsTo(Variable* cv)
{}
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//AIMA p.146 AC-3 algorithm
template <typename T>
INLINE
bool CSP<T>::CheckArcConsistency(Variable* x)
{
  return false;
}
////////////////////////////////////////////////////////////
//CHECK that for each value of x there is a value of y 
//which makes all constraints involving x and y satisfiable
template <typename T>
INLINE
bool CSP<T>::RemoveInconsistentValues(Variable* x, Variable* y, const Constraint* c)
{
  return false;
}

////////////////////////////////////////////////////////////
//choose next variable for assignment
//choose the one with minimum remaining values
template <typename T>
INLINE
typename CSP<T>::Variable* CSP<T>::MinRemVal()
{
  const typename std::vector<Variable*>& variables = cg.GetAllVariables();
  size_t size = variables.size();

  std::vector<Value> assigned;
  for (size_t i = 0; i < size; i++)
  {
    if (variables[i]->IsAssigned())
    {
      assigned.push_back(variables[i]->GetValue());
    }
  }
  auto assignedBegin = assigned.begin();
  auto assignedEnd = assigned.end();

  int currMin = INT_MAX;
  Variable* minVar = nullptr;

  for (size_t i = 0; i < size; ++i)
  {
    if (!variables[i]->IsAssigned())
    {
      const std::set<Value>& domain = variables[i]->GetDomain();
      auto curr = domain.begin();
      auto end = domain.end();
      int valid = 0;

      while (curr != end)
      {
        if (std::find(assignedBegin, assignedEnd, *curr) == assignedEnd)
        {
          ++valid;
        }

        ++curr;
      }

      if (valid && valid < currMin)
      {
        currMin = valid;
        minVar = variables[i];
      }
    }
  }

  return minVar;
}
////////////////////////////////////////////////////////////
//choose next variable for assignment
//choose the one with max degree
template <typename T>
typename CSP<T>::Variable* CSP<T>::MaxDegreeHeuristic()
{
  const typename std::vector<Variable*>& variables = cg.GetAllVariables();
  size_t size = variables.size();

  std::vector<Value> assigned;
  for (size_t i = 0; i < size; i++)
  {
    if (variables[i]->IsAssigned())
    {
      assigned.push_back(variables[i]->GetValue());
    }
  }
  auto assignedBegin = assigned.begin();
  auto assignedEnd = assigned.end();

  int currMin = INT_MAX;
  Variable* minVar = nullptr;
  size_t constraintSize = 0;

  for (size_t i = 0; i < size; ++i)
  {
    if (!variables[i]->IsAssigned())
    {
      const std::set<Value>& domain = variables[i]->GetDomain();
      auto curr = domain.begin();
      auto end = domain.end();
      int valid = 0;

      while (curr != end)
      {
        if (std::find(assignedBegin, assignedEnd, *curr) == assignedEnd)
        {
          ++valid;
        }

        ++curr;
      }

      if (valid && valid < currMin)
      {
        currMin = valid;
        minVar = variables[i];
        constraintSize = cg.GetConstraints(minVar).size();
      }
      else if (valid && valid == currMin)
      {
        if (cg.GetConstraints(variables[i]).size() > constraintSize)
        {
          minVar = variables[i];
        }
      }
    }
  }

  return minVar;
}
#undef INLINE
