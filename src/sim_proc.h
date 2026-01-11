#ifndef SIM_PROC_H
#define SIM_PROC_H
#include <vector>
#include <fstream>
using namespace std;

struct proc_params {
    int rob_size;
    int iq_size;
    int width;
};

struct cycle_count {
    int start, total;
};

struct instruction {
    uint32_t pc;
    int op_type, dest, src1, src2;
    int sq_no;
    cycle_count fe_count, de_count, rn_count, rr_count, di_count, is_count, ex_count, wb_count, rt_count;
};

struct renamed_instruction {
    instruction instr;
    bool src1_ready, src2_ready;
    int dest_renamed, src1_renamed, src2_renamed;
};

struct renamed_instruction_execute {
    renamed_instruction renamed_instr;
    int timer;
};

struct rob_entry {
    bool ready;
    bool occupied;
    instruction instr;
};

struct iq_entry {
    bool valid;
    renamed_instruction renamed_instr;
};

struct rmt_entry {
    bool valid;
    uint16_t rob_tag;
    instruction instr;
};

class processor {
public:
    processor(const proc_params& params);
    void run();
    void load_file(const string& trace_file);
    string file_name;

private:
    bool end_of_file;
    ifstream file;
    int instr_count;
    int cycle_count;
    proc_params parameters;
    vector<instruction> DE, RN;
    vector<renamed_instruction> RR, DI;
    vector<iq_entry> IQ;
    vector<renamed_instruction_execute> execute_list;
    vector<renamed_instruction> WB;
    vector<rob_entry> ROB;
    vector<rmt_entry> RMT;
    int rob_head, rob_tail, free_rob_entries;

    void fetch();
    void decode();
    void rename();
    void reg_read();
    void dispatch();
    void issue();
    void execute();
    void writeback();
    void retire(); 
    bool advance_cycle();

};

#endif
