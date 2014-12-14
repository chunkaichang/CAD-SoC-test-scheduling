#ifndef SUBSETSUM_H
#define SUBSETSUM_H
#include <vector>
class Core_wire{
    public:
        int core;
        int width;
        int load;//total test length assigned on this resource
};

void find_solution(int i, int j, std::vector<std::vector<int> >& M, std::vector<Core_wire>& width, std::vector<int>& result);
void subset_sum(int W, int n, std::vector<Core_wire>& width, std::vector<int>& result);

#endif
