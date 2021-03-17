#pragma once
#include "Misc/PathfindingDetails.hpp"
#include <queue>

enum List
{
  Free = 0,
  Inlist,
  Closed,
};

struct Tile
{
  Tile() : parentRow(-1), parentCol(-1), status(Free), cost(0), Fcost(DBL_MAX)
  {}

  int parentRow, parentCol;
  double cost;
  double Fcost;
  List status;
};

struct Node
{
  Node(int Row, int Col, double Cost = DBL_MAX) :col(Col), row(Row), cost(Cost) {}
  int col, row;
  double cost;

  friend bool operator<(const Node& lhs, const Node& rhs)
  {
    return(lhs.cost < rhs.cost);
  }
  friend bool operator>(const Node& lhs, const Node& rhs)
  {
    return(lhs.cost > rhs.cost);
  }
};

class AStarPather
{
public:
  /*
      The class should be default constructible, so you may need to define a constructor.
      If needed, you can modify the framework where the class is constructed in the
      initialize functions of ProjectTwo and ProjectThree.
  */

  /* ************************************************** */
  // DO NOT MODIFY THESE SIGNATURES
  bool initialize();
  void shutdown();
  PathResult compute_path(PathRequest& request);
  /* ************************************************** */

  /*
      You should create whatever functions, variables, or classes you need.
      It doesn't all need to be in this header and cpp, structure it whatever way
      makes sense to you.
  */

  void AddNeighbours(GridPos& curr, double currCost);
  void AddtoOpenList(GridPos& curr, GridPos& parent, double cost);
  void Reset();
  double CaculateHeuristic(GridPos& curr);

  void CreatePath(WaypointList& path);
  void RubberBand(std::vector<GridPos>& path);
  void Smooth(WaypointList& path);

  bool RemovePoint(GridPos& prev, GridPos& next);
  void AddPoints(WaypointList& path);

  double rootTwo;
  double maxLenSQ;
  GridPos start, goal;
  Vec3 startPoint;
  int width, height;
  PathRequest::Settings settings;

  Tile map[40][40];
  //std::vector<Node> openlist;

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openlist;
};