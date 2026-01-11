#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <iomanip>
#include "sim_proc.h"
using namespace std;

processor::processor(const proc_params& params) 
        : parameters(params), end_of_file(false), instr_count(0), cycle_count(0),
         rob_head(0), rob_tail(0), free_rob_entries(parameters.rob_size) {
    ROB.resize(parameters.rob_size);
    RMT.resize(67);
    for (int i=0; i<67; ++i)
        RMT[i].valid = false;
    for (int i=0; i<parameters.rob_size; ++i) {
        ROB[i].ready = false;
        ROB[i].occupied = false;
    }
}

void processor::load_file(const string& trace_file) {
    uint32_t pc;
    int op_type, dest, src1, src2;
    file.open(trace_file);
    if (!file) {
        cout << "Error: Unable to open file " << trace_file << "\n";
        exit(EXIT_FAILURE);
    }
}

void processor::run() {
    do {
        retire();
        writeback();
        execute();
        issue();
        dispatch();
        reg_read();
        rename();
        decode();
        fetch();
        ++cycle_count;
    } while (advance_cycle());
}
void processor::fetch() {
    if (!end_of_file && DE.empty()) {
        vector<instruction> input_reg;
        while (input_reg.size() < parameters.width) {
            uint32_t pc;
            int op_type, dest, src1, src2;
            if (file >> hex >> pc >> dec >> op_type >> dest >> src1 >> src2) {
                instruction instr;
                instr.pc = pc;
                instr.op_type = op_type;
                instr.dest = dest;
                instr.src1 = src1;
                instr.src2 = src2;
                instr.sq_no = instr_count;
                instr.fe_count.start = cycle_count;
                instr.fe_count.total = -1;
                instr.de_count.start = -1;
                instr.de_count.total = -1;
                instr.rn_count.start = -1;
                instr.rn_count.total = -1;
                instr.rr_count.start = -1;
                instr.rr_count.total = -1;
                instr.di_count.start = -1;
                instr.di_count.total = -1;
                instr.is_count.start = -1;
                instr.is_count.total = -1;
                instr.ex_count.start = -1;
                instr.ex_count.total = -1;
                instr.wb_count.start = -1;
                instr.wb_count.total = -1;
                instr.rt_count.start = -1;
                instr.rt_count.total = -1;

                input_reg.push_back(instr);
                ++instr_count;
            }
            else {
                end_of_file = true;
                file.close();
                break;
            }
        }
        if (!input_reg.empty()) {
            DE = move(input_reg);
        }
    }
}

void processor::decode() {
    for (int i=0; i<DE.size(); ++i) {
        if (DE[i].de_count.start == -1)
        DE[i].de_count.start = cycle_count;
        if (DE[i].fe_count.total == -1 && DE[i].de_count.start != -1 && DE[i].fe_count.start != -1)
        DE[i].fe_count.total = DE[i].de_count.start - DE[i].fe_count.start;
    }
    if (!DE.empty() && RN.empty()) {
        RN = move(DE);
        DE.clear();
    }
}

void processor::rename() {
    for (int i=0; i<RN.size(); ++i) {
        if (RN[i].rn_count.start == -1)
        RN[i].rn_count.start = cycle_count;
        if (RN[i].de_count.total == -1 && RN[i].rn_count.start != -1 && RN[i].de_count.start != -1) 
        RN[i].de_count.total = RN[i].rn_count.start - RN[i].de_count.start;
    }
    if (!RN.empty() && RR.empty() && free_rob_entries>=RN.size()) {
        for (int i=0; i<RN.size(); ++i) {
            renamed_instruction renamed_instr;
            renamed_instr.instr = RN[i];
            renamed_instr.dest_renamed = rob_tail;

            if (RMT[RN[i].src1].valid && RN[i].src1!=-1) {
                renamed_instr.src1_ready = false;
                renamed_instr.src1_renamed = RMT[RN[i].src1].rob_tag;
            }
            else {
                renamed_instr.src1_ready = true;
                renamed_instr.src1_renamed = -1;
            }

            if (RMT[RN[i].src2].valid && RN[i].src2!=-1) {
                renamed_instr.src2_ready = false;
                renamed_instr.src2_renamed = RMT[RN[i].src2].rob_tag;
            }
            else {
                renamed_instr.src2_ready = true;
                renamed_instr.src2_renamed = -1;
            }

            if (RN[i].dest != -1) {
                RMT[RN[i].dest].instr = RN[i];
                RMT[RN[i].dest].valid = true;
                RMT[RN[i].dest].rob_tag = rob_tail;
            }

            RR.push_back(renamed_instr);
            ROB[rob_tail].instr = RN[i];
            ROB[rob_tail].ready = false;
            ROB[rob_tail].occupied = true;
            if (rob_tail == parameters.rob_size-1)
            rob_tail = 0;
            else
            ++rob_tail;
            --free_rob_entries;
        }
        RN.clear();
    }
}

void processor::reg_read() {
    for (int i=0; i<RR.size(); ++i) {
        if (RR[i].instr.rr_count.start == -1)
        RR[i].instr.rr_count.start = cycle_count;
        if (RR[i].instr.rn_count.total == -1 && RR[i].instr.rr_count.start != -1 && RR[i].instr.rn_count.start != -1)
        RR[i].instr.rn_count.total = RR[i].instr.rr_count.start - RR[i].instr.rn_count.start;
    }
    if (!RR.empty() && DI.empty()) {
        DI = move(RR);
        RR.clear();
    }
}

void processor::dispatch() {
    for (int i=0; i<DI.size(); ++i) {
        if (DI[i].instr.di_count.start == -1)
        DI[i].instr.di_count.start = cycle_count;
        if (DI[i].instr.rr_count.total == -1 && DI[i].instr.rr_count.start != -1 && DI[i].instr.rr_count.start != -1)
        DI[i].instr.rr_count.total = DI[i].instr.di_count.start - DI[i].instr.rr_count.start;
    }
    if (!DI.empty() && ((parameters.iq_size - IQ.size()) >= DI.size())) {
        for (int i=0; i<DI.size(); ++i) {
            iq_entry iq_row;
            iq_row.valid = true;
            iq_row.renamed_instr = DI[i];
            IQ.push_back(iq_row);
        }
        DI.clear();
    }
}

void processor::issue() {
    int width_temp = 0;
    for (int i=0; i<IQ.size(); ++i) {
        if (IQ[i].renamed_instr.instr.is_count.start == -1)
        IQ[i].renamed_instr.instr.is_count.start = cycle_count;
        if (IQ[i].renamed_instr.instr.di_count.total == -1 && IQ[i].renamed_instr.instr.is_count.start != -1 && IQ[i].renamed_instr.instr.di_count.start != -1)
        IQ[i].renamed_instr.instr.di_count.total = IQ[i].renamed_instr.instr.is_count.start - IQ[i].renamed_instr.instr.di_count.start;

        if (width_temp < parameters.width && IQ[i].valid && IQ[i].renamed_instr.src1_ready && IQ[i].renamed_instr.src2_ready && execute_list.size()<parameters.width*5) {
            renamed_instruction_execute renamed_instr_exe;
            renamed_instr_exe.renamed_instr = IQ[i].renamed_instr;
            switch (renamed_instr_exe.renamed_instr.instr.op_type) {
                case 0: 
                    renamed_instr_exe.timer = 1;
                    break;
                case 1: 
                    renamed_instr_exe.timer = 2;
                    break;
                case 2: 
                    renamed_instr_exe.timer = 5;
                    break;
                default: 
                    renamed_instr_exe.timer = 1;
                    break;
            }
            execute_list.push_back(renamed_instr_exe);
            IQ[i].valid = false;
            ++width_temp;         
        }
    }
    for (int i=IQ.size()-1; i>=0; --i) {
        if (!IQ[i].valid)
        IQ.erase(IQ.begin() + i);
    }
}

void processor::execute() {
    vector<renamed_instruction> executed_instructions;
    int exe_temp_count = WB.size();
    for (int i = execute_list.size() - 1; i >= 0; --i) {

        if (execute_list[i].renamed_instr.instr.ex_count.start == -1)
        execute_list[i].renamed_instr.instr.ex_count.start = cycle_count;
        if (execute_list[i].renamed_instr.instr.is_count.total == -1 && execute_list[i].renamed_instr.instr.ex_count.start != -1 && execute_list[i].renamed_instr.instr.is_count.start != -1)
        execute_list[i].renamed_instr.instr.is_count.total = execute_list[i].renamed_instr.instr.ex_count.start - execute_list[i].renamed_instr.instr.is_count.start;

        --execute_list[i].timer;
        if (execute_list[i].timer == 0 && exe_temp_count < parameters.width * 5) {
            executed_instructions.push_back(execute_list[i].renamed_instr);
            execute_list.erase(execute_list.begin() + i);
            ++exe_temp_count;
        }
    }

    for (const auto& instr1 : executed_instructions) {
        WB.push_back(instr1);
        for (int j = 0; j < IQ.size(); ++j) {
            if (instr1.dest_renamed == IQ[j].renamed_instr.src1_renamed) {
                IQ[j].renamed_instr.src1_ready = true;
                IQ[j].renamed_instr.src1_renamed = -1;
            }
            if (instr1.dest_renamed == IQ[j].renamed_instr.src2_renamed) {
                IQ[j].renamed_instr.src2_ready = true;
                IQ[j].renamed_instr.src2_renamed = -1;
            }
        }
        for (int j = 0; j < DI.size(); ++j) {
            if (instr1.dest_renamed == DI[j].src1_renamed) {
                DI[j].src1_ready = true;
                DI[j].src1_renamed = -1;
            }
            if (instr1.dest_renamed == DI[j].src2_renamed) {
                DI[j].src2_ready = true;
                DI[j].src2_renamed = -1;
            }
        }
        for (int j = 0; j < RR.size(); ++j) {
            if (instr1.dest_renamed == RR[j].src1_renamed) {
                RR[j].src1_ready = true;
                RR[j].src1_renamed = -1;
            }
            if (instr1.dest_renamed == RR[j].src2_renamed) {
                RR[j].src2_ready = true;
                RR[j].src2_renamed = -1;
            }
        }
    }
}

void processor::writeback() {
    for (int i=0; i<WB.size(); ++i) {
        if (WB[i].instr.wb_count.start == -1)
        WB[i].instr.wb_count.start = cycle_count;
        if (WB[i].instr.ex_count.total == -1 && WB[i].instr.wb_count.start != -1 && WB[i].instr.ex_count.start != -1)
        WB[i].instr.ex_count.total = WB[i].instr.wb_count.start - WB[i].instr.ex_count.start;

        ROB[WB[i].dest_renamed].ready = true;
        ROB[WB[i].dest_renamed].instr = WB[i].instr;
    }
    WB.clear();
}

void processor::retire() {
    for (int i=0; i<parameters.rob_size; ++i) {
        if (ROB[i].ready) {
            if (ROB[i].instr.rt_count.start == -1)
            ROB[i].instr.rt_count.start = cycle_count;
            if (ROB[i].instr.wb_count.total == -1 && ROB[i].instr.rt_count.start != -1)
            ROB[i].instr.wb_count.total = ROB[i].instr.rt_count.start - ROB[i].instr.wb_count.start;   

            for (int j = 0; j < DI.size(); ++j) {
                if (i == DI[j].src1_renamed) {
                    DI[j].src1_ready = true;
                    DI[j].src1_renamed = -1;
                }
                if (i == DI[j].src2_renamed) {
                    DI[j].src2_ready = true;
                    DI[j].src2_renamed = -1;
                }
            }
            for (int j = 0; j < RR.size(); ++j) {
                if (i == RR[j].src1_renamed) {
                    RR[j].src1_ready = true;
                    RR[j].src1_renamed = -1;
                }
                if (i == RR[j].src2_renamed) {
                    RR[j].src2_ready = true;
                    RR[j].src2_renamed = -1;
                }
            }
        }
    }

    int retire_count = 0;
    while (ROB[rob_head].ready && retire_count<parameters.width) {
        ROB[rob_head].ready = false;
        ROB[rob_head].occupied = false;
        ROB[rob_head].instr.rt_count.total = (cycle_count - ROB[rob_head].instr.rt_count.start) + 1;

        cout << ROB[rob_head].instr.sq_no << " fu{" << ROB[rob_head].instr.op_type << "} src{" 
        << ROB[rob_head].instr.src1 << "," << ROB[rob_head].instr.src2 << "} dst{" << ROB[rob_head].instr.dest 
        << "} FE{" << ROB[rob_head].instr.fe_count.start << "," << ROB[rob_head].instr.fe_count.total
        << "} DE{" << ROB[rob_head].instr.de_count.start << "," << ROB[rob_head].instr.de_count.total
        << "} RN{" << ROB[rob_head].instr.rn_count.start << "," << ROB[rob_head].instr.rn_count.total
        << "} RR{" << ROB[rob_head].instr.rr_count.start << "," << ROB[rob_head].instr.rr_count.total
        << "} DI{" << ROB[rob_head].instr.di_count.start << "," << ROB[rob_head].instr.di_count.total
        << "} IS{" << ROB[rob_head].instr.is_count.start << "," << ROB[rob_head].instr.is_count.total
        << "} EX{" << ROB[rob_head].instr.ex_count.start << "," << ROB[rob_head].instr.ex_count.total
        << "} WB{" << ROB[rob_head].instr.wb_count.start << "," << ROB[rob_head].instr.wb_count.total
        << "} RT{" << ROB[rob_head].instr.rt_count.start << "," << ROB[rob_head].instr.rt_count.total << "}\n";


        int dest = ROB[rob_head].instr.dest;
        if (dest!=-1 && RMT[dest].valid && RMT[dest].rob_tag == rob_head) {
            RMT[dest].valid = false;
            RMT[dest].rob_tag = -1;
        }
        if (rob_head == parameters.rob_size-1)
        rob_head = 0;
        else
        ++rob_head;

        ++retire_count;
        ++free_rob_entries;
    }
}

bool processor::advance_cycle() {
    bool rob_empty = false;
    if (rob_head == rob_tail && !ROB[rob_head].ready)
        rob_empty = true;
    int rob_filled = 0;
    for (int i=0; i<parameters.rob_size;++i) {
        if(ROB[i].occupied)
        ++rob_filled;
    }

    if (end_of_file && DE.empty() && RN.empty() && RR.empty() && DI.empty() && IQ.empty() && execute_list.empty() && WB.empty() && rob_empty) {
        cout << "# === Simulator Command =========\n";
        cout << "# ./sim " << parameters.rob_size << " " << parameters.iq_size << " " << parameters.width << " " << file_name << "\n";
        cout << "# === Processor Configuration ===\n";
        cout << "# ROB_SIZE = " << parameters.rob_size << "\n";
        cout << "# IQ_SIZE  = " << parameters.iq_size << "\n";
        cout << "# WIDTH    = " << parameters.width << "\n";
        cout << "# === Simulation Results ========\n";
        cout << "# Dynamic Instruction Count    = " << instr_count << "\n";
        cout << "# Cycles                       = " << cycle_count << "\n";
        float ips = static_cast<float>(instr_count) / static_cast<float>(cycle_count);
        cout << "# Instructions Per Cycle (IPC) = " << fixed << setprecision(2) << ips << "\n";
        
        return false;
    }
    else 
        return true;
}

int main (int argc, char* argv[]) {
    ifstream file;              
    string trace_file;      
    proc_params params;       
    int op_type, dest, src1, src2;  
    uint32_t pc; 
    
    if (argc != 5) {
        cout << "Error: Wrong number of inputs:" << argc-1 << "\n";
        exit(EXIT_FAILURE);
    }
    
    params.rob_size = strtoul(argv[1], NULL, 10);
    params.iq_size = strtoul(argv[2], NULL, 10);
    params.width = strtoul(argv[3], NULL, 10);
    trace_file = argv[4];
    
    file.open(trace_file);
    if(!file) {
        cout << "Error: Unable to open file " << trace_file << "\n";
        exit(EXIT_FAILURE);
    }

    processor proc(params);
    proc.file_name = trace_file;
    proc.load_file(trace_file);
    proc.run();

    return 0;
}
