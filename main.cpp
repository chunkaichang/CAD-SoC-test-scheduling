#include <iostream>
#include <ctime>
#include <iomanip>
#include <stdlib.h>
#include "system.h"
using namespace std;

System S;
int main(int argc, char** argv)
{
    clock_t start, end;
    start = clock();

    if(argc != 3){
        cerr<< "Number of Input Arguments Mismatch!" <<endl;
        cerr<< "./cadb032 [test.in] [test.out]" <<endl;
    }
    // parsing input file
    if(S.parser(argv)) return 1;
    
    // check error information
    if(S.check_info_error(argv,0,"tmp")) return 0;
    
    // arrange TAM wires
    S.wire();
    // debug check input
    //S.pg_display();
    //S.display();
    
    // fill in table
    S.table_init();
    
    
    // initial in final_table
    S.final_table_init();
    
    // debug check table
    //S.table_display();
    //S.trash_table_display();
    
    // scheduling
    S.parallel_sched();
    
    // fill in trash table
    S.trash_table_init();
    
    // initial precedence
    S.preced_ini();
    S.trash_schedule();
    
    // Result 
    //S.display();
    
    // output results
    if(S.output(argv)) return 1;

    end = clock();
    
    cout << "Execution time: " << (end-start)/(double)(CLOCKS_PER_SEC) << endl;
    
    return 0;
}
