#include <stdio.h>
#include <iostream>
#include "log.h"
#include "objs.h"
#include "route.h"

std::vector<Node> nodes;
std::vector<Net> nets;

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("no input files\n");
    return -1;
  }

  FILE* fp1 = fopen(argv[1], "r");
  if (fp1 == NULL) {
    printf("can not open file %s\n", argv[1]);
    return -1;
  }

  log() << "DB: start reading data" << std::endl;

  int node_cnt = 0;
  fscanf(fp1, "nodes %d\n", &node_cnt);
  nodes.resize(node_cnt);
  for (int i = 0; i < node_cnt; i++) {
    int id, sx, sy, ex, ey;
    fscanf(fp1, "n%d %d %d %d %d\n", &id, &sx, &sy, &ex, &ey);
    new (&nodes[id]) Node(id, sx, sy, ex, ey);
  }

  int edge_cnt = 0;
  fscanf(fp1, "edges %d\n", &edge_cnt);
  for (int i = 0; i < edge_cnt; i++) {
    int id1, id2;
    fscanf(fp1, "n%d n%d\n", &id1, &id2);
    nodes[id1].nbs_.push_back(id2);
  }

  fclose(fp1);

  log() << "DB: add "
        << node_cnt << " nodes, "
        << edge_cnt << " edges"
        << std::endl;

  FILE* fp2 = fopen(argv[2], "r");
	if (fp2 == NULL) {
	  printf("can not open file %s\n", argv[2]);
	  return -1;
	}

	int net_cnt = 0;
  fscanf(fp2, "nets %d\n", &net_cnt);
  nets.resize(net_cnt);
  for (int i = 0; i < net_cnt; i++) {
    int id, net_node_cnt;
    fscanf(fp2, "net%d %d\n", &id, &net_node_cnt);
    nets[i].id_ = i;
    nets[i].pin_node_ids_.resize(net_node_cnt);
    for (int j = 0; j < net_node_cnt; j++) {
      int node_id;
      fscanf(fp2, "n%d\n", &node_id);
      nets[i].pin_node_ids_[j] = node_id;
    }
  }
	fclose(fp2);

	log() << "DB: add " << net_cnt << " nets" << std::endl;

	Route router;
	router.route();

	FILE* fp3 = fopen(argv[3], "w");
  if (fp3 == NULL) {
    printf("can not open file %s\n", argv[3]);
    return -1;
  }

  fprintf(fp3, "nets %d\n\n", net_cnt);
	for (int i = 0; i < net_cnt; i++) {
	  Net& net = nets[i];
	  fprintf(fp3, "net%d %d\n", i, net.route_edges_.size());
	  for (int j = 0; j < net.route_edges_.size(); j++) {
	    int id1 = net.route_edges_[j].first;
	    int id2 = net.route_edges_[j].second;
	    fprintf(fp3, "n%d -> n%d\n", id1, id2);
	  }
	  fprintf(fp3, "\n");
	}
	fclose(fp3);

  return 0;
}
