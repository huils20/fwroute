#ifndef OBJ_H_
#define OBJ_H_

#include <vector>

class Node {
 public:
  Node() { }
  Node(const int id, const int sx, const int sy, const int ex, const int ey) :
    id_(id), start_x_(sx), start_y_(sy), end_x_(ex), end_y_(ey) { }

 public:
  int id_;
  int start_x_;
  int start_y_;
  int end_x_;
  int end_y_;
  std::vector<int> nbs_;
};

class Net {
 public:
  int id_;
  std::vector<int> pin_node_ids_;
  std::vector<int> route_node_ids_;
  std::vector<std::pair<int, int> > route_edges_;
};

extern std::vector<Node> nodes;
extern std::vector<Net> nets;

#endif
