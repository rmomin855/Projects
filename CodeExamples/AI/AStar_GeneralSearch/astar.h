#ifndef ASTAR
#define ASTAR
#include <vector>
#include <queue>
#include <float.h>
#include <map>
#include <algorithm>

enum ListStatus
{
  None = 0,
  InList,
  Closed
};

//callback object for Astar
template <typename GraphType, typename AstarType>
class Callback {
protected:
  GraphType const& g;
public:
  Callback(GraphType const& _g) : g(_g) {}
  virtual ~Callback() {}
  virtual void OnIteration(AstarType const&) { }
  virtual void OnFinish(AstarType const&) { }
};

template <typename GraphType, typename Heuristic>
class Astar {
public:

  struct AStarNode
  {
    AStarNode() : id(-1), parentId(-1), Fcost(0.0f), cost(0.0f)
    {}
    AStarNode(size_t Id, size_t parent, float Cost = 0.0f, float FCost = 0.0f) : id(Id), parentId(parent), Fcost(FCost), cost(Cost)
    {}

    size_t id;
    size_t parentId;
    float Fcost;
    float cost;

    friend bool operator>(const AStarNode& lhs, const AStarNode& rhs)
    {
      return lhs.Fcost > rhs.Fcost;
    }

    friend bool operator<(const AStarNode& lhs, const AStarNode& rhs)
    {
      return lhs.Fcost < rhs.Fcost;
    }

    friend bool operator==(const AStarNode& lhs, const AStarNode& rhs)
    {
      return lhs.id == rhs.id;
    }
  };

  typedef std::vector<typename GraphType::Edge> SolutionContainer;
  typedef std::priority_queue < AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> OpenListContainer;
  typedef std::map<size_t, AStarNode> ClosedListContainer;

  template <class T, class S, class C>
  S& Container(std::priority_queue<T, S, C>& q)
  {
    struct HackedQueue : private std::priority_queue<T, S, C>
    {
      static S& Container(std::priority_queue<T, S, C>& q)
      {
        return q.* & HackedQueue::c;
      }
    };
    return HackedQueue::Container(q);
  }

  ////////////////////////////////////////////////////////////
  Astar(GraphType const& _graph, Callback<GraphType, Astar>& cb) :
    graph(_graph),
    callback(cb),
    openlist(),
    closedlist(),
    solution(),
    start_id(0),
    goal_id(0),
    goalVertex(),
    pathID(1024),
    tasks(Container(openlist))
  {}
  ////////////////////////////////////////////////////////////

  void Reset()
  {
    tasks.clear();

    closedlist.clear();
    solution.clear();
  }

  void CreatePath(AStarNode& goalNode)
  {
    AStarNode curr = goalNode;

    pathID.clear();

    while (curr.id != start_id)
    {
      pathID.push_back(curr.id);

      curr.id = curr.parentId;
      curr = closedlist[curr.id];
    }

    pathID.push_back(start_id);

    size_t pathSize = pathID.size();

    for (size_t i = pathSize - 1; i > 0; --i)
    {
      typename GraphType::Vertex const& vertex = graph.GetVertex(pathID[i]);
      std::vector<typename GraphType::Edge> const& outedges = graph.GetOutEdges(vertex);

      size_t outedges_size = outedges.size();
      size_t nextID = pathID[i - 1];
      for (size_t j = 0; j < outedges_size; ++j)
      {
        if (outedges[j].GetID2() == nextID)
        {
          solution.push_back(outedges[j]);
          break;
        }
      }
    }
  }

  bool CheckClosedList(AStarNode& node)
  {
    AStarNode& curr = closedlist[node.id];

    if (curr.id != size_t(-1))
    {
      if (node.Fcost < curr.Fcost)
      {
        curr.id = -1;
        openlist.push(node);
      }

      return true;
    }

    return false;
  }

  bool CheckOpenList(AStarNode& node)
  {
    //auto end = tasks.end();
    //auto openListPos = std::find(tasks.begin(), end, node);

    //if (openListPos != end)
    //{
    //  if (node.Fcost < (*openListPos).Fcost)
    //  {
    //    std::swap(*openListPos, tasks.back());
    //    tasks.pop_back();
    //    openlist.push(node);
    //  }

    //  return true;
    //}

    //return false;

    int size = int(tasks.size());

    if (!size)
    {
      return false;
    }

    int start = 0;
    int pos = size;
    int end = size - 1;

    while (start <= end)
    {
      if (tasks[start].id == node.id)
      {
        pos = start;
        break;
      }

      if (tasks[end].id == node.id)
      {
        pos = end;
        break;
      }

      ++start;
      --end;
    }

    if (pos != size)
    {
      if (node.Fcost < tasks[pos].Fcost)
      {
        std::swap(tasks[pos], tasks.back());
        tasks.pop_back();
        openlist.push(node);
      }

      return true;
    }

    return false;
  }

  void AddtoOpenList(AStarNode& node)
  {
    if (CheckClosedList(node))
    {
      return;
    }

    if (CheckOpenList(node))
    {
      return;
    }

    openlist.push(node);
  }

  void AddNeighbours(size_t id, float cost, Heuristic& heuristic)
  {
    typename GraphType::Vertex const& vertex = graph.GetVertex(id);
    std::vector<typename GraphType::Edge> const& outedges = graph.GetOutEdges(vertex);

    size_t outedges_size = outedges.size();
    for (size_t i = 0; i < outedges_size; ++i)
    {
      size_t nextID = outedges[i].GetID2();
      float gCost = cost + outedges[i].GetWeight();
      float hCost = heuristic(graph, graph.GetVertex(nextID), goalVertex);
      AStarNode curr(nextID, id, gCost, gCost + hCost);
      AddtoOpenList(curr);
    }
  }

  void AddToClosed(AStarNode& curr)
  {
    closedlist[curr.id] = curr;
  }

  ////////////////////////////////////////////////////////////
  std::vector<typename GraphType::Edge> search(size_t startID, size_t goalID)
  {
    start_id = startID;
    goal_id = goalID;
    Reset();
    Heuristic heuristic;
    goalVertex = graph.GetVertex(goal_id);

    AStarNode start(start_id, start_id);
    openlist.push(start);

    while (openlist.size() > 0)
    {
      callback.OnIteration(*this);

      AStarNode curr = openlist.top();
      openlist.pop();

      if (curr.id == goal_id)
      {
        CreatePath(curr);
        callback.OnFinish(*this);
        return solution;
      }

      AddNeighbours(curr.id, curr.cost, heuristic);
      AddToClosed(curr);
    }

    callback.OnFinish(*this);
    return solution;
  }
  ////////////////////////////////////////////////////////////////////////
private:
  // do not modify the next 2 lines
  const GraphType& graph;
  Callback<GraphType, Astar>& callback;
  // the next 4 lines are just sugestions
  // OpenListContainer, ClosedListContainer, SolutionContainer are typedefed
  OpenListContainer            openlist;
  ClosedListContainer          closedlist;
  SolutionContainer            solution;
  size_t                       start_id, goal_id;
  typename GraphType::Vertex goalVertex;
  std::vector<size_t> pathID;
  std::vector<AStarNode>& tasks;
};

#endif
