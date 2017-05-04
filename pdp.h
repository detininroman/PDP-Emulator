typedef unsigned char      byte;
typedef unsigned short int word;
typedef unsigned int        adr;

word reg [8]       = {};
byte mem [64*1024] = {};

#define pc reg [7]
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD 1<<1
#define HAS_R  1<<2
#define HAS_NN 1<<3

struct Command
{
    word mask;
    word opcode;
    char* name;
    void (*do_action) ();
    word param;
};

struct SSDD { word val; adr a; } ss, dd;
word nn;
word r;

byte b_read      (adr a);
word w_read      (adr a);
void b_write     (adr a, byte val);
void w_write     (adr a, word val);
void load_file   (FILE* file);
void mem_dump    (adr start, word n);
void do_halt     ();
void do_add      ();
void do_mov      ();
void do_sob      ();
void do_clr      ();
void do_unknown  ();
void run_program ();

struct Command command_list[] = {
    {0xFFFF,  0,         "HALT", do_halt,        NO_PARAM},
    {0170000, 0010000,    "MOV", do_mov,  HAS_SS | HAS_DD},
    {0170000, 0060000,    "ADD", do_add,  HAS_SS | HAS_DD},
    {0177000, 0077000,    "SOB", do_sob,  HAS_R  | HAS_NN},
    {0177700, 0005000,    "CLR", do_clr,           HAS_DD},
    {0,       0,      "unknown", do_unknown,     NO_PARAM}
};
