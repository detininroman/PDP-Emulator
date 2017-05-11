#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "pdp.h"

word reg [NUMBER_OF_REGS] = {};
byte mem [MEM_SIZE]       = {};

#define sp reg [6]
#define pc reg [7]

int N = 0;
int Z = 0;

struct SSDD ss, dd;

word nn, r, xx;

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
}

void mem_dump (adr start, word n)
{
    for (int cnt = 0; cnt < n; cnt += 2)
    {
        assert (cnt % 2 == 0);
        printf ("%.6o : %.6o\n",
            start + cnt, w_read (start + cnt));
    }
}

void do_halt ()
{
    printf ("\n");
    for (int i = 0; i < 8; i++)
    {
        printf ("R[%d] = %o\n", i, reg[i]);
    }
    exit (0);
}

void do_sob ()
{
    if (--reg[r] != 0)
    {
        pc = pc - 2 * nn;
    }
}

void do_clr ()
{
    reg [dd.a] = 0;
    N = 0;
    Z = 1;
}

void do_br ()
{
    pc = pc + 2 * xx;
}

void do_beq ()
{
    if (Z == 1)
    {
        do_br ();
    }
}

void do_mov ()
{
    reg [dd.a] = ss.val;
    //if (dd.place =
    change_state_flags (reg[dd.a]);
}

void do_add ()
{
    reg [dd.a] = ss.val + dd.val;
    change_state_flags (reg[dd.a]);
}

void do_unknown () {}

void change_state_flags (word result)
{
    (result <  0)? N = 1: N = 0;
    (result == 0)? Z = 1: Z = 0;
}

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

            res.a    = reg [n];
            res.val  = w_read (res.a);
            reg [n] += 2;
            if (n == 7) printf ("#%o ", res.val);
            else        printf ("(R%d)+ ", n);
            break;

        case 3: //@(R2)+

            res.a   = w_read (reg [n]);
            res.val = w_read (res.a);
            reg [n] += 2;
            if (n == 7) printf ("@#%o ", res.val);
            else        printf ("@(R%d)+ ", n);
            break;

        case 4: //-(R2)

            reg [n] += 2;
            res.a    = reg [n];
            res.val  = w_read (res.a);
            if (n == 7) printf ("#%o ", res.val);
            else        printf ("-(R%d) ", n);
            break;

        case 5: //@-(R2)

            reg [n] += 2;
            res.a   = w_read (reg [n]);
            res.val = w_read (res.a);
            if (n == 7) printf ("@#%o ", res.val);
            else        printf ("@-(R%d) ", n);
            break;

        default: printf ("Unknown mode\n"); exit (0);

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

word get_r (word w)
{
    return (w >> 6) & 7;
}

word get_nn (word w)
{
    return w & 077;
}

word get_xx (word w)
{
    w = w & 0xFF;

    if (w >> 7 == 1)
    {
        return -(0x100 - w);
    }

    else
    {
        return  w;
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
