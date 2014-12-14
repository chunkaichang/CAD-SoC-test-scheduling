/*
 * Change log:
 * new class "pre","str_vec_t_and_length"
 * modify System.displat()
 * new function preced_ini()
 */
#ifndef SYSTEM_H
#define SYSTEM_H
#include <iostream>
#include <vector>
#include <string>


typedef std::vector<std::string> str_vec_t;

class Test{
public:    
    int length;
    int power;
    int partition;
    std::vector <int> time_begin;
    std::vector <int> time_end;
    std::string rsc;
    std::string name;
    Test():length(0), power(0), partition(0)
    {   
    }    
    void display(){
        std::cout<<name<<" L "<<length<<" P "<<power<<" "<<rsc <<" " << partition<<std::endl;
    }    
};

typedef std::vector<Test> test_vec_t;

class Core{
public:    
    int width;
    int rsc_id;
    int start_line;
    int load;
    std::string name;
	 
    test_vec_t tam;
    std::vector<test_vec_t> bist;
    test_vec_t free_bist;//BIST test without specific rsc
    test_vec_t trash_tam;
    std::vector<test_vec_t> trash_bist;
    test_vec_t trash_free_bist;//BIST test without specific rsc

    Core():width(0), rsc_id(0), start_line(0){
    }    
    void display(){
        //std::cout<<"Assigned Wire Rsc id: "<< rsc_id<<std::endl;
        std::cout<<"Assigned Wires: ";
        std::cout<<"["<<start_line<<","<<start_line+width-1<<"]"<<std::endl;
        
        /* 
        std::cout<<"Preced Test: "<<std::endl;
        for(int i=0; i<trash_tam.size(); i++)
            trash_tam[i].display();
        //for(int i=0; i<bist.size(); i++)
        //    trash_bist[i].display();
        */
    }    
};    
class str_vec_t_and_length{
	public:
	    str_vec_t name;
	    int** length;
};
class System{
public:    
    int test_time;
    int PMAX;
    int TAM_WIDTH;
    int num_core;
    int num_wire_rsc;
    
    //precedence list
    std::vector<str_vec_t_and_length> preced;
    //bist resources
    str_vec_t rsc;
    //cores 
    std::vector<Core> cores;
    //parallel groups: cores that can do external tests in parallel
    std::vector<std::vector <int> > pg; 
    //table
    std::vector< std::vector<test_vec_t> > table;//dimension: num_core * num_rsc
    //table for tests with precedence
    std::vector< std::vector<test_vec_t> > trash_table;//dimension: num_core * (num_core + num_bist_rsc + 1) 
    //final table
    std::vector< std::vector<test_vec_t> > final_table;
    
    
    /* member functions*/
    System():test_time(0)
    {
    }
    int parser(char** argv);
    bool is_preced(std::string s);
    void wire();
    void table_init();
    void trash_table_init();
    void final_table_init();
    void preced_ini();
    void parallel_sched();
    void schedule();
    void trash_schedule();
    int output(char** argv);
    int check_info_error(char** argv,int type, std::string bist_name);

    // debugging funtions
    void pg_display(){

        for(int i=0; i<pg.size(); i++){
            std::cout<<"PG "<<i+1<<std::endl;
            for(int j=0; j<pg[i].size(); j++){
                // core id
                std::cout<<pg[i][j]+1<<" ";
                // core load
                std::cout<<cores[pg[i][j]].load<<std::endl;
            }    
            std::cout<<std::endl;
        }    
    }    
    void display(){

        std::cout<<"System"<<std::endl;
        for(int i=0; i<rsc.size(); i++){
            std::cout<<"BIST rsc "<<std::endl;
            std::cout<<rsc[i]<<std::endl;
        }    
        for(int i=0; i<cores.size(); i++){
            std::cout<<"Core "<<i+1<<std::endl;
            cores[i].display();
        }
        /* table
        for(int i=0; i<cores.size(); i++){
            std::cout<<"Core "<<i+1<<std::endl;
            cores[i].display();
            for(int j=0;j<table[0].size();j++){
                if(table[i][j].size()!=0){
                	for(int k=0;k<table[i][j].size();k++){
                		std::cout << table[i][j][k].name;
                		for(int l=0;l<table[i][j][k].time_begin.size();l++){
                			std::cout<<" ("<<table[i][j][k].time_begin[l]<<","<<table[i][j][k].time_end[l]<<")";
                		}
                		std::cout<<std::endl;
                	}
                }
            }
        }
        */
        /*for(int i=0; i<preced.size(); i++){
            std::cout<<"Precedence "<<i+1<<std::endl; 
            for(int j=0; j<preced[i].name.size(); j++)
                std::cout<< preced[i].name[j] <<" ";
            std::cout<<std::endl;
        }*/
    }    
    void table_display();
    void trash_table_display();
        
};    
class nlist{
    public:
    	test_vec_t* list_pointer;
    	int core_num;
    	int rsc_num;
};
class ntask{
	public:
		Test* task_pointer; 
		int core_num;
		int rsc_num;
};
class check{
    public:
    	bool decided;
    	bool done;
};
#endif
