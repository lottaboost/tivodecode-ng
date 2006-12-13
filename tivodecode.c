/*
 * tivodecode, (c) 2006, Jeremy Drake
 * See COPYING file for license terms
 *
 * derived from mpegcat, copyright 2006 Kees Cook, used with permission
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "getopt_long.h"
#include "happyfile.h"
#include "tivo-parse.h"
#include "turing_stream.h"
#include "tivodecoder.h"

#ifdef WIN32
#   define HOME_ENV_NAME "USERPROFILE"
#   define DEFAULT_EMPTY_HOME "C:"
#else
#   define HOME_ENV_NAME "HOME"
#   define DEFAULT_EMPTY_HOME ""
#endif

static const char MAK_DOTFILE_NAME[] = "/.tivodecode_mak";

static const char * VERSION_STR = "CVS Head";

int o_verbose = 0;
int o_no_verify = 0;

happy_file * hfh=NULL;
// file position options
off_t begin_at = 0;


static int hread_wrapper (void * mem, int size, void * fh)
{
    return (int)hread (mem, size, (happy_file *)fh);
}

static int fwrite_wrapper (void * mem, int size, void * fh)
{
    return (int)fwrite (mem, 1, size, (FILE *)fh);
}

static struct option long_options[] = {
    {"mak", 1, 0, 'm'},
    {"out", 1, 0, 'o'},
    {"help", 0, 0, 'h'},
    {"verbose", 0, 0, 'v'},
    {"version", 0, 0, 'V'},
    {"no-verify", 0, 0, 'n'},
    {0, 0, 0, 0}
};

static void do_help(char * arg0, int exitval)
{
    fprintf(stderr, "Usage: %s [--help] [--verbose|-v] [--no-verify|-n] {--mak|-m} mak [{--out|-o} outfile] <tivofile>\n\n", arg0);
#define ERROUT(s) fprintf(stderr, s)
    ERROUT ("  --mak, -m        media access key (required)\n");
    ERROUT ("  --out, -o        output file (default stdout)\n");
    ERROUT ("  --verbose, -v    verbose\n");
    ERROUT ("  --no-verify, -n  do not verify MAK while decoding\n");
    ERROUT ("  --version, -V    print the version information and exit\n\n");
    ERROUT ("  --help, -h       print this help and exit\n\n");
    ERROUT ("The file names specified for the output file or the tivo file may be -, which\n");
    ERROUT ("means stdout or stdin respectively\n\n");
#undef ERROUT

    exit (exitval);
}

static void do_version(int exitval)
{
    fprintf (stderr, "tivodecode version %s\n", VERSION_STR);
    fprintf (stderr, "Copyright (c) 2006, Jeremy Drake\n");
    fprintf (stderr, "See COPYING file in distribution for details\n");

    exit (exitval);
}


int main(int argc, char *argv[])
{
    unsigned int marker;
    unsigned char byte;
    char first = 1;

    int running = 1;

    char * tivofile = NULL;
    char * outfile = NULL;
    char mak[12];

    int makgiven = 0;

    turing_state turing;

    FILE * ofh;

    memset(&turing, 0, sizeof(turing));
    memset(mak, 0, sizeof(mak));

    fprintf(stderr, "Encryption by QUALCOMM ;)\n\n");

    while (1)
    {
        int c = getopt_long (argc, argv, "m:o:hnvV", long_options, 0);

        if (c == -1)
            break;

        switch (c)
        {
            case 'm':
                strncpy(mak, optarg, 11);
                mak[11] = '\0';
                makgiven = 1;
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'h':
                do_help(argv[0], 1);
                break;
            case 'v':
                o_verbose = 1;
                break;
            case 'n':
                o_no_verify = 1;
                break;
            case '?':
                do_help(argv[0], 2);
                break;
            case 'V':
                do_version(10);
                break;
            default:
                do_help(argv[0], 3);
                break;
        }
    }

    if (optind < argc)
    {
        tivofile=argv[optind++];
        if (optind < argc)
            do_help(argv[0], 4);
    }

    if (!makgiven)
    {
        char * mak_fname;
        FILE * mak_file;
        const char * home_dir = getenv(HOME_ENV_NAME);
        size_t home_dir_len;
        
        if (!home_dir)
            home_dir = DEFAULT_EMPTY_HOME;
        
        home_dir_len = strlen(home_dir);

        mak_fname = malloc (home_dir_len + sizeof(MAK_DOTFILE_NAME));
        if (!mak_fname)
        {
            fprintf(stderr, "error allocing string for mak config file name\n");
            exit(11);
        }

        memcpy (mak_fname, home_dir, home_dir_len);
        memcpy (mak_fname + home_dir_len, MAK_DOTFILE_NAME, sizeof(MAK_DOTFILE_NAME));

        if ((mak_file = fopen(mak_fname, "r")))
        {
            if (fread(mak, 1, 11, mak_file) >= 10)
            {
                int i;
                for (i = 11; i >= 10 && (mak[i] == '\0' || isspace(mak[i])); --i)
                {
                    mak[i] = '\0';
                }

                makgiven = 1;
            }
            else if (ferror(mak_file))
            {
                perror ("reading mak config file");
                exit(12);
            }
            else
            {
                fprintf(stderr, "mak too short in mak config file\n");
                exit(13);
            }

            fclose (mak_file);
        }

        free(mak_fname);
    }

    if (!makgiven || !tivofile)
    {
        do_help(argv[0], 5);
    }

    if (!strcmp(tivofile, "-"))
    {
        hfh=hattach(stdin);
    }
    else
    {
        if (!(hfh=hopen(tivofile, "rb")))
        {
            perror(tivofile);
            return 6;
        }
    }

    if (!outfile || !strcmp(outfile, "-"))
    {
        ofh = stdout;
    }
    else
    {
        if (!(ofh = fopen(outfile, "wb")))
        {
            perror("opening output file");
            return 7;
        }
    }

    if ((begin_at = setup_turing_key (&turing, hfh, &hread_wrapper, mak)) < 0)
    {
        return 8;
    }

    if (hseek(hfh, begin_at, SEEK_SET) < 0)
    {
        perror ("seek");
        return 9;
    }

    marker = 0xFFFFFFFF;
    while (running)
    {
        if ((marker & 0xFFFFFF00) == 0x100)
        {
            int ret = process_frame(byte, &turing, htell(hfh), hfh, &hread_wrapper, ofh, &fwrite_wrapper);
            if (ret == 1)
            {
                marker = 0xFFFFFFFF;
            }
            else if (ret == 0)
            {
                fwrite(&byte, 1, 1, ofh);
            }
            else if (ret < 0)
            {
                perror ("processing frame");
                return 10;
            }
        }
        else if (!first)
        {
            fwrite(&byte, 1, 1, ofh);
        }
        marker <<= 8;
        if (hread(&byte, 1, hfh) == 0)
        {
            fprintf(stderr, "End of File\n");
            running = 0;
        }
        else
            marker |= byte;
        first = 0;
    }

    destruct_turing (&turing);

    if (hfh->fh == stdin)
        hdetach(hfh);
    else
        hclose(hfh);

    if (ofh != stdout)
        fclose(ofh);

    return 0;
}

/* vi:set ai ts=4 sw=4 expandtab: */
