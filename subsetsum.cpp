#include <iostream>
#include <vector>
#include "subsetsum.h"


using namespace std;


void find_solution(int i, int j, vector<vector<int> >& M, vector<Core_wire>& cw, vector<int>& result){
    
    int wi = cw[i-1].width;
    if(i == 0 || j == 0) return;
    else if(wi > j)//item too heavy
        find_solution(i-1, j, M, cw, result);
    else        
    {
        if(M[i-1][j] < wi + M[i-1][j- wi]){
            find_solution(i-1, j-wi, M, cw, result);
            result.push_back(cw[i-1].core);
        }
        else
            find_solution(i-1, j, M, cw, result);

    }    
}    

void subset_sum(int W, int n, vector<Core_wire>& cw, vector<int>& result){
    vector <vector <int> > M (n+1, vector<int>(W+1,0));
    int wi;
    for(int i=1; i<= n; i++){
        wi = cw[i-1].width;
        for(int j=1; j<= W; j++){
            if(wi > j)
                M[i][j] = M[i-1][j];
            else{
                if(M[i-1][j] < wi + M[i-1][j-wi])
                    M[i][j] =  wi + M[i-1][j-wi];
                else
                    M[i][j] = M[i-1][j];
            }
            //cout << M[i][j] << " ";
            //if(j==W) cout<<endl;
        }
    }
    find_solution(n, W , M, cw, result);
}
/*
int main(){
    
    int W = 11;
    int n = 5;
    vector<int> items;
    items.push_back(1);
    items.push_back(2);
    items.push_back(5);
    items.push_back(6);
    items.push_back(7);
    vector<int> result;

    subset_sum(W, n, items, result);
    
    for(int i=0; i<result.size(); i++)
        cout<<" "<< result[i];
    cout<<endl;
    
    return 0;
} 
*/
