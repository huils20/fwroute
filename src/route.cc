#include <stdio.h>
#include "log.h"
#include "objs.h"
#include "route.h"

bool Route::route() {
  log() << "Route: beign " << std::endl;

  init();

  double factor = 0.5;
  double acc_factor = 1;
  int max_iter_count = 12;

  log() << "Route: start iteration... " << std::endl;
  for (int iter = 1; iter <= max_iter_count; ++iter) {
    // route all nets one iteration
    for (int i = 0; i < (int)(nets.size()); i++) {
      bool success = routeOneNet(i, factor);
      if (!success) {
        return false;
      }
    }

    // count all overflows after all net routed
    int overflow_cnt = countOverflows();
    log() << "Route: iter = " << iter << ", overflow = " <<
        overflow_cnt << std::endl;
    if (overflow_cnt == 0) {
      break;
    }

    // increase cost for next iteration
    updateRouteCost(factor, acc_factor);
  }

  log() << "Route: end " << std::endl;
}

void Route::init() {
  node_infos_.resize(nodes.size());

  net_infos_.resize(nets.size());
}

bool Route::isNetCongested(const int net_id) {
  Net& net = nets[net_id];

  if (net.route_node_ids_.size() < 1)
    return true;

  for (size_t i = 0; i < net.route_node_ids_.size(); i++) {
    int id = net.route_node_ids_[i];
    if (node_infos_[id].occ_ > 1)
      return true;
  }
  return false;
}

int Route::countOverflows() {
  int cnt = 0;
  for (size_t i = 0; i < nets.size(); i++) {
    Net& net = nets[i];
    for (size_t j = 0; j < net.route_node_ids_.size(); j++) {
      int id = net.route_node_ids_[j];
      if (node_infos_[id].occ_ > 1) {
        //printf("overflow : %d\n", id);
        cnt++;
      }
    }
  }
  return cnt;
}

bool Route::route(const int net_id,
                  const int src_node_id,
                  const int sink_node_id) {
  Net& net = nets[net_id];

  if (net.route_node_ids_.size() > 0) {
    for (size_t i = 0; i < net.route_node_ids_.size(); i++) {
      int id = net.route_node_ids_[i];
      PQNode pq_node(id, 0, 0);
      priority_queue_.push(pq_node);
      node_infos_[id].path_cost_ = 0;
      change_list_.push_back(id);
    }
  } else {
    PQNode pq_node(src_node_id, 0, 0);
    priority_queue_.push(pq_node);
    node_infos_[src_node_id].path_cost_ = 0;
    change_list_.push_back(src_node_id);
  }

  double backward_path_cost = 0;
  int node_id = -1;
  while (!priority_queue_.empty()) {
    do {
      const PQNode& pq_node = priority_queue_.top();
      backward_path_cost = pq_node.path_cost_;
      node_id = pq_node.node_id_;
      priority_queue_.pop();
    } while (node_id == -1);

    if (node_id == sink_node_id) {
      break;
    }

    Node& node = nodes[node_id];
    for (size_t i = 0; i < node.nbs_.size(); i++) {
      int neighbor_id = node.nbs_[i];
      //if (isNodeOutOfNetBound(neighbor_id, net_id))
      //  continue;

      NodeInfo& node_info = node_infos_[neighbor_id];

      double path_cost = backward_path_cost;
      path_cost += node_info.cost_ * node_info.acc_cost_;
      if (node_info.path_cost_ > path_cost) {
        node_info.path_cost_ = path_cost;
        node_info.prev_node_id_ = node_id;
        change_list_.push_back(neighbor_id);
        double cost = path_cost + getFutureCost(neighbor_id, sink_node_id);
        PQNode pq_node(neighbor_id, path_cost, cost);
        priority_queue_.push(pq_node);
      }
    }
  }

  if (node_id != sink_node_id) {
    return false;
  }

  std::vector<int> this_path;
  this_path.push_back(sink_node_id);
  int id = sink_node_id;
  partial_nodes_.insert(id);
  int prev_id = -1;
  while (true) {
    prev_id = node_infos_[id].prev_node_id_;
    if (prev_id < 0 || partial_nodes_.find(prev_id) != partial_nodes_.end())
      break;
    this_path.push_back(prev_id);
    id = prev_id;
    partial_nodes_.insert(id);
  }
  std::reverse(this_path.begin(), this_path.end());
  net.route_node_ids_.insert(net.route_node_ids_.end(),
        this_path.begin(), this_path.end());
  if (prev_id >= 0) {
    this_path.insert(this_path.begin(), prev_id);
  }
  for (size_t i = 1; i < this_path.size(); i++) {
    net.route_edges_.emplace_back(this_path[i - 1], this_path[i]);
  }

  //printf("change_list_ = %d\n", change_list_.size());
  //fflush(stdout);
  for (size_t i = 0; i < change_list_.size(); i++) {
    int id = change_list_[i];
    node_infos_[id].prev_node_id_ = -1;
    node_infos_[id].path_cost_ = HUGE_FLOAT;
  }
  change_list_.clear();
  priority_queue_.clear();
  return true;
}

bool Route::routeOneNet(const int net_id, const double factor) {
  if (!isNetCongested(net_id)) {
    return true;
  }

  updateNetCost(net_id, factor, -1);

  Net& net = nets[net_id];
  net.route_node_ids_.clear();
  net.route_edges_.clear();

  int src_node_id = net.pin_node_ids_[0];
  for (int i = 1; i < net.pin_node_ids_.size(); ++i) {
    int sink_node_id = net.pin_node_ids_[i];
    bool success = route(net_id, src_node_id, sink_node_id);
    if (!success) {
      return false;
    }
  }

  partial_nodes_.clear();
  updateNetCost(net_id, factor, 1);
  return true;
}

void Route::updateNetCost(const int net_id,
                          const double factor,
                          const int sign) {
  Net& net = nets[net_id];
  for (size_t i = 0; i < net.route_node_ids_.size(); i++) {
    int id = net.route_node_ids_[i];
    NodeInfo& node_info = node_infos_[id];

    int occ = node_info.occ_ + sign;
    node_info.occ_ = std::max(occ, 0);

    int capacity = 1;
    if (occ < capacity) {
      node_info.cost_ = 1;
    } else {
      node_info.cost_ = 1 + (occ + 1 - capacity) * factor;
    }
  }
}

void Route::updateRouteCost(const double factor,
                            const double acc_factor) {
  for (size_t i = 0; i < nodes.size(); i++) {
    NodeInfo& node_info = node_infos_[i];
    int occ = node_info.occ_;
    int capacity = 1;
    if (occ > capacity) {
      double acc_cost = node_info.acc_cost_ + (occ - capacity) * acc_factor;
      node_info.acc_cost_ = acc_cost;
      node_info.cost_ = 1 + (occ + 1 - capacity) * factor;
    } else if (occ == capacity) {
      node_info.cost_ = 1 + factor;
    } else {
      node_info.cost_ = 1;
    }
  }
}

double Route::getFutureCost(const int cur_node_id, const int target_node_id) {
  Node& cur_node = nodes[cur_node_id];
  Node& target_node = nodes[target_node_id];
  int delta_x = abs(cur_node.end_x_ - target_node.start_x_);
  int delta_y = abs(cur_node.end_y_ - target_node.start_y_);

  int cost = 0;
  cost += delta_x / 12;
  cost += delta_y / 18;
  delta_x = delta_x % 12;
  delta_y = delta_y % 18;

  cost += delta_y / 6;
  delta_y = delta_y % 6;
  cost += delta_y / 2;
  delta_y = delta_y % 2;
  cost += delta_y;

  cost += delta_x / 4;
  delta_x = delta_x % 4;
  cost += delta_x / 2;
  delta_x = delta_x % 2;
  cost += delta_x;

  return cost;
}

