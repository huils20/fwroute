#ifndef ROUTE_H_
#define ROUTE_H_

#include <math.h>
#include <unordered_set>
#include "queue.h"

#define HUGE_FLOAT 1.e30
#define EPSILON 1.e-7

class PQNode {
 public:
  PQNode() : node_id_(-1) { }
  PQNode(const int node_id, const double path_cost, const double cost)
      : node_id_(node_id), path_cost_(path_cost), cost_(cost) { }

 public:
  int node_id_;
  double path_cost_;
  double cost_;
};

class PQNodeCompare {
 public:
  bool operator() (const PQNode& na, const PQNode& nb) const {
    //return na.cost_ > nb.cost_;
    return na.cost_ - nb.cost_ >=  EPSILON ||
           (fabs(nb.cost_ - na.cost_) < EPSILON &&
            nb.path_cost_ > na.path_cost_);
  }
};

typedef HeapPriorityQueue<PQNode, PQNodeCompare> pqueue_t;

class NodeInfo {
 public:
  NodeInfo() {
    prev_node_id_ = -1;
    path_cost_ = HUGE_FLOAT;
    occ_ = 0;
    cost_ = 1;
    acc_cost_ = 1;
  }

 public:
  int prev_node_id_;
  double path_cost_;

  int occ_;
  double cost_;
  double acc_cost_;
};

class NetInfo {
 public:
  int min_x_;
  int min_y_;
  int max_x_;
  int max_y_;
};

class Route {
 public:
  bool route();

 private:
  void init();
  bool isNetCongested(const int net_id);
  bool route(const int net_id, const int src_node_id, const int sink_node_id);
  bool routeOneNet(const int net_id, const double factor);
  void updateNetCost(const int net_id, const double factor, const int sign);
  void updateRouteCost(const double factor, const double acc_factor);
  int countOverflows();
  double getFutureCost(const int cur_node_id, const int target_node_id);

 private:
  pqueue_t priority_queue_;
  std::vector<NetInfo> net_infos_;
  std::vector<NodeInfo> node_infos_;
  std::vector<int> change_list_;
  std::unordered_set<int> partial_nodes_;
};

#endif
