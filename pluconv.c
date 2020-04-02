/*
* pluconv.c 
* (c) 2007 
* The ROM in the IBM PC starts the boot process by performing a hardware
* initialization and a verification of all external devices.  If all goes
* well, it will then load from the boot drive the sector from track 0, head 0,
* sector 1.  This sector is placed at physical address 07C00h.
* bla blah 
* about to three years ago
*/
#include <stdio.h>
#include <ctype.h>
#include <mem.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PATH_LNG (80)
#define BUFFER_LNG (1024*4)
#define CELL_CN (49)
#define FIELD_SZ (128)

FILE *infp,*outfp[2];
char in_fname[PATH_LNG+1];
char o_fname[2][PATH_LNG+1];

char buf[BUFFER_LNG+1];
char *bufptr;

int rowno = 0;

char pfields[CELL_CN][FIELD_SZ+1];

unsigned prm_plu = 0;
unsigned prm_prom = 0;
unsigned done = 0;
int in_count = 0;

void usage(void)
{
    static char help[] =
    "usage: <input-file> <output1> <output2>\n" \
    "       if only output1 is specified, then output2 won't be created.\n" \
    "       output1 is the PLU output\n" \
    "       output2 is the PLU promo output\n";

    printf(help);
    exit (1);
}

int get_items(void)
{
    int length = 0;
    int cell_no;
    int cell_lng = 0;

bufget:

    if (!feof(infp))
    {
        if (fgets(buf,BUFFER_LNG,infp) == NULL)
        {
            done = 1;
            length = 0;
        }
        rowno++;
        buf[length - 1] = '\0';
        bufptr = buf;
        length = strlen(buf);

        if (*bufptr == NULL) goto bufget;

        for (cell_no = 0; cell_no < CELL_CN; cell_no++)
             memset(pfields[cell_no],'\0',FIELD_SZ);

        cell_no = 0;

        while (*bufptr!='\0')
        {
            if (*bufptr == '|')
            {
                pfields[cell_no][cell_lng] = '\0';
                cell_no++;
                bufptr++;
                cell_lng = 0;
                continue;
            }
            if (cell_lng > FIELD_SZ) {
                printf("warning: cell length exceeded %d length limit\n",FIELD_SZ);
                cell_lng = FIELD_SZ;
            }
            pfields[cell_no][cell_lng] = *bufptr;
            cell_lng++;
            bufptr++;
        }
    }
    else done = 1;
    return length;
}

void compile(void)
{
    int cell_no;
    struct tm *t;
    time_t l_time;
    int y,d,m;
    char yy[4+1],mm[2+1],dd[2+1];

    l_time = time(NULL);
    t = localtime(&l_time);

    rowno = 0;

    while (!done)
    {
         if (get_items() == 0) continue;
         if (prm_plu)
         {
             for (cell_no = 0; cell_no < CELL_CN; cell_no++)
             {
                  if (cell_no) fputc('|',outfp[0]);
                  
                  if (cell_no == 16) {
                      if (pfields[cell_no] == '\0') {
                          fputc('P',outfp[0]);
                          continue;
                      }
                      if (strcmp(pfields[cell_no],"KG") == 0)
                          fputc('G',outfp[0]);
                      else 
                          fputc('P',outfp[0]);
                  }
                  else if (cell_no == 18) {
                      if (pfields[cell_no] == '\0') {
                          fputc('P',outfp[0]);
                          continue;
                      }
                      if (strcmp(pfields[cell_no],"KG") == 0 ||
                          strcmp(pfields[cell_no],"G") == 0)
                          fputc('G',outfp[0]);
                      else
                          fputc('P',outfp[0]);
                  }
                  else {
                     if (pfields[cell_no] == '\0') continue;
                     fputs(pfields[cell_no],outfp[0]);
                  }
             }
             fputc('\n',outfp[0]);
         }

         if (prm_prom && (pfields[3] != '\0' || atof(pfields[3]) != 0.00))
         {
            if (pfields[33] != '\0' && strlen(pfields[33]) == 10)
            {
                memset(dd,'\0',3);
                memset(mm,'\0',3);
                memset(yy,'\0',5);

                dd[0] = pfields[33][0];
                dd[1] = pfields[33][1];

                mm[0] = pfields[33][3];
                mm[1] = pfields[33][4];

                yy[0] = pfields[33][6];
                yy[1] = pfields[33][7];
                yy[2] = pfields[33][8];
                yy[3] = pfields[33][9];

                d = atoi(dd);
                m = atoi(mm);
                y = atoi(yy);

            }
            else {
               continue;
            }

            if (y >= (t->tm_year + 1900))
            {
                if (y == (t->tm_year + 1900)) {
                    if (m < (t->tm_mon + 1)) continue;
                    if ((m == (t->tm_mon + 1)) && (d < t->tm_mday)) continue;
                }
                if (pfields[3] != '\0')
                    fputs(pfields[3],outfp[1]);
                fputc('|',outfp[1]);
                if (pfields[31] != '\0')
                    fputs(pfields[31],outfp[1]);
                fputc('|',outfp[1]);
                if (pfields[32] != '\0')
                {
                    memset(dd,'\0',3);
                    memset(mm,'\0',3);
                    memset(yy,'\0',5);

                    dd[0] = pfields[32][0];
                    dd[1] = pfields[32][1];

                    mm[0] = pfields[32][3];
                    mm[1] = pfields[32][4];

                    yy[0] = pfields[32][6];
                    yy[1] = pfields[32][7];
                    yy[2] = pfields[32][8];
                    yy[3] = pfields[32][9];

                    fputs(yy,outfp[1]);
                    fputs(mm,outfp[1]);
                    fputs(dd,outfp[1]);
                    fputs("0000",outfp[1]);
                }
                fputc('|',outfp[1]);

                if (pfields[33] != '\0')
                {
                    memset(dd,'\0',3);
                    memset(mm,'\0',3);
                    memset(yy,'\0',5);

                    dd[0] = pfields[33][0];
                    dd[1] = pfields[33][1];

                    mm[0] = pfields[33][3];
                    mm[1] = pfields[33][4];

                    yy[0] = pfields[33][6];
                    yy[1] = pfields[33][7];
                    yy[2] = pfields[33][8];
                    yy[3] = pfields[33][9];

                    fputs(yy,outfp[1]);
                    fputs(mm,outfp[1]);
                    fputs(dd,outfp[1]);
                    fputs("2359",outfp[1]);
                }
                fputc('\n',outfp[1]);

                in_count++;
            }
         }
    }
}

int main(int argc, char *argv[])
{
    int arg_in = 1;
    int length;

    if (argc < 3) usage();

    while (arg_in < argc)
    {
       length = strlen(argv[arg_in]);
       if (length > 80)
       {
           printf("error: input/output filename is too long\n");
           exit (1);
       }
       if (arg_in == 1) {
           strcpy(in_fname,argv[arg_in]);
       }
       else if (arg_in == 2) {
           strcpy(o_fname[0],argv[arg_in]);
           prm_plu = 1;
       }
       else if (arg_in == 3) {
           strcpy(o_fname[1],argv[arg_in]);
           prm_prom = 1;
       }
       arg_in++;
    }

    if ((infp = fopen(in_fname,"r+t")) == NULL) {
       printf("error: unable to open '%s' input-file\n",in_fname);
       return 1;
    }

    if (prm_plu == 1)
    {
        if ((outfp[0] = fopen(o_fname[0],"w+t")) == NULL) {
           printf("error: unable to create output file\n");
           fclose (infp);
           return 1;
        }
    }

    if (prm_prom == 1)
    {
        if ((outfp[1] = fopen(o_fname[1],"w+t")) == NULL) {
           printf("error: unable to create output file\n");
           fclose (infp);
           return 1;
        }
    }

    compile();

    if (prm_plu == 1) fclose (outfp[0]);
    if (prm_prom == 1) fclose (outfp[1]);
    if (in_count <= 0)
        unlink(o_fname[1]);

    fclose (infp);

    return 0;
}
