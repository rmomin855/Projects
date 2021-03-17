#include <pch.h>
#include "Projects/ProjectTwo.h"
#include "P2_Pathfinding.h"

#pragma region Extra Credit
bool ProjectTwo::implemented_floyd_warshall()
{
  return false;
}

bool ProjectTwo::implemented_goal_bounding()
{
  return false;
}

bool ProjectTwo::implemented_jps_plus()
{
  return false;
}
#pragma endregion

bool AStarPather::initialize()
{
  // handle any one-time setup requirements you have

  /*
      If you want to do any map-preprocessing, you'll need to listen
      for the map change message.  It'll look something like this:

      Callback cb = std::bind(&AStarPather::your_function_name, this);
      Messenger::listen_for_message(Messages::map_CHANGE, cb);

      There are other alternatives to using std::bind, so feel free to mix it up.
      Callback is just a typedef for std::function<void(void)>, so any std::invoke'able
      object that std::function can wrap will suffice.
  */
  //openlist.reserve(1600);
  rootTwo = std::sqrt(2.0);
  return true; // return false if any errors actually occur, to stop engine initialization
}

void AStarPather::shutdown()
{
  /*
      Free any dynamically allocated memory or any other general house-
      keeping you need to do during shutdown.
  */
}

PathResult AStarPather::compute_path(PathRequest& request)
{
  /*
      This is where you handle pathing requests, each request has several fields:

      start/goal - start and goal world positions
      path - where you will build the path upon completion, path should be
          start to goal, not goal to start
      heuristic - which heuristic calculation to use
      weight - the heuristic weight to be applied
      newRequest - whether this is the first request for this path, should generally
          be true, unless single step is on

      smoothing - whether to apply smoothing to the path
      rubberBanding - whether to apply rubber banding
      singleStep - whether to perform only a single A* step
      debugColoring - whether to color the grid based on the A* state:
          closed list nodes - yellow
          open list nodes - blue

          use terrain->set_color(row, col, Colors::YourColor);
          also it can be helpful to temporarily use other colors for specific states
          when you are testing your algorithms

      method - which algorithm to use: A*, Floyd-Warshall, JPS+, or goal bounding,
          will be A* generally, unless you implement extra credit features

      The return values are:
          PROCESSING - a path hasn't been found yet, should only be returned in
              single step mode until a path is found
          COMPLETE - a path to the goal was found and has been built in request.path
          IMPOSSIBLE - a path from start to goal does not exist, do not add start position to path
  */

  // WRITE YOUR CODE HERE
  if (request.newRequest)
  {
    request.newRequest = false;
    Reset();

    width = terrain->get_map_width();
    height = terrain->get_map_height();

    start = terrain->get_grid_position(request.start);
    goal = terrain->get_grid_position(request.goal);
    startPoint = terrain->get_world_position(start);
    settings = request.settings;

    map[start.row][start.col].status = Inlist;

    Node node(start.row, start.col, 0.0f);
    //openlist.push_back(node);
    openlist.push(node);
  }

  while (openlist.size())
  {
    //get smallest cost
    Node curr = openlist.top();
    openlist.pop();

    GridPos pos;
    pos.row = curr.row;
    pos.col = curr.col;

    if (settings.debugColoring)
    {
      terrain->set_color(pos, Colors::Yellow);
    }

    if (pos == goal)
    {
      CreatePath(request.path);
      return PathResult::COMPLETE;
    }

    //check neighbours and add
    AddNeighbours(pos, map[pos.row][pos.col].cost);
    map[pos.row][pos.col].status = Closed;

    if (request.settings.singleStep)
    {
      return PathResult::PROCESSING;
    }
  }

  return PathResult::IMPOSSIBLE;
}

void AStarPather::AddNeighbours(GridPos& curr, double currCost)
{
  GridPos up = curr;
  ++up.row;
  bool upWall = (up.row >= height) ? true : terrain->is_wall(up);

  GridPos right = curr;
  ++right.col;
  bool rightWall = (right.col >= width) ? true : terrain->is_wall(right);

  GridPos down = curr;
  --down.row;
  bool downWall = (down.row < 0) ? true : terrain->is_wall(down);

  GridPos left = curr;
  --left.col;
  bool leftWall = (left.col < 0) ? true : terrain->is_wall(left);

  //top tile
  if (up.row < height && !upWall)
  {
    AddtoOpenList(up, curr, currCost + 1.0f);
  }

  //bottom tile
  if (down.row > -1 && !downWall)
  {
    AddtoOpenList(down, curr, currCost + 1.0f);
  }

  //right tile
  if (right.col < width && !rightWall)
  {
    AddtoOpenList(right, curr, currCost + 1.0f);
  }

  //left tile
  if (left.col > -1 && !leftWall && !terrain->is_wall(left))
  {
    AddtoOpenList(left, curr, currCost + 1.0f);
  }

  //top right tile
  ++up.col;
  if (up.row < height && up.col < width && !upWall && !rightWall && !terrain->is_wall(up))
  {
    AddtoOpenList(up, curr, currCost + rootTwo);
  }

  //top left tile
  up.col -= 2;
  if (up.row < height && up.col > -1 && !upWall && !leftWall && !terrain->is_wall(up))
  {
    AddtoOpenList(up, curr, currCost + rootTwo);
  }

  //bottom right tile
  --right.row;
  if (right.col < width && right.row > -1 && !rightWall && !downWall && !terrain->is_wall(right))
  {
    AddtoOpenList(right, curr, currCost + rootTwo);
  }

  //bottom left tile
  --down.col;
  if (down.row > -1 && down.col > -1 && !downWall && !leftWall && !terrain->is_wall(down))
  {
    AddtoOpenList(down, curr, currCost + rootTwo);
  }
}

void AStarPather::AddtoOpenList(GridPos& curr, GridPos& parent, double cost)
{
  //calculate total cost
  double Fcost = cost + (CaculateHeuristic(curr) * settings.weight);

  //if already processed but now with better cost
  if (map[curr.row][curr.col].status == Closed && Fcost >= map[curr.row][curr.col].Fcost)
  {
    return;
  }
  //currently in open list
  else if (map[curr.row][curr.col].status == Inlist)
  {
    //if cost is better
    if (Fcost < map[curr.row][curr.col].Fcost)
    {
      map[curr.row][curr.col].parentRow = parent.row;
      map[curr.row][curr.col].parentCol = parent.col;

      map[curr.row][curr.col].cost = cost;
      map[curr.row][curr.col].Fcost = Fcost;

      //find in last and update cost
     /* for (size_t i = 0; i < openlist.size(); i++)
      {
        if (openlist[i].x == curr.col && openlist[i].y == curr.row)
        {
          openlist[i].cost = Fcost;
          break;
        }
      }*/

      std::priority_queue<Node, std::vector<Node>, std::greater<Node>> temp;
      while (openlist.size())
      {
        Node top = openlist.top();
        openlist.pop();

        if (top.row == curr.row && top.col == curr.col)
        {
          top.cost = Fcost;
        }

        temp.push(top);
      }

      openlist.swap(temp);
    }

    return;
  }

  //add to openlist
  map[curr.row][curr.col].status = Inlist;
  map[curr.row][curr.col].parentRow = parent.row;
  map[curr.row][curr.col].parentCol = parent.col;

  map[curr.row][curr.col].cost = cost;
  map[curr.row][curr.col].Fcost = Fcost;

  Node node(curr.row, curr.col, Fcost);
  //openlist.push_back(node);
  openlist.push(node);

  if (settings.debugColoring)
  {
    terrain->set_color(curr, Colors::Blue);
  }
}

void AStarPather::Reset()
{
  //openlist.clear();
  openlist = std::priority_queue<Node, std::vector<Node>, std::greater<Node>>();

  for (size_t i = 0; i < 40; i++)
  {
    for (size_t j = 0; j < 40; j++)
    {
      map[i][j].cost = 0.0;
      map[i][j].Fcost = FLT_MAX;
      map[i][j].status = Free;
    }
  }

  GridPos zero;
  zero.col = 0;
  zero.row = 0;
  GridPos diag;
  diag.col = 1;
  diag.row = 0;
  maxLenSQ = Vec3::Distance(terrain->get_world_position(zero), terrain->get_world_position(diag));
  maxLenSQ *= 1.5;
}

double AStarPather::CaculateHeuristic(GridPos& curr)
{
  double dy = double(std::abs(goal.row - curr.row));
  double dx = double(std::abs(goal.col - curr.col));

  switch (settings.heuristic)
  {
  case Heuristic::OCTILE:
    return ((rootTwo * std::min(dx, dy)) + std::max(dx, dy) - std::min(dx, dy)) * 1.000001;
  case Heuristic::CHEBYSHEV:
    return std::max(dx, dy);
  case Heuristic::MANHATTAN:
    return dx + dy;
  case Heuristic::EUCLIDEAN:
    return std::sqrt((dx * dx) + (dy * dy));
  }

  return 0.0;
}

void AStarPather::CreatePath(WaypointList& path)
{
  GridPos pos = goal;

  std::vector<GridPos> posPath;

  while (pos != start)
  {
    posPath.push_back(pos);
    Tile& tile = map[pos.row][pos.col];
    pos.row = tile.parentRow;
    pos.col = tile.parentCol;
  }

  posPath.push_back(start);

  if (settings.rubberBanding)
  {
    RubberBand(posPath);
  }

  for (size_t i = 0; i < posPath.size(); i++)
  {
    if (posPath[i].col != -1)
    {
      path.push_back(terrain->get_world_position(posPath[i]));
    }
  }

  path.reverse();

  if (settings.smoothing)
  {
    Smooth(path);
  }
}

void AStarPather::RubberBand(std::vector<GridPos>& path)
{
  size_t size = path.size() - 1;

  for (size_t i = 1; i < size; i++)
  {
    size_t c = i - 1;

    while (path[c].col == -1)
    {
      --c;
    }

    GridPos prev = path[c];
    GridPos next = path[i + 1];

    if (RemovePoint(prev, next))
    {
      path[i].col = -1;
    }
  }
}

void AStarPather::Smooth(WaypointList& path)
{
  AddPoints(path);

  path.push_front(path.front());
  path.push_back(path.back());

  std::list<Vec3> temp;

  auto curr = path.begin();
  ++curr;
  auto end = path.end();
  --end; --end;

  while (curr != end)
  {
    auto x1 = curr; --x1;
    auto x2 = curr;
    auto x3 = x2; ++x3;
    auto x4 = x3; ++x4;

    temp.push_back(*curr);
    temp.push_back(Vec3::CatmullRom(*curr, *x2, *x3, *x4, 0.25f));
    temp.push_back(Vec3::CatmullRom(*curr, *x2, *x3, *x4, 0.5f));
    temp.push_back(Vec3::CatmullRom(*curr, *x2, *x3, *x4, 0.75f));

    ++curr;
  }

  temp.push_back(path.back());

  path = temp;
}

bool AStarPather::RemovePoint(GridPos& prev, GridPos& next)
{
  int dx = next.col - prev.col;
  int dy = next.row - prev.row;

  dx = (dx == 0) ? 0 : dx / std::abs(dx);
  dy = (dy == 0) ? 0 : dy / std::abs(dy);

  GridPos curr;
  curr.col = prev.col;
  curr.row = prev.row;

  if (dx == 0)
  {
    for (int j = prev.row; j != (next.row + dy); j += dy)
    {
      curr.row = j;

      if (terrain->is_wall(curr))
      {
        return false;
      }
    }
  }
  else if (dy == 0)
  {
    for (int i = prev.col; i != (next.col + dx); i += dx)
    {
      curr.col = i;

      if (terrain->is_wall(curr))
      {
        return false;
      }
    }
  }
  else
  {
    for (int i = prev.col; i != (next.col + dx); i += dx)
    {
      curr.col = i;
      for (int j = prev.row; j != (next.row + dy); j += dy)
      {
        curr.row = j;

        if (terrain->is_wall(curr))
        {
          return false;
        }
      }
    }
  }

  return true;
}

void AStarPather::AddPoints(WaypointList& path)
{
  auto curr = path.begin();
  auto end = path.end();
  --end;

  while (curr != end)
  {
    auto next = curr; ++next;
    double dist = Vec3::Distance(*curr, *next);

    if (dist > maxLenSQ)
    {
      Vec3 mid = (*curr + *next) / 2.0f;
      path.insert(next, mid);
      continue;
    }

    ++curr;
  }
}
