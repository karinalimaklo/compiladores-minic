#include<vector>
#include"util.hpp"
#include <vector>
namespace util {
std::vector<int> merge(std::vector<int>& vetor) {
    if (vetor.size() <= 1) {
        return vetor;
    }
    int tamanho = static_cast<int>(vetor.size());
    int meio = (tamanho/2);
    std::vector<int> prim_metade(vetor.begin(), vetor.begin() + meio);
    std::vector<int> seg_metade(vetor.begin() + meio, vetor.end());
    prim_metade = merge(prim_metade);
    seg_metade = merge(seg_metade);

    return sort(prim_metade, seg_metade);
}

std::vector<int> sort(const std::vector<int> a, std::vector<int>b) {
    std::vector<int> result;
    size_t i = 0;
    size_t j = 0;
    while(i<a.size() && j<b.size()) {
        if (a[i] < b[j]) {
            result.push_back(a[i]);
            i++;
        } else if(a[i] > b[j]) {
            result.push_back(b[j]);
            j++;
        } else {
            result.push_back(a[i]);
            i++;
            j++;
        }

    }
    while (i < a.size()) {          // sobras de a
        result.push_back(a[i]);
        i++;
    }
    while (j < b.size()) {          // sobras de b
        result.push_back(b[j]);
        j++;
    }
    return result;
}
}