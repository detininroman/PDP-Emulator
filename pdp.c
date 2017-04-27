#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef unsigned char      byte;
typedef unsigned short int word;
typedef unsigned int        adr;

word reg [8] = {};
byte mem [64*1024] = {};

#define pc reg [7]
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD 1<<1

struct Command
{
    word mask;
    word opcode;
    char* name;
    void (*do_action) ();
    word param;
};

byte b_read      (adr a);
word w_read      (adr a);
void b_write     (adr a, byte val);
void w_write     (adr a, word val);
void load_file   (FILE* file);
void mem_dump    (adr start, word n);
void do_halt     ();
void do_add      ();
void do_mov      ();
void do_unknown  ();
void run_program ();


byte b_read (adr a)
{
    return mem [a];
}


word w_read (adr a)
{
    word val = 0;
    val = val | mem[a+1];
    val = val << 8;
    val = val | mem[a];

    return val;
}


void b_write (adr a, byte val)
{
    mem [a] = val;
}


void w_write (adr a, word val)
{
    mem [a]   = val;
    mem [a+1] = (val >> 8);
}


void load_file (FILE* file)
{
    unsigned int address  = 0;
    unsigned int cnt      = 0;
    unsigned int quantity = 0;
    byte value            = 0;

    while (fscanf (file, "%x %x", &address, &quantity) == 2)
        {
            for (cnt = 0; cnt < quantity; cnt ++)
            {
                fscanf (file, "%hhx", &value);
                b_write (address + cnt , value);
            }
        }
    //mem_dump (address, quantity);
}

void mem_dump (adr start, word n)
{
    int cnt = 0;

    for (cnt = 0; cnt < n; cnt += 2)
    {
        assert (cnt % 2 == 0);
        printf ("%.6o : %.6o\n",
                start + cnt, w_read (start + cnt));
    }
}

struct SSDD {
    word val;
    adr a;
} ss, dd;

void do_halt ()
{
    printf ("\n");
    for (int i = 0; i < 8; i++)
    {
        printf ("R[%d] = %d\n", i, reg[i]);
    }
    exit (0);
}

void do_mov ()
{
    reg [dd.a] = ss.val;
    //w_write (dd.a, ss.val);
}

void do_add ()
{
    reg [dd.a] = ss.val + dd.val;
    //w_write (dd.a, ss.val + dd.val);
}

void do_unknown () {}

struct SSDD get_mr (word w)
{
    struct SSDD res;
    int n = w & 7;
    int mode = (w >> 3) & 7;

    switch (mode)
    {
        case 0: //R1

            res.a   = n;
            res.val = reg[n];
            printf ("R%d ", n);
            break;

        case 1: //(R1)

            res.a   = reg [n];
            res.val = w_read (res.a);
            printf ("(R%d)  ", n);
            break;

        case 2: //(R1)+

            res.a = reg [n];
            res.val =  w_read (res.a);
            reg [n] += 2;
            if (n == 7)
                printf ("#%o ", res.val);
            else
                printf ("(R%d)+ ", n);
            break;

        default: printf ("Unknown node\n"); exit (0);

    }
    return res;
}

struct Command command_list[] = {
    {0xFFFF,  0,          "HALT", do_halt,           NO_PARAM},
    {0170000, 0010000,     "MOV", do_mov,     HAS_SS | HAS_DD},
    {0170000, 0060000,     "ADD", do_add,     HAS_SS | HAS_DD},
    {0,       0,       "UNKNOWN", do_unknown}
};


void run_program ()
{
    pc = 01000;

    while (1)
    {
        word w = w_read (pc);
        printf ("%.6o : %.6o", pc, w);
        pc += 2;

        for (int cnt = 0; ;cnt++)
        {
            struct Command cmd = command_list [cnt];
            if ((w & cmd.mask) == cmd.opcode)
            {
                printf (" %s ", cmd.name);

                if (cmd.param & HAS_SS)
                {
                    ss = get_mr (w >> 6);
                }
                if (cmd.param & HAS_DD)
                {
                    dd = get_mr (w);
                }
                cmd.do_action ();
                break;
            }
        }
        printf ("\n");
    }
}


int main (int argc, char *argv[])
{
    FILE* file = fopen (argv[1], "r");
    perror (argv[1]);

    load_file (file);
    run_program ();
    fclose (file);

    return 0;
}
