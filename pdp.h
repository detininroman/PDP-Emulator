#ifndef PDP_H
#define PDP_H

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int    adr;

enum
{
    REGS_NUM = 8,
    MEM_SIZE = 64 * 1024
};

enum
{
    NO_PARAM = 0,
    HAS_SS   = 1,
    HAS_DD   = 1<<1,
    HAS_R    = 1<<2,
    HAS_NN   = 1<<3,
    HAS_B    = 1<<4,
    HAS_XX   = 1<<5
};

struct Command
{
    word mask;
    word opcode;
    char* name;
    void (*do_action) ();
    word param;
};

struct SSDD
{
    word val;
    adr a;
};

byte b_read             (adr a);
word w_read             (adr a);
void b_write            (adr a, byte val);
void w_write            (adr a, word val);
void load_file          (FILE* file);
void change_state_flags (word result);
word get_b              (word w);
word get_r              (word w);
word get_nn             (word w);
int  get_xx             (word w);
void dump               ();
void do_halt            ();
void do_add             ();
void do_mov             ();
void do_movb            ();
void do_sob             ();
void do_clr             ();
void do_beq             ();
void do_br              ();
void do_unknown         ();
void run_program        ();

struct Command command_list[] = {
    {0xFFFF, 0,       "HALT", do_halt,                NO_PARAM},
    {0xF000, 0010000,  "MOV", do_mov,  HAS_SS | HAS_DD | HAS_B},
    {0xF000, 0110000, "MOVB", do_movb, HAS_SS | HAS_DD | HAS_B},
    {0xF000, 0060000,  "ADD", do_add,          HAS_SS | HAS_DD},
    {0xFE00, 0077000,  "SOB", do_sob,          HAS_R  | HAS_NN},
    {0xFFC0, 0005000,  "CLR", do_clr,                   HAS_DD},
    {0xFF00, 0001400,  "BEQ", do_beq,                   HAS_XX},
    {0xFF00, 0000400,   "BR", do_br,                    HAS_XX},
    {0,      0,    "unknown", do_unknown,             NO_PARAM}
};

#endif
