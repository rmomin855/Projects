#include <pch.h>
#include "Terrain/TerrainAnalysis.h"
#include "Terrain/MapMath.h"
#include "Agent/AStarAgent.h"
#include "Terrain/MapLayer.h"
#include "Projects/ProjectThree.h"

#include <iostream>

bool ProjectThree::implemented_fog_of_war() const // extra credit
{
  return false;
}

bool isBoundary(int row, int col, int height, int width)
{
  return (row == 0 || col == 0 || row == (height - 1) || col == (width - 1));
}

bool isOutsideBoundary(int row, int col, int height, int width)
{
  return (row < 0 || col < 0 || row >= height || col >= width);
}

float distance_to_closest_wall(int row, int col)
{
  /*
      Check the euclidean distance from the given cell to every other wall cell,
      with cells outside the map bounds treated as walls, and return the smallest
      distance.  Make use of the is_valid_grid_position and is_wall member
      functions in the global terrain to determine if a cell is within map bounds
      and a wall, respectively.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  if (isBoundary(row, col, height, width))
  {
    return 1.0f;
  }

  int bottomRow = row - 1;
  int bottomCol = col - 1;
  int topRow = row + 1;
  int topCol = col + 1;

  float dist = FLT_MAX;
  bool wall = false;

  while (!wall)
  {
    //only check the bottom row tiles
    for (int i = bottomCol; i <= topCol; ++i)
    {
      GridPos curr(bottomRow, i);

      if (isOutsideBoundary(curr.row, curr.col, height, width) || terrain->is_wall(curr))
      {
        float currDist = std::powf(float(row - curr.row), 2.0f) + std::powf(float(col - curr.col), 2.0f);

        if (currDist < dist)
        {
          dist = currDist;
        }

        wall = true;
      }
    }

    //only check the top row tiles
    for (int i = bottomCol; i <= topCol; ++i)
    {
      GridPos curr(topRow, i);

      if (isOutsideBoundary(curr.row, curr.col, height, width) || terrain->is_wall(curr))
      {
        float currDist = std::powf(float(row - curr.row), 2.0f) + std::powf(float(col - curr.col), 2.0f);

        if (currDist < dist)
        {
          dist = currDist;
        }

        wall = true;
      }
    }

    //only check the left col tiles
    for (int i = bottomRow; i <= topRow; ++i)
    {
      GridPos curr(i, bottomCol);

      if (isOutsideBoundary(curr.row, curr.col, height, width) || terrain->is_wall(curr))
      {
        float currDist = std::powf(float(row - curr.row), 2.0f) + std::powf(float(col - curr.col), 2.0f);

        if (currDist < dist)
        {
          dist = currDist;
        }

        wall = true;
      }
    }

    //only check the left col tiles
    for (int i = bottomRow; i <= topRow; ++i)
    {
      GridPos curr(i, topCol);

      if (isOutsideBoundary(curr.row, curr.col, height, width) || terrain->is_wall(curr))
      {
        float currDist = std::powf(float(row - curr.row), 2.0f) + std::powf(float(col - curr.col), 2.0f);

        if (currDist < dist)
        {
          dist = currDist;
        }

        wall = true;
      }
    }

    //increase boundry
    --bottomRow;
    --bottomCol;
    ++topRow;
    ++topCol;
  }

  return std::sqrtf(dist);
}

bool is_clear_path(int row0, int col0, int row1, int col1)
{
  /*
      Two cells (row0, col0) and (row1, col1) are visible to each other if a line
      between their centerpoints doesn't intersect the four boundary lines of every
      wall cell.  You should puff out the four boundary lines by a very tiny amount
      so that a diagonal line passing by the corner will intersect it.  Make use of the
      line_intersect helper function for the intersection test and the is_wall member
      function in the global terrain to determine if a cell is a wall or not.
  */

  // WRITE YOUR CODE HERE
  Vec3 l1p1 = terrain->get_world_position(row0, col0);
  l1p1.y = l1p1.z;
  Vec3 l1p2 = terrain->get_world_position(row1, col1);
  l1p2.y = l1p2.z;
  Vec2 L1P1 = l1p1;
  Vec2 L1P2 = l1p2;

  Vec3 BlockSize = terrain->get_world_position(1, 1) - terrain->get_world_position(0, 0);
  BlockSize *= 0.5f;
  BlockSize.y = BlockSize.z;
  Vec2 blockSize = BlockSize;
  blockSize *= 1.001f;

  int dx = col1 - col0;
  int dy = row1 - row0;

  dx = (dx == 0) ? 0 : dx / std::abs(dx);
  dy = (dy == 0) ? 0 : dy / std::abs(dy);

  if (dx == 0)
  {
    int col = col0;
    for (int row = row0; row != (row1 + dy); row += dy)
    {
      if (terrain->is_wall(row, col))
      {
        //check for collisions
        Vec3 POS = terrain->get_world_position(row, col);
        POS.y = POS.z;
        Vec2 pos = POS;

        Vec2 topRight = pos + blockSize;
        Vec2 Bottomleft = pos - blockSize;
        Vec2 topLeft = pos; topLeft.x -= blockSize.x; topLeft.y += blockSize.y;
        Vec2 bottomRight = pos; bottomRight.x += blockSize.x; bottomRight.y -= blockSize.y;

        if (line_intersect(L1P1, L1P2, topLeft, topRight) ||
          line_intersect(L1P1, L1P2, topRight, bottomRight) ||
          line_intersect(L1P1, L1P2, bottomRight, Bottomleft) ||
          line_intersect(L1P1, L1P2, Bottomleft, topLeft))
        {
          return false;
        }
      }
    }
  }
  else if (dy == 0)
  {
    int row = row0;
    for (int col = col0; col != (col1 + dx); col += dx)
    {
      if (terrain->is_wall(row, col))
      {
        //check for collisions
        Vec3 POS = terrain->get_world_position(row, col);
        POS.y = POS.z;
        Vec2 pos = POS;

        Vec2 topRight = pos + blockSize;
        Vec2 Bottomleft = pos - blockSize;
        Vec2 topLeft = pos; topLeft.x -= blockSize.x; topLeft.y += blockSize.y;
        Vec2 bottomRight = pos; bottomRight.x += blockSize.x; bottomRight.y -= blockSize.y;

        if (line_intersect(L1P1, L1P2, topLeft, topRight) ||
          line_intersect(L1P1, L1P2, topRight, bottomRight) ||
          line_intersect(L1P1, L1P2, bottomRight, Bottomleft) ||
          line_intersect(L1P1, L1P2, Bottomleft, topLeft))
        {
          return false;
        }
      }
    }
  }
  else
  {
    for (int row = row0; row != (row1 + dy); row += dy)
    {
      for (int col = col0; col != (col1 + dx); col += dx)
      {
        if (terrain->is_wall(row, col))
        {
          //check for collisions
          Vec3 POS = terrain->get_world_position(row, col);
          POS.y = POS.z;
          Vec2 pos = POS;

          Vec2 topRight = pos + blockSize;
          Vec2 Bottomleft = pos - blockSize;
          Vec2 topLeft = pos; topLeft.x -= blockSize.x; topLeft.y += blockSize.y;
          Vec2 bottomRight = pos; bottomRight.x += blockSize.x; bottomRight.y -= blockSize.y;

          if (line_intersect(L1P1, L1P2, topLeft, topRight) ||
            line_intersect(L1P1, L1P2, topRight, bottomRight) ||
            line_intersect(L1P1, L1P2, bottomRight, Bottomleft) ||
            line_intersect(L1P1, L1P2, Bottomleft, topLeft))
          {
            return false;
          }
        }
      }
    }

  }

  return true; // REPLACE THIS
}

void analyze_openness(MapLayer<float>& layer)
{
  /*
      Mark every cell in the given layer with the value 1 / (d * d),
      where d is the distance to the closest wall or edge.  Make use of the
      distance_to_closest_wall helper function.  Walls should not be marked.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }

      float dist = distance_to_closest_wall(row, col);
      float val = 1.0f / (dist * dist);
      layer.set_value(row, col, val);
    }
  }
}

int CheckVisibility(int row, int col, int width, int height)
{
  int total = 0;

  for (int row1 = 0; row1 < height; ++row1)
  {
    for (int col1 = 0; col1 < width; ++col1)
    {
      if (row == row1 && col == col1)
      {
        continue;
      }

      if (is_clear_path(row, col, row1, col1))
      {
        ++total;
      }
    }
  }

  return total;
}

void analyze_visibility(MapLayer<float>& layer)
{
  /*
      Mark every cell in the given layer with the number of cells that
      are visible to it, divided by 160 (a magic number that looks good).  Make sure
      to cap the value at 1.0 as well.

      Two cells are visible to each other if a line between their centerpoints doesn't
      intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
      helper function.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }

      int numVisible = CheckVisibility(row, col, width, height);
      float val = float(numVisible) / 160.0f;
      val = val > 1.0f ? 1.0f : val;

      layer.set_value(row, col, val);
    }
  }
}

void ShadeSurrounding(MapLayer<float>& layer, int Row, int Col)
{
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  GridPos up(Row, Col);
  ++up.row;
  bool upWall = (up.row >= height) ? true : terrain->is_wall(up);

  GridPos right(Row, Col);
  ++right.col;
  bool rightWall = (right.col >= width) ? true : terrain->is_wall(right);

  GridPos down(Row, Col);
  --down.row;
  bool downWall = (down.row < 0) ? true : terrain->is_wall(down);

  GridPos left(Row, Col);
  --left.col;
  bool leftWall = (left.col < 0) ? true : terrain->is_wall(left);

  //top tile
  if (up.row < height && !upWall)
  {
    float val = layer.get_value(up);
    if (layer.get_value(up) < 0.5f)
    {
      layer.set_value(up, 0.5f);
    }
  }

  //bottom tile
  if (down.row > -1 && !downWall)
  {
    float val = layer.get_value(down);
    if (layer.get_value(down) < 0.5f)
    {
      layer.set_value(down, 0.5f);
    }
  }

  //right tile
  if (right.col < width && !rightWall)
  {
    float val = layer.get_value(right);
    if (layer.get_value(right) < 0.5f)
    {
      layer.set_value(right, 0.5f);
    }
  }

  //left tile
  if (left.col > -1 && !leftWall && !terrain->is_wall(left))
  {
    float val = layer.get_value(left);
    if (layer.get_value(left) < 0.5f)
    {
      layer.set_value(left, 0.5f);
    }
  }

  //top right tile
  ++up.col;
  if (up.row < height && up.col < width && !upWall && !rightWall && !terrain->is_wall(up))
  {
    if (layer.get_value(up) < 0.5f)
    {
      layer.set_value(up, 0.5f);
    }
  }

  //top left tile
  up.col -= 2;
  if (up.row < height && up.col > -1 && !upWall && !leftWall && !terrain->is_wall(up))
  {
    if (layer.get_value(up) < 0.5f)
    {
      layer.set_value(up, 0.5f);
    }
  }

  //bottom right tile
  --right.row;
  if (right.col < width && right.row > -1 && !rightWall && !downWall && !terrain->is_wall(right))
  {
    if (layer.get_value(right) < 0.5f)
    {
      layer.set_value(right, 0.5f);
    }
  }

  //bottom left tile
  --down.col;
  if (down.row > -1 && down.col > -1 && !downWall && !leftWall && !terrain->is_wall(down))
  {
    if (layer.get_value(down) < 0.5f)
    {
      layer.set_value(down, 0.5f);
    }
  }
}

void analyze_visible_to_cell(MapLayer<float>& layer, int Row, int Col)
{
  /*
      For every cell in the given layer mark it with 1.0
      if it is visible to the given cell, 0.5 if it isn't visible but is next to a visible cell,
      or 0.0 otherwise.

      Two cells are visible to each other if a line between their centerpoints doesn't
      intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
      helper function.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      layer.set_value(row, col, 0.0f);
    }
  }

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }

      if (row == Row && col == Col)
      {
        layer.set_value(row, col, 1.0f);
        continue;
      }

      if (is_clear_path(Row, Col, row, col))
      {
        layer.set_value(row, col, 1.0f);
        ShadeSurrounding(layer, row, col);
      }
    }
  }
}

void analyze_agent_vision(MapLayer<float>& layer, const Agent* agent)
{
  /*
      For every cell in the given layer that is visible to the given agent,
      mark it as 1.0, otherwise don't change the cell's current value.

      You must consider the direction the agent is facing.  All of the agent data is
      in three dimensions, but to simplify you should operate in two dimensions, the XZ plane.

      Take the dot product between the view vector and the vector from the agent to the cell,
      both normalized, and compare the cosines directly instead of taking the arccosine to
      avoid introducing floating-point inaccuracy (larger cosine means smaller angle).

      Give the agent a field of view slighter larger than 180 degrees.

      Two cells are visible to each other if a line between their centerpoints doesn't
      intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
      helper function.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  Vec3 pos = agent->get_position();
  Vec3 forwardDir = agent->get_forward_vector();
  GridPos coord = terrain->get_grid_position(pos);

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }

      if (row == coord.row && col == coord.col)
      {
        layer.set_value(row, col, 1.0f);
        continue;
      }

      Vec3 tilePos = terrain->get_world_position(row, col);
      float dirDot = forwardDir.Dot(tilePos - pos);

      if (dirDot < 0.0001f)
      {
        continue;
      }

      if (is_clear_path(coord.row, coord.col, row, col))
      {
        layer.set_value(row, col, 1.0f);
      }
    }
  }
}

float GetMaxSurroundingVal(MapLayer<float>& layer, int Row, int Col, float decay)
{
  float max = 0.0f;
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  GridPos up(Row, Col);
  ++up.row;

  GridPos right(Row, Col);
  ++right.col;

  GridPos down(Row, Col);
  --down.row;

  GridPos left(Row, Col);
  --left.col;
  bool upWall = (up.row >= height) ? true : terrain->is_wall(up);
  bool rightWall = (right.col >= width) ? true : terrain->is_wall(right);
  bool downWall = (down.row < 0) ? true : terrain->is_wall(down);
  bool leftWall = (left.col < 0) ? true : terrain->is_wall(left);


  //Old_Influence* exp(-1 * Distance * Decay_Factor)

  //top tile
  if (up.row < height && !upWall)
  {
    float val = layer.get_value(up);
    val = val * exp(-1.0f * decay);

    if (max < val)
    {
      max = val;
    }
  }

  //bottom tile
  if (down.row > -1 && !downWall)
  {
    float val = layer.get_value(down);
    val = val * exp(-1.0f * decay);

    if (max < val)
    {
      max = val;
    }
  }

  //right tile
  if (right.col < width && !rightWall)
  {
    float val = layer.get_value(right);
    val = val * exp(-1.0f * decay);

    if (max < val)
    {
      max = val;
    }
  }

  //left tile
  if (left.col > -1 && !leftWall && !terrain->is_wall(left))
  {
    float val = layer.get_value(left);
    val = val * exp(-1.0f * decay);

    if (max < val)
    {
      max = val;
    }
  }

  float rootTwo = sqrtf(2.0f);

  //top right tile
  ++up.col;
  if (up.row < height && up.col < width && !upWall && !rightWall && !terrain->is_wall(up))
  {
    float val = layer.get_value(up);
    val = val * exp(-rootTwo * decay);

    if (max < val)
    {
      max = val;
    }
  }

  //top left tile
  up.col -= 2;
  if (up.row < height && up.col > -1 && !upWall && !leftWall && !terrain->is_wall(up))
  {
    float val = layer.get_value(up);
    val = val * exp(-rootTwo * decay);

    if (max < val)
    {
      max = val;
    }
  }

  //bottom right tile
  --right.row;
  if (right.col < width && right.row > -1 && !rightWall && !downWall && !terrain->is_wall(right))
  {
    float val = layer.get_value(right);
    val = val * exp(-rootTwo * decay);

    if (max < val)
    {
      max = val;
    }
  }

  //bottom left tile
  --down.col;
  if (down.row > -1 && down.col > -1 && !downWall && !leftWall && !terrain->is_wall(down))
  {
    float val = layer.get_value(down);
    val = val * exp(-rootTwo * decay);

    if (max < val)
    {
      max = val;
    }
  }

  return max;
}

void propagate_solo_occupancy(MapLayer<float>& layer, float decay, float growth)
{
  /*
      For every cell in the given layer:

          1) Get the value of each neighbor and apply decay factor
          2) Keep the highest value from step 1
          3) Linearly interpolate from the cell's current value to the value from step 2
             with the growing factor as a coefficient.  Make use of the lerp helper function.
          4) Store the value from step 3 in a temporary layer.
             A float[40][40] will suffice, no need to dynamically allocate or make a new MapLayer.

      After every cell has been processed into the temporary layer, write the temporary layer into
      the given layer;
  */

  // WRITE YOUR CODE HERE
  float tempLayer[40][40] = {};
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }

      float maxVal = GetMaxSurroundingVal(layer, row, col, decay);
      float currVal = layer.get_value(row, col);
      float val = lerp(currVal, maxVal, growth);
      tempLayer[row][col] = val;
    }
  }

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      layer.set_value(row, col, tempLayer[row][col]);
    }
  }
}

void propagate_dual_occupancy(MapLayer<float>& layer, float decay, float growth)
{
  /*
      Similar to the solo version, but the values range from -1.0 to 1.0, instead of 0.0 to 1.0

      For every cell in the given layer:

      1) Get the value of each neighbor and apply decay factor
      2) Keep the highest ABSOLUTE value from step 1
      3) Linearly interpolate from the cell's current value to the value from step 2
         with the growing factor as a coefficient.  Make use of the lerp helper function.
      4) Store the value from step 3 in a temporary layer.
         A float[40][40] will suffice, no need to dynamically allocate or make a new MapLayer.

      After every cell has been processed into the temporary layer, write the temporary layer into
      the given layer;
  */

  // WRITE YOUR CODE HERE
}

void normalize_solo_occupancy(MapLayer<float>& layer)
{
  /*
      Determine the maximum value in the given layer, and then divide the value
      for every cell in the layer by that amount.  This will keep the values in the
      range of [0, 1].  Negative values should be left unmodified.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  float max = -FLT_MAX;
  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      float val = layer.get_value(row, col);
      if (val > max)
      {
        max = val;
      }
    }
  }

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      float val = layer.get_value(row, col);
      if (val > 0.0f)
      {
        val /= max;;
      }

      layer.set_value(row, col, val);
    }
  }
}

void normalize_dual_occupancy(MapLayer<float>& layer)
{
  /*
      Similar to the solo version, but you need to track greatest positive value AND
      the least (furthest from 0) negative value.

      For every cell in the given layer, if the value is currently positive divide it by the
      greatest positive value, or if the value is negative divide it by -1.0 * the least negative value
      (so that it remains a negative number).  This will keep the values in the range of [-1, 1].
  */

  // WRITE YOUR CODE HERE
}

float getDist(int row, int col, int prow, int pcol)
{
  float dist = 0.0f;
  float rootTwo = sqrtf(2.0f);

  int diffX = abs(col - pcol);
  int diffY = abs(row - prow);

  int min = std::min(diffX, diffY);
  dist += min * rootTwo;

  diffX -= min;
  diffY -= min;

  dist += diffX;
  dist += diffY;

  return dist;
}

void enemy_field_of_view(MapLayer<float>& layer, float fovAngle, float closeDistance, float occupancyValue, AStarAgent* enemy)
{
  /*
      First, clear out the old values in the map layer by setting any negative value to 0.
      Then, for every cell in the layer that is within the field of view cone, from the
      enemy agent, mark it with the occupancy value.  Take the dot product between the view
      vector and the vector from the agent to the cell, both normalized, and compare the
      cosines directly instead of taking the arccosine to avoid introducing floating-point
      inaccuracy (larger cosine means smaller angle).

      If the tile is close enough to the enemy (less than closeDistance),
      you only check if it's visible to enemy.  Make use of the is_clear_path
      helper function.  Otherwise, you must consider the direction the enemy is facing too.
      This creates a radius around the enemy that the player can be detected within, as well
      as a fov cone.
  */

  // WRITE YOUR CODE HERE
  int width = terrain->get_map_width();
  int height = terrain->get_map_height();
  fovAngle /= 2.0f;
  float cosFOV = 1.0f - abs(cosf(fovAngle * (PI / 180.0f)));

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (layer.get_value(row, col) < 0.0f)
      {
        layer.set_value(row, col, 0.0f);
      }
    }
  }

  Vec3 pos = enemy->get_position();
  Vec3 forwardDir = enemy->get_forward_vector();
  forwardDir.Normalize();
  GridPos playerPos = terrain->get_grid_position(pos);

  //do FOV thingy
  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }
      Vec3 tilePos = terrain->get_world_position(row, col);
      Vec3 dir = tilePos - pos;

      float dist = getDist(playerPos.row, playerPos.col, row, col);
      if ((dist < closeDistance) && is_clear_path(playerPos.row, playerPos.col, row, col))
      {
        layer.set_value(row, col, occupancyValue);
        continue;
      }

      dir.Normalize();

      float dotVal = 1.0f - dir.Dot(forwardDir);

      if ((dotVal <= cosFOV) && is_clear_path(playerPos.row, playerPos.col, row, col))
      {
        layer.set_value(row, col, occupancyValue);
      }
    }
  }
}

bool enemy_find_player(MapLayer<float>& layer, AStarAgent* enemy, Agent* player)
{
  /*
      Check if the player's current tile has a negative value, ie in the fov cone
      or within a detection radius.
  */

  const auto& playerWorldPos = player->get_position();

  const auto playerGridPos = terrain->get_grid_position(playerWorldPos);

  // verify a valid position was returned
  if (terrain->is_valid_grid_position(playerGridPos) == true)
  {
    if (layer.get_value(playerGridPos) < 0.0f)
    {
      return true;
    }
  }

  // player isn't in the detection radius or fov cone, OR somehow off the map
  return false;
}

bool enemy_seek_player(MapLayer<float>& layer, AStarAgent* enemy)
{
  /*
      Attempt to find a cell with the highest nonzero value (normalization may
      not produce exactly 1.0 due to floating point error), and then set it as
      the new target, using enemy->path_to.

      If there are multiple cells with the same highest value, then pick the
      cell closest to the enemy.

      Return whether a target cell was found.
  */

  // WRITE YOUR CODE HERE
  float max = 0.0f;
  float dist = FLT_MAX;
  int targetCol = -1;
  int targetRow = -1;

  int width = terrain->get_map_width();
  int height = terrain->get_map_height();

  Vec3 pos = enemy->get_position();
  GridPos gridPos = terrain->get_grid_position(pos);

  for (int col = 0; col < width; ++col)
  {
    for (int row = 0; row < height; ++row)
    {
      if (terrain->is_wall(row, col))
      {
        continue;
      }

      float val = layer.get_value(row, col);

      if (val == 0.0f)
      {
        continue;
      }

      Vec3 tilePos = terrain->get_world_position(row, col);

      if (val > max)
      {
        max = val;
        dist = Vec3::Distance(tilePos, pos);
        targetCol = col;
        targetRow = row;
      }
      else if (val == max)
      {
        float tempDist = Vec3::Distance(tilePos, pos);

        if (tempDist < dist)
        {
          dist = tempDist;
          targetCol = col;
          targetRow = row;
        }
      }
    }
  }

  if (targetCol == -1 || targetRow == -1)
  {
    return false;
  }

  enemy->path_to(terrain->get_world_position(targetRow, targetCol));
  return true;
}
