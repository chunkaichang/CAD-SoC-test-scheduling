/*
 * Change log:
 * precedence modification
 */
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include "system.h"
#include "parser.h"
//void parse_comment(std::ifstream inf){}

bool System::is_preced(std::string s){
    
    for(int i=0; i<preced.size(); i++)
        for(int j=0; j<preced[i].name.size(); j++)
            if(s == preced[i].name[j])
                return true;
    
    return false;
}    

int System::parser(char** argv){
    
    std::ifstream inf(argv[1]);
    if(!inf){
        std::cerr <<"Failed to open input file"<<std::endl;
        return 1;
    }  
    // Parsing system
    std::string s;
    inf >> s;// System
    inf >> s;// begin
    inf >> s;
    while(s != "end"){
        if(s == "TAM_width"){
            inf >> TAM_WIDTH;
            inf >> s;
        }
        else if(s == "Power"){
            inf >> PMAX;
            inf >> s;
        }
        else if(s == "Precedence"){
            str_vec_t_and_length p_list;    
            inf >> s;
            p_list.name.push_back(s);
            inf >> s;
            while(s == ">"){
                inf >> s;
                p_list.name.push_back(s);
                inf >> s;
            }
            // reverse the vector to make it a stack
            std::reverse(p_list.name.begin(), p_list.name.end());
            preced.push_back(p_list);
        }
        else if(s == "Resource"){
            inf >> s;
            rsc.push_back(s);
            inf >> s;
            while(s != "end"){
                rsc.push_back(s);
                inf >> s;
            }
        }    
    }
    
    // Parsing Cores
    int length_sum;
    bool bist_check;
    while(inf>>s){
        Core c;
        length_sum = 0;
        //push empty test list to bist
        for(int i=0; i<rsc.size();i++){
            test_vec_t tv;
            c.bist.push_back(tv); 
            c.trash_bist.push_back(tv); 
        }    
        inf >> c.name; // core_xx
        inf >> s; // begin
        inf >> s; // TAM_width
        inf >> c.width;
        inf >> s;
        int p;
        while(1){
            if(s == "External"){
                Test t;
                inf >> t.name;
                inf >> s;//length
                inf >> t.length;
                length_sum += t.length;
                inf >> s;//power
                inf >> t.power;
                inf >> s;
                if(s == "partition"){
                    inf >> p;
                    t.partition = p-1;
                    inf >> s;
                }    
                if(is_preced(t.name)){//put test in trash table
                    c.trash_tam.push_back(t);
                    //std::cout<<"Find preced: "<< t.name<<std::endl;
                }    
                else
                    c.tam.push_back(t);
            }
            else if(s == "BIST"){
            	bist_check = false;
                Test t;
                inf >> t.name;
                inf >> s;//length
                inf >> t.length;
                inf >> s;//power
                inf >> t.power;
                inf >> s;
                if(s == "resource"){
                    inf >> t.rsc;
                    inf >> s;
                    if(s == "partition"){
                        inf >> p;
                        t.partition = p-1;
                    }    
                    for(int i=0; i<rsc.size(); i++){
                        if(t.rsc == rsc[i]){
                        	bist_check = true;
                            if(is_preced(t.name))
                                c.trash_bist[i].push_back(t);
                            else
                                c.bist[i].push_back(t);
                        }    
                    }   
					if(!bist_check) check_info_error(argv,2,t.rsc);
                }
                else if(s == "partition"){
                    inf >> p;
                    t.partition = p-1;
                    inf >> s;
                    if(s == "resource"){
                        inf >> t.rsc;
                        inf >> s;
                        for(int i=0; i<rsc.size(); i++){
                            if(t.rsc == rsc[i]){
                            	bist_check = true;
                                if(is_preced(t.name))
                                    c.trash_bist[i].push_back(t); 
                                else
                                    c.bist[i].push_back(t);
                            }    
                        }
						if(!bist_check) check_info_error(argv,2,t.rsc);    
                    }
                    else{
                        if(is_preced(t.name))
                            c.trash_free_bist.push_back(t); 
                        else    
                            c.free_bist.push_back(t);
                    }    
                }
                else{
                    if(is_preced(t.name))
                        c.trash_free_bist.push_back(t); 
                    else    
                        c.free_bist.push_back(t);
                }    
            }
            else if(s == "end")
                break;
        }
        c.load = length_sum;
        cores.push_back(c); 
    } 
    
    inf.close();
    return 0;
}    

int System::output(char** argv){
	
	std::ofstream outf(argv[2]);
	
	outf<<"Schedule"<<std::endl;
	outf<<"begin"<<std::endl<<std::endl;
	outf<<"\tTest_time "<<test_time<<std::endl<<std::endl;
	
	for(int i=0; i<cores.size(); i++){
		if(cores[i].width!=0){
        	outf<<"\tTAM_assignment "<<cores[i].name;
        	outf<<" ["<<cores[i].start_line+cores[i].width-1<<":"<<cores[i].start_line<<"]"<<std::endl;
        }
    } 
    outf<<std::endl;
    
    for(int i=0; i<final_table.size(); i++){
    	for(int j=final_table[0].size()-1; j>-1; j--){
    	    if(final_table[i][j].size()!=0){
    	        for(int k=0;k<final_table[i][j].size();k++){
    	        	if(j > 0){
    	        	    outf<<"\tBIST "<<cores[i].name<<" "<<final_table[i][j][k].name;
    	        	    for(int l=0;l<final_table[i][j][k].time_begin.size();l++){
                			outf<<" ("<<final_table[i][j][k].time_begin[l]<<", "<<final_table[i][j][k].time_end[l]<<")";
                		}
                		outf<<std::endl;
    	        	}
    	            else{
    	            	outf<<"\tExternal "<<cores[i].name<<" "<<final_table[i][j][k].name;
    	        	    for(int l=0;l<final_table[i][j][k].time_begin.size();l++){
                			outf<<" ("<<final_table[i][j][k].time_begin[l]<<", "<<final_table[i][j][k].time_end[l]<<")";
                		}
                		outf<<std::endl;
    	            }
    	        }
    	    }	
    	}
    	outf<<std::endl;
    }
    
    outf<<"end";
    
	outf.close();
	return 0;
}
