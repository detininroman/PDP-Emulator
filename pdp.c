#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "pdp.h"

word reg [REGS_NUM] = {};
byte mem [MEM_SIZE] = {};

#define sp reg [6]
#define pc reg [7]

int N = 0;
int Z = 0;

struct SSDD ss, dd;
word nn = 0;
word r  = 0;
word xx = 0;

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
    unsigned int value    = 0;

    while (fscanf (file, "%x%x", &address, &quantity) == 2)
    {
        for (cnt = 0; cnt < quantity; cnt ++)
        {
            fscanf (file, "%x", &value);
            b_write (address + cnt , (byte) value);
        }
    }
}

void dump ()
{
#ifdef DEBUG_MODE
    printf ("\n\t\t\t\t");
    for (int i = 0; i < 8; i++)
    {
        printf ("R[%d] = %o  ", i, reg[i]);
    }
#else
    printf ("\n");
    for (int i = 0; i < 8; i++)
    {
        printf ("R[%d] = %o\n", i, reg[i]);
    }
#endif
}

void do_halt ()
{
    dump ();
    printf ("\n");
    exit (0);
}

void do_sob ()
{
    if (--reg[r] != 0)
    {
        pc = pc - 2 * nn;
    }
#ifdef DEBUG_MODE
    dump ();
#endif
}

void do_clr ()
{
    reg [dd.a] = 0;
    N = 0;
    Z = 1;
#ifdef DEBUG_MODE
    dump ();
#endif
}

void do_br ()
{
    pc = pc + 2 * xx;
#ifdef DEBUG_MODE
    dump ();
#endif
}

void do_beq ()
{
    if (Z == 1)
    {
        do_br ();
    }
#ifdef DEBUG_MODE
    dump ();
#endif
}

void do_mov ()
{
    reg [dd.a] = ss.val;
    change_state_flags (reg[dd.a]);
#ifdef DEBUG_MODE
    dump ();
#endif
}

void do_add ()
{
    reg [dd.a] = ss.val + dd.val;
    change_state_flags (reg[dd.a]);
#ifdef DEBUG_MODE
    dump ();
#endif
}

void do_unknown ()
{
    printf ("Unknown command!\n");
    assert (0);
}

void change_state_flags (word result)
{
    N = (result <  0)? 1: 0;
    Z = (result == 0)? 1: 0;
}

struct SSDD get_mr (word w)
{
    struct SSDD res;
    int n = w & 7;
    int mode = (w >> 3) & 7;
    word b = get_b (w >> 15);
    switch (mode)
    {
        case 0: //R1
        {
            res.a   = n;
            res.val = reg [n];
            printf ("R%d ", n);
            break;
        }
        case 1: //(R1)
        {
            res.a = reg [n];
            if (b == 1)
            {
                res.val = b_read (res.a);
            }
            else
            {
                res.val = w_read (res.a);
            }
            printf ("(R%d) ", n);
            break;
        }
        case 2: //(R1)+
        {
            res.a = reg [n];
            if (b == 1)
            {
                res.val = b_read (res.a);
                if ((res.val >> 7) == 1)
                {
                    res.val = res.val | 0xFF00;
                }
                reg [n] += (n == 6 || n == 7)? 2: 1;
            }
            else
            {
                res.val = w_read (res.a);
                reg [n] += 2;
            }

            if (n == 7)
            {
                printf ("#%o ", res.val);
            }
            else
            {
                printf ("(R%d)+ ", n);
            }
            break;
        }
        case 3: //@(R2)+
        {
            res.a   = w_read (reg[n]);
            res.val = w_read (res.a);
            reg [n] += 2;

            if (n == 7)
            {
                printf ("@#%o ", res.val);
            }
            else
            {
                printf ("@(R%d)+ ", n);
            }
            break;
        }
        case 4: //-(R2)
        {
            res.a = reg [n];
            if (b == 1)
            {
                res.val = b_read (res.a);
                if (res.val >> 7 == 1)
                {
                    res.val = res.val | 0xFF00;
                }
                reg [n] -= (n == 6 || n == 7)? 2: 1;
            }
            else
            {
                res.val = w_read (res.a);
                reg [n] -= 2;
            }
            printf ("-(R%d) ", n);
            break;
        }
        case 5: //@-(R2)
        {
            reg [n] -= 2;
            res.a   = w_read (reg[n]);
            res.val = w_read (res.a);
            printf ("@-(R%d) ", n);
        }
        default: printf ("Unknown mode\n"); exit (3);
    }
    return res;
}

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

                if (cmd.param & HAS_SS) { ss = get_mr (w >> 6); }
                if (cmd.param & HAS_DD) { dd = get_mr (w);      }
                if (cmd.param & HAS_NN) { nn = get_nn (w);      }
                if (cmd.param & HAS_R)  { r  = get_r  (w);      }
                if (cmd.param & HAS_XX) { xx = get_xx (w);      }

                cmd.do_action (); break;
            }
        }
        printf ("\n");
    }
}

word get_r  (word w) { return (w >> 6) & 7; }
word get_b  (word w) { return w & 1;        }
word get_nn (word w) { return w & 0x3F;     }

word  get_xx (word w)
{
    w = w & 0xFF;
    int ret_val = (w >> 7 == 1)? (w-0x100): w;
    return ret_val;
}

int main (int argc, char *argv[])
{
    FILE* file = fopen (argv[1], "r");
    if (!argv [1])
    {
        printf ("You haven't entered the file!\n"); exit (1);
    }
    if (!file)
    {
        printf ("Cannot open the file!\n"); exit (2);
    }

    load_file (file);
    fclose(file);
    run_program ();
    return 0;
}
