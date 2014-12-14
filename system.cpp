/*
 * Change log:
 * new class "check","nlist","ntask"
 * new function "schedule()","min_power()","preced_ini()"
 * new struct "Compare_wire","Compare_length"
 * modify system.display()
 * 2014/06/12
 * modify void wire();
 * 2014/06/14
 * new function "parallel_sched()"
 */

#include "system.h"
#include "subsetsum.h"
#include <algorithm>
#include <limits>
#include <fstream>

struct Compare_wire{
        bool operator()(Core_wire const &a, Core_wire const &b){
            return (a.width > b.width);
        }
}cmp_wire;      
struct Compare_load{
        bool operator()(Core_wire const &a, Core_wire const &b){
            return (a.load > b.load);
        }
}cmp_load;      
struct Compare_power{
        bool operator()(Test const &a, Test const &b){
            return (a.power > b.power);
        }
}cmp_power;  
struct Compare_length{
    	bool operator()(ntask const &a, ntask const &b){
        	return (a.task_pointer->length < b.task_pointer->length);
        }
}cmp_length;    
void System::wire(){
    
    int remain = TAM_WIDTH;
    std::vector<Core_wire> demand;
    for(int i=0; i<cores.size(); i++){
        Core_wire cw;
        cw.core = i; 
        cw.width = cores[i].width;
        cw.load = cores[i].load;
        demand.push_back(cw);
    }
    /* Parallel part */
    // reduce the wire assignment problem to the subset sum problem
    //std::sort(demand.begin(),demand.end(), cmp_wire);
    //sort cores to the descending order of load to balance the test length
    std::sort(demand.begin(),demand.end(), cmp_load);

    std::vector<Core_wire> tmp_vec;
    std::vector<int> result;
    while(1){
    //for(int k=0; k<10; k++){
        //put cores with nonzero width together
        for(int i=0; i<demand.size(); i++){
            if(demand[i].width > 0 ){
                tmp_vec.push_back(demand[i]);
                //std::cout<<"core "<<demand[i].core + 1;
            }    
        }    
        subset_sum(TAM_WIDTH, tmp_vec.size(), tmp_vec, result);
        if(result.size()==0) break;
        pg.push_back(result);

        // modify the width of selected cores to 0 in demand
        for(int i=0; i<result.size(); i++){
            for(int j=0; j<demand.size(); j++){
                if(result[i] == demand[j].core){
                    //std::cout<<demand[j].core+1<<" "<<demand[j].width<<std::endl;
                    demand[j].width = 0;
                    break;
                }
            }
        }    
        //std::cout<<std::endl;
        tmp_vec.clear();
        result.clear();
    }
    /* Assign wires to cores*/
    int start = 0; int max_rsc = 0;
    for(int i=0; i<pg.size(); i++){
        start = 0;
        for(int j=0; j<pg[i].size(); j++){
            cores[pg[i][j]].start_line = start;
            start += cores[pg[i][j]].width;
            cores[pg[i][j]].rsc_id = j;
        }        
        if(pg[i].size() > max_rsc)
            max_rsc = pg[i].size();
    }
    num_wire_rsc = max_rsc;
    
    /* Sequential part
    remain = TAM_WIDTH;
    int start = 0;
    int rsc_id = 0;
    std::vector<Core_wire> wire_rsc;
    //std::vector<int> rsc_host_id;
    // resource arranging
    while(remain > 0){
        //sort according to wire in descending order
        std::sort(demand.begin(),demand.end(), cmp_wire);
        for(int i=0; i<demand.size(); i++){
            if(demand[i].width != 0 && demand[i].width <= remain){
                //assign this core to some wire rsc
                int c = demand[i].core;
                int w = demand[i].width;
                int l = cores[c].load;
                cores[c].rsc_id = rsc_id++;
                cores[c].start_line = start;
                start += w;
                remain -= w;
                demand[i].width = 0;
                //rsc_host_id.push_back(c);
                Core_wire r;
                r.core = c;
                r.width = w;
                r.load = l;
                wire_rsc.push_back(r);
            }    
        }
    }
    num_wire_rsc = rsc_id;
    // sort rsc to ascending order of width
    std::sort(wire_rsc.begin(), wire_rsc.end(), cmp_wire);
    std::reverse(wire_rsc.begin(), wire_rsc.end());
    
    int c, w, w_bound;
    int min_id;
    int load_min = std::numeric_limits<int>::max();//set len_min to a huge number

    // arranging other cores
    for(int i=0; i<demand.size(); i++){
        c = demand[i].core;
        w = demand[i].width;
        if(w != 0){//exclude cores already been assigned
            int host;        
            // first round: find resource with enough wires
            std::vector<Core_wire> round_1;
            std::vector<int> round_1_idx;
            for(int j=0; j<wire_rsc.size(); j++){
                host = wire_rsc[j].core;
                w_bound = wire_rsc[j].width;
                
                if(w <= w_bound){
                    round_1.push_back(wire_rsc[j]);
                    round_1_idx.push_back(j);
                }    
                
            }
            int host_load; int rsc_idx;
            // Second round: find resource with less loading
            for(int j=0; j<round_1.size(); j++){
                host = round_1[j].core;
                host_load = round_1[j].load;
                
                if(host_load < load_min){
                    min_id = host;
                    load_min = host_load;
                    rsc_idx = round_1_idx[j];
                }   
            }    
            
            cores[c].rsc_id = cores[min_id].rsc_id;
            cores[c].start_line = cores[min_id].start_line;
            demand[i].width = 0;
            wire_rsc[rsc_idx].load += cores[c].load;
        }    
    }
    */
}

void System::parallel_sched(){
    int row; 
    int col;
    for(int i=0; i<pg.size(); i++){
        //fill external tests for this parallel group
        for(int j=0; j<pg[i].size(); j++){
            row = pg[i][j];
            col = cores[row].rsc_id;
            //std::cout<<"r "<<row<<" c "<<col<<std::endl;
            if(!cores[row].tam.empty()){
                table[row][col] = cores[row].tam;
            }    
        }
        schedule();
        //std::cout<<"PG "<<i<<"Finish!"<<std::endl;
        //remove external tests for this parallel group
        for(int j=0; j<pg[i].size(); j++){
            row = pg[i][j];
            col = cores[row].rsc_id;
            table[row][col].clear();
                
        }     
    }    
    
    // Fill the trash_table ////////////////////////////////////////////////////////
	for(int i=0;i<cores.size();i++){
		for(int j=num_wire_rsc;j<table[0].size();j++){
			if(table[i][j].size()!=0){
				for(int k=0;k<table[i][j].size();k++){
				    if(table[i][j][k].length!=0)  trash_table[i][j-num_wire_rsc+cores.size()].push_back(table[i][j][k]);
				}
			}
		}
	}
}    
void System::table_init(){
    
    std::vector<test_vec_t> core_list; 
    for(int i=0; i<cores.size(); i++){
        // fill wire rsc
        for(int j=0; j<num_wire_rsc; j++){
            //if(cores[i].rsc_id == j)
            //    core_list.push_back(cores[i].tam);
            //else{
            //push an empty list
                test_vec_t tv;
                core_list.push_back(tv);
            //}    
        }    
        // fill bist rsc
        for(int j=0; j<rsc.size(); j++){
            if(!cores[i].bist[j].empty())
                core_list.push_back(cores[i].bist[j]);
            else{
                test_vec_t tv;
                core_list.push_back(tv);
            }    
        }    
        // fill free_bis rsc
        if(!cores[i].free_bist.empty())
            core_list.push_back(cores[i].free_bist);
        else{
            test_vec_t tv;
            core_list.push_back(tv);
        }    
        table.push_back(core_list);
        core_list.clear();
    }
}
void System::trash_table_init(){
    
    std::vector<test_vec_t> core_list; 
    for(int i=0; i<cores.size(); i++){
        // fill external tests
        for(int j=0; j<cores.size(); j++){
            if(i == j)
                core_list.push_back(cores[i].trash_tam);
            else{
            //push an empty list
                test_vec_t tv;
                core_list.push_back(tv);
            }    
        }    
        // fill bist rsc
        for(int j=0; j<rsc.size(); j++){
            if(!cores[i].trash_bist[j].empty())
                core_list.push_back(cores[i].trash_bist[j]);
            else{
                test_vec_t tv;
                core_list.push_back(tv);
            }    
        }    
        // fill free_bis rsc
        if(!cores[i].trash_free_bist.empty())
            core_list.push_back(cores[i].trash_free_bist);
        else{
            test_vec_t tv;
            core_list.push_back(tv);
        }    
        trash_table.push_back(core_list);
        core_list.clear();
    }
    
    for(int i=0; i<cores.size(); i++){
    	for(int j=num_wire_rsc; j<table[0].size(); j++){
    	    if(table[i][j].size()!=0){
    	    	for(int k=0;k<table[i][j].size(); k++){
    	    	    trash_table[i][j-num_wire_rsc+cores.size()].push_back(table[i][j][k]);
    	    	}
    	    }
    	}
    }
}	

void System::final_table_init(){
	std::vector<test_vec_t> core_list; 
	test_vec_t tv;

	for (int i=0;i<rsc.size()+2;i++) core_list.push_back(tv);
	for (int i=0;i<cores.size();i++) final_table.push_back(core_list);
	
	core_list.clear();
}

void System::table_display(){
    
    for(int i=0; i<table.size(); i++){
        std::cout<<"Core "<<i+1<<std::endl;
        for(int j=0; j<table[i].size(); j++){
            
            if(j < num_wire_rsc)//wire
                std::cout<<"External"<<std::endl;
            else if(j < num_wire_rsc+rsc.size())//bist
                std::cout<<"BIST"<<std::endl;
            else//free bist
                std::cout<<"Free BIST"<<std::endl;
            
            if(!table[i][j].empty()){    
                for(int k=0; k<table[i][j].size(); k++){
                    std::cout<<"\t"<<std::endl;
                    table[i][j][k].display();

                }    
            }
            else
                std::cout<<"\tEmpty"<<std::endl;
        }    
        std::cout<<std::endl;
    }
}    
void System::trash_table_display(){
    
    for(int i=0; i<trash_table.size(); i++){
        std::cout<<"Core "<<i+1<<std::endl;
        for(int j=0; j<trash_table[i].size(); j++){
            
            if(j < cores.size())//wire
                std::cout<<"External"<<std::endl;
            else if(j < cores.size()+rsc.size())//bist
                std::cout<<"BIST"<<std::endl;
            else//free bist
                std::cout<<"Free BIST"<<std::endl;
            
            if(!trash_table[i][j].empty()){    
                for(int k=0; k<trash_table[i][j].size(); k++){
                    std::cout<<"\t"<<std::endl;
                    trash_table[i][j][k].display();
                }    
            }
            else
                std::cout<<"\tEmpty"<<std::endl;
        }    
        std::cout<<std::endl;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Precedence initial
void System::preced_ini(){
	for(int i=0;i<preced.size();i++){
		preced[i].length = new int* [preced[i].name.size()];
	    for(int j=0;j<preced[i].name.size();j++){
        	for (int k=0;k<trash_table.size();k++){
		    	for (int l=0;l<trash_table[0].size();l++){
		        	if(trash_table[k][l].size()!=0){
		        		for(int m=0;m<trash_table[k][l].size();m++){
		        			if(preced[i].name[j]==trash_table[k][l][m].name) preced[i].length[j] = &trash_table[k][l][m].length;
		        		}
		        	}
		        }
		    }
		}
	}
}

//Find min power
int min_power(test_vec_t a){
    int min;
    for(int i=a.size()-1;i>-1;i--){
    	if(a[i].length!=0) {
    	    min = a[i].power;
    	    break;
    	}
    } 
    return min;
}
void System::schedule(){
	int pl = PMAX;
	int tpw = 0;
	int timeline_before = test_time;
	int timeline_after = test_time;
	bool* task_remove;
	bool* list_remove;
	
	// Check row & col
	bool* row_check;
	bool* col_check;

	row_check = new bool[table.size()];
	col_check = new bool[table[0].size()-1];
    for(int i=0;i<table.size();i++) row_check[i]=false;
    for(int i=0;i<table[0].size();i++) col_check[i]=false;
    
	// Table_check /////////////////////////////////////////////////////////////////
	check** table_check;
	table_check = new check* [table.size()];
	for (int i=0;i<table.size();i++){
		table_check[i] = new check[table[0].size()];
	}

	for (int i=0;i<table.size();i++){
		for (int j=0;j<table[0].size();j++){
			table_check[i][j].decided = false;
			table_check[i][j].done = false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////

	// Sort function ///////////////////////////////////////////////////////////////

	//Sort the tasks m_ijk according to its power
	for (int i=0; i<table.size(); i++){
    	for (int j=0; j<table[0].size(); j++){
    		std::sort(table[i][j].begin(),table[i][j].end(), cmp_power);
    	}
	}
	////////////////////////////////////////////////////////////////////////////////

	// new class, ntask and nlist //////////////////////////////////////////////////
	std::vector<nlist> list_list;
	std::vector<ntask> task_list;
	nlist temp_list;
	ntask temp_task;
	////////////////////////////////////////////////////////////////////////////////

	int m_min;
	int p_min;
	int begin;
	bool list_empty;
	bool stop_check;
	int ext_task;

    ext_task =0;	
	for(int i=0;i<cores.size();i++){
    	for(int j=0;j<num_wire_rsc;j++){
    		if(table[i][j].size()!=0){
    		    ext_task += table[i][j].size();
    		}
    	}
    }
    
	// Scheduling //////////////////////////////////////////////////////////////////
	while (1){	
	    if(ext_task){
	    	for (int i=0; i<table.size(); i++){
	        	for (int j=0; j<table[0].size(); j++){
	        		//Violation check
	        		if (table[i][j].size()==0) continue;
	        		if (row_check[i]) break;
	        		if (j==table[0].size()-1) col_check[j] = false; // free bist compensation
	        		if (col_check[j]) continue;
					if (table_check[i][j].done) continue;
					if (table_check[i][j].decided) continue;
				    
					//Find min_power
					p_min = min_power(table[i][j]);
	
					//Select tasks from table
			 	    if (tpw + p_min <= pl){     
					    for (int k=0; k<table[i][j].size();k++){
				            if(table[i][j][k].length!=0){
				            	
				            	//Power limitation
				            	if(tpw + table[i][j][k].power > pl) continue;
	                	    	
	                	    	// Pick list
	                	    	temp_list.list_pointer = &table[i][j];
	                	    	temp_list.core_num = i;
	                	    	temp_list.rsc_num = j;
	                    		list_list.push_back(temp_list);
	                    		
	                    		// Pick task
	                    		temp_task.task_pointer = &table[i][j][k];
	                    		temp_task.core_num = i;
	                	    	temp_task.rsc_num = j;
                     			task_list.push_back(temp_task);
                     	    
                        		row_check[i]=true;
                        		col_check[j]=true;
                        		tpw += table[i][j][k].power;
                        		table_check[i][j].decided = true;
                        		break;
                   			}
                   			else continue;
           	        	}
					}
				}
   			}
   	    }
	
    	//Sort task_list according to its remaining length
    	std::sort(task_list.begin(),task_list.end(), cmp_length);
    	m_min = 0;
		if(task_list.size()!=0)m_min = task_list[0].task_pointer->length;

		//Record time
		timeline_after = timeline_before + m_min;
		
		//Remove_list or task initial
		list_remove = new bool [list_list.size()];
		task_remove = new bool [task_list.size()];
		for(int i=0;i<list_list.size();i++) list_remove[i] = false;
		for(int i=0;i<task_list.size();i++) task_remove[i] = false;

		//Schedule tasks represented by elements in task_list for time m_min;
		for (int i=task_list.size()-1;i>-1;i--){
	    	task_list[i].task_pointer->length -= m_min;
        	if(task_list[i].task_pointer->time_begin.size()==task_list[i].task_pointer->time_end.size()){
        	    task_list[i].task_pointer->time_begin.push_back(timeline_before);
        	}
        	

            //Mark list where the task belongs to
		    for(int j=0;j<list_list.size();j++){
	        	if((list_list[j].core_num)==(task_list[i].core_num)&&(list_list[j].rsc_num)==(task_list[i].rsc_num)){
	            	begin = j;
	            	break;
	        	}
	    	}

	    	//Task done
    		if (task_list[i].task_pointer->length==0) {
    			task_list[i].task_pointer->time_end.push_back(timeline_after-1);
	    		if(task_list[i].task_pointer->partition >0) task_list[i].task_pointer->partition = 0;
	    		tpw = tpw - task_list[i].task_pointer->power;
	    		list_empty = true;
	        
	            //Check external task size
	            if(task_list[i].rsc_num < num_wire_rsc) ext_task--;
	            
	    		//Mark list where mijk belongs to as undecided
	    		table_check[list_list[begin].core_num][list_list[begin].rsc_num].decided = false;
	    	
	    		//Record the list to remove
	    		list_remove[begin]=true;
	        
	        	//Remove the task from the list
	        	task_remove[i] = true;
	        
	        	//Mark empty list as done
	        	for(int j=0;j<table[list_list[begin].core_num][list_list[begin].rsc_num].size();j++){
	            	if(table[list_list[begin].core_num][list_list[begin].rsc_num][j].length!=0) list_empty = false;
	        	}
	        
	        	if(list_empty){
	            	table_check[list_list[begin].core_num][list_list[begin].rsc_num].done = true;
	        	}
	        
	        	//free the resource
	        	row_check[list_list[begin].core_num]=false;
	        	col_check[list_list[begin].rsc_num]=false;
	    	}
	    	if (task_list[i].task_pointer->partition >0) {
	    		task_list[i].task_pointer->time_end.push_back(timeline_after-1);
	    		task_list[i].task_pointer->partition -= 1;
	    		tpw = tpw - task_list[i].task_pointer->power;
	    	
	    		//Mark list where mijk belongs to as undecided
	    		table_check[list_list[begin].core_num][list_list[begin].rsc_num].decided = false;
	    	
	    		//Record the list to remove
	    		list_remove[begin]=true;
	    
	        	//Remove the task from the list
	    		task_remove[i] = true;
	    	
	    		//free the resource
	        	row_check[list_list[begin].core_num]=false;
	        	col_check[list_list[begin].rsc_num]=false;
	    	}
    	} 
    	//Record time
		timeline_before = timeline_after;
	
	    //Remove task from task_list
	    for(int i=task_list.size()-1;i>-1;i--){
			if(task_remove[i]) task_list.erase(task_list.begin()+i);
		}
	    
		//Remove list from list_list
		for(int i=list_list.size()-1;i>-1;i--){
			if(list_remove[i]) list_list.erase(list_list.begin()+i);
		}
		
		delete [] task_remove;
		delete [] list_remove;
		
		if(ext_task==0 && task_list.size()==0) {
			break;
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// Fill the final table ////////////////////////////////////////////////////////
	for(int i=0; i<cores.size(); i++){
		
		for(int j=0; j<num_wire_rsc; j++){
		    if(table[i][j].size()!=0) {
		    	for(int k=0; k<table[i][j].size();k++){
		    	    if(table[i][j][k].length==0)final_table[i][0].push_back(table[i][j][k]);
		        }
		        break;
		    }
		}
		
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// Fill the final table ////////////////////////////////////////////////////////
	test_time = timeline_after;
	for(int i=0;i<cores.size();i++) delete [] table_check[i];
	delete [] table_check;
	delete [] row_check;
	delete [] col_check;
}
























void System::trash_schedule(){
	int pl = PMAX;
	int tpw = 0;
	int timeline_before = test_time;
	int timeline_after = test_time;
	bool* task_remove;
	bool* list_remove;
	
	// Check row & col
	bool* row_check;
	bool* col_check;

	row_check = new bool[trash_table.size()];
	col_check = new bool[trash_table[0].size()-1];
    for(int i=0;i<trash_table.size();i++) row_check[i]=false;
    for(int i=0;i<trash_table[0].size();i++) col_check[i]=false;
    
	// Total task 
	int total_task=0;

	for (int i=0;i<trash_table.size();i++){
		for (int j=0;j<trash_table[0].size();j++){
		    if(!trash_table[i][j].empty()) {
		        for(int k=0;k<trash_table[i][j].size();k++){
		        	if(trash_table[i][j][k].length!=0) total_task ++;
		        }
		    }	
    	}
	}
	// trash_table_check /////////////////////////////////////////////////////////////////
	check** trash_table_check;
	trash_table_check = new check* [trash_table.size()];
	for (int i=0;i<trash_table.size();i++){
		trash_table_check[i] = new check[trash_table[0].size()];
	}

	for (int i=0;i<trash_table.size();i++){
		for (int j=0;j<trash_table[0].size();j++){
			trash_table_check[i][j].decided = false;
			trash_table_check[i][j].done = false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////

	// Sort function ///////////////////////////////////////////////////////////////

	//Sort the tasks m_ijk according to its power
	for (int i=0; i<trash_table.size(); i++){
    	for (int j=0; j<trash_table[0].size(); j++){
    		std::sort(trash_table[i][j].begin(),trash_table[i][j].end(), cmp_power);
    	}
	}
	////////////////////////////////////////////////////////////////////////////////

	// new class, ntask and nlist //////////////////////////////////////////////////
	std::vector<nlist> list_list;
	std::vector<ntask> task_list;
	nlist temp_list;
	ntask temp_task;
	////////////////////////////////////////////////////////////////////////////////

	bool check_preced;
	bool check_wire;
	int m_min;
	int p_min;
	int begin;
	bool list_empty;

	// Scheduling //////////////////////////////////////////////////////////////////
	while (total_task!=0){		
    	for (int i=0; i<trash_table.size(); i++){
        	for (int j=0; j<trash_table[0].size(); j++){
        		//Violation check
        		if (trash_table[i][j].size()==0) continue;
        		if (row_check[i]) break;
        		if (j==trash_table[0].size()-1) col_check[j] = false; // free bist compensation
        		if (col_check[j]) continue;
				if (trash_table_check[i][j].done) continue;
				if (trash_table_check[i][j].decided) continue;
			    
			    //Check wire
			    check_wire = false;
			    if (j<cores.size()){
			        for (int k=0;k<list_list.size();k++){
			            if (list_list[k].rsc_num<cores.size()){
			                if (cores[list_list[k].core_num].start_line <= cores[i].start_line){
			                    if (cores[list_list[k].core_num].start_line+cores[list_list[k].core_num].width>=cores[i].start_line){
			                		check_wire = true;
			                		break;
			                	}
			                }
			                else {
			                    if (cores[i].start_line+cores[i].width>=cores[list_list[k].core_num].start_line){
			                    	check_wire = true;
			                    	break;
			                    }
			                }
			            }
			        }
			    }
			    if(check_wire) continue;
			    
				//Find min_power
				p_min = min_power(trash_table[i][j]);

				//Select tasks from trash_table
		 	    if (tpw + p_min <= pl){     
				    for (int k=0; k<trash_table[i][j].size();k++){
			            if(trash_table[i][j][k].length!=0){
			            	
			            	//Power limitation
			            	if(tpw + trash_table[i][j][k].power > pl) continue;
			            	
				    		//Check precedence
				    		check_preced=false;
				    		for(int l=0; l<preced.size(); l++){
				    			for(int m=0; m<preced[l].name.size(); m++){
				    			    if(preced[l].name[m] == trash_table[i][j][k].name){
				    		       	    for(int n=preced[l].name.size()-1;n>m;n--){
			  		  		                if(*(preced[l].length[n])!=0){
			  		  		                	check_preced = true;
			  		  		                }
				    		            }
			    		  	        }
			    			    }
			    		    }
			    		    if(check_preced) continue;
                	    	
                	    	// Pick list
                	    	temp_list.list_pointer = &trash_table[i][j];
                	    	temp_list.core_num = i;
                	    	temp_list.rsc_num = j;
                    		list_list.push_back(temp_list);
                    		
                    		// Pick task
                    		temp_task.task_pointer = &trash_table[i][j][k];
                    		temp_task.core_num = i;
                	    	temp_task.rsc_num = j;
                     		task_list.push_back(temp_task);
                     	    
                        	row_check[i]=true;
                        	col_check[j]=true;
                        	tpw += trash_table[i][j][k].power;
                        	trash_table_check[i][j].decided = true;
                        	break;
                   		}
                   		else continue;
           	        }
				}
			}
   		}
	
    	//Sort task_list according to its remaining length
    	std::sort(task_list.begin(),task_list.end(), cmp_length);
		m_min = task_list[0].task_pointer->length;

		//Record time
		timeline_after = timeline_before + m_min;
		
		//Remove_list or task initial
		list_remove = new bool [list_list.size()];
		task_remove = new bool [task_list.size()];
		for(int i=0;i<list_list.size();i++) list_remove[i] = false;
		for(int i=0;i<task_list.size();i++) task_remove[i] = false;

		//Schedule tasks represented by elements in task_list for time m_min;
		for (int i=task_list.size()-1;i>-1;i--){
	    	task_list[i].task_pointer->length -= m_min;
        	if(task_list[i].task_pointer->time_begin.size()==task_list[i].task_pointer->time_end.size()){
        	    task_list[i].task_pointer->time_begin.push_back(timeline_before);
        	}
        	

            //Mark list where the task belongs to
		    for(int j=0;j<list_list.size();j++){
	        	if((list_list[j].core_num)==(task_list[i].core_num)&&(list_list[j].rsc_num)==(task_list[i].rsc_num)){
	            	begin = j;
	            	break;
	        	}
	    	}

	    	//Task done
    		if (task_list[i].task_pointer->length==0) {
    			task_list[i].task_pointer->time_end.push_back(timeline_after-1);
	    		if(task_list[i].task_pointer->partition >0) task_list[i].task_pointer->partition = 0;
	    		tpw = tpw - task_list[i].task_pointer->power;
	        	total_task -= 1;
	    		list_empty = true;
	        
	    		//Mark list where mijk belongs to as undecided
	    		trash_table_check[list_list[begin].core_num][list_list[begin].rsc_num].decided = false;
	    	
	    		//Record the list to remove
	    		list_remove[begin]=true;
	        
	        	//Remove the task from the list
	        	task_remove[i] = true;
	        
	        	//Mark empty list as done
	        	for(int j=0;j<trash_table[list_list[begin].core_num][list_list[begin].rsc_num].size();j++){
	            	if(trash_table[list_list[begin].core_num][list_list[begin].rsc_num][j].length!=0) list_empty = false;
	        	}
	        
	        	if(list_empty){
	            	trash_table_check[list_list[begin].core_num][list_list[begin].rsc_num].done = true;
	        	}
	        
	        	//free the resource
	        	row_check[list_list[begin].core_num]=false;
	        	col_check[list_list[begin].rsc_num]=false;
	    	}
	    	if (task_list[i].task_pointer->partition >0) {
	    		task_list[i].task_pointer->time_end.push_back(timeline_after-1);
	    		task_list[i].task_pointer->partition -= 1;
	    		tpw = tpw - task_list[i].task_pointer->power;
	    	
	    		//Mark list where mijk belongs to as undecided
	    		trash_table_check[list_list[begin].core_num][list_list[begin].rsc_num].decided = false;
	    	
	    		//Record the list to remove
	    		list_remove[begin]=true;
	    
	        	//Remove the task from the list
	    		task_remove[i] = true;
	    	
	    		//free the resource
	        	row_check[list_list[begin].core_num]=false;
	        	col_check[list_list[begin].rsc_num]=false;
	    	}
    	} 
    	//Record time
		timeline_before = timeline_after;
	
	    //Remove task from task_list
	    for(int i=task_list.size()-1;i>-1;i--){
			if(task_remove[i]) task_list.erase(task_list.begin()+i);
		}
	    
		//Remove list from list_list
		for(int i=list_list.size()-1;i>-1;i--){
			if(list_remove[i]) list_list.erase(list_list.begin()+i);
		}
		
		delete [] task_remove;
		delete [] list_remove;
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// Fill the final trash_table ////////////////////////////////////////////////////////
	for(int i=0; i<cores.size(); i++){
		
		for(int j=0; j<cores.size(); j++){
		    if(trash_table[i][j].size()!=0) {
		    	for(int k=0; k<trash_table[i][j].size();k++){
		    	    final_table[i][0].push_back(trash_table[i][j][k]);
		        }
		        break;
		    }
		}
		
		for(int j=cores.size(); j<cores.size()+rsc.size(); j++){
			if(trash_table[i][j].size()!=0) {
				for(int k=0; k<trash_table[i][j].size();k++){
				    final_table[i][j+1-cores.size()].push_back(trash_table[i][j][k]);
			    }
			}
		}
		
		if(trash_table[i][trash_table[0].size()-1].size()!=0) {
		    for(int k=0; k<trash_table[i][trash_table[0].size()-1].size();k++){
		        final_table[i][final_table[0].size()-1].push_back(trash_table[i][trash_table[0].size()-1][k]);
		    }
		}
		
	}
	////////////////////////////////////////////////////////////////////////////////
	test_time = timeline_after;
	for(int i=0;i<cores.size();i++) delete [] trash_table_check[i];
	delete [] trash_table_check;
	delete [] row_check;
	delete [] col_check;
}

int System::check_info_error(char** argv,int type,std::string bist_name){
	
	bool error = false; 
	std::ofstream outf(argv[2]);
	
	// error_2 Bist resoure does not exist
	if(type==2) {
		outf << "Error_2: "<<bist_name<<" does not exist in the system"<<std::endl;
		outf.close();
		exit (EXIT_FAILURE);
	}
	
	// error_0 TAM width violation
	for (int i=0;i<cores.size();i++){
	    if(cores[i].width > TAM_WIDTH) {
	        outf << "Error_0: "<<cores[i].name<<" TAM_width too large"<<std::endl;
	        error = true;
	    }
	}
	
	// error_1 Power violation
	for (int i=0;i<cores.size();i++){
		for(int j=0;j<cores[i].tam.size();j++){
			if(cores[i].tam[j].power > PMAX){
				outf << "Error_1: "<<cores[i].name<<" "<<cores[i].tam[j].name<<" power exceeds system limitation"<<std::endl;
				error = true;
			}
		}
		for(int j=0;j<cores[i].bist.size();j++){
			for(int k=0;k<cores[i].bist[j].size();k++){
				if(cores[i].bist[j][k].power > PMAX){
					outf << "Error_1: "<<cores[i].name<<" "<<cores[i].bist[j][k].name<<" power exceeds system limitation"<<std::endl;
					error = true;
				}
			}
		}
		for(int j=0;j<cores[i].free_bist.size();j++){
			if(cores[i].free_bist[j].power > PMAX){
				outf << "Error_1: "<<cores[i].name<<" "<<cores[i].free_bist[j].name<<" power exceeds system limitation"<<std::endl;
				error = true;
			}
		}
	}

	
	outf.close();
	
	if(error) return 1;
	
	return 0;
}
	
