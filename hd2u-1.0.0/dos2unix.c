/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * dos2unix '\n' converter 0.8.0
 *   based on Unix2Dos 0.9.0 by Peter Hanecak (made 19.2.1997)
 * Copyright 1997,.. by Peter Hanecak <hanecak@megaloman.sk>.
 * All rights reserved.
 *
 * Some portions by Rob Ginda <rginda@netscape.com> (07.2.2001)
 *
 * dos2unix filters reading input from stdin and writing output to stdout.
 * Without arguments it reverts the format (i.e. if source is in UNIX format,
 * output is in DOS format and vice versa).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * See the COPYING file for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include "config.h"

/* conversions */
#define CT_AUTO     0
#define CT_DOS2UNIX 1
#define CT_MAC2UNIX 2
#define CT_UNIX2DOS 3
#define CT_UNIX2MAC 4
#define CT_DOS2MAC  5
#define CT_MAC2DOS  6
#define CT_SKIP     7
#define CT_COPY     8

/* input data formats (bit mask) */
#define FT_DOS    1
#define FT_UNIX   2
#define FT_MIXED  3
#define FT_BINARY 4
#define FT_MAC    8

/* input data flags (bit mask) */
#define FF_STRAYR 1

/* read/write buffers size */
#define BUFFER_SIZE	1024

static struct option long_options[] = {
    { "auto", no_argument, NULL, 'A' },
    { "d2u", no_argument, NULL, 'U' },
    { "m2u", no_argument, NULL, 'T' },
    { "u2d", no_argument, NULL, 'D' },
    { "u2m", no_argument, NULL, 'M' },
    { "d2m", no_argument, NULL, 'O' },
    { "m2d", no_argument, NULL, 'C' },
    { "force", no_argument, NULL, 'f' },
    { "help", no_argument, NULL, 'h' },
    { "skipbin", no_argument, NULL, 'b' },
    { "test", no_argument, NULL, 't' },
    { "verbose", no_argument, NULL, 'v' },
    { "version", no_argument, NULL, 'V' },
    { 0, 0, 0, 0 }
};

char testmode = 0, verbose = 0, skipbin = 0, force = 0;
char *argv0;

/* if fn is NULL then input is stdin and output is stdout
 *
 * buffer1 vs. buffer2 size: "worst case scenario" is UNIX->DOS conversion of input consisting of only '\n' characters - for such
 * case buffer2 have to be two times bigger than buffer1
 */ 
int convert(char *fn, int convType) {
    char c;
    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE * 2], *outBuffer = (char *)&buffer2;
    int count1 = -1, count2 = -1, n1;
    int crFlag = 0, validDosEolFound = 0;
    int fileType = 0, fileFlags = 0;
    char *tempFn = NULL;
    char *tempFnTemplate = "/dos2unix-XXXXXX";
    char *tmpdir = NULL;
    int temp_fd = -1;
    mode_t old_mode;
    FILE *in = NULL, *out = NULL;
    struct stat buf;

    /* ugly little macro to clean up and return */
#define BAIL(_rval, _msg) {                                                    \
        if (in && in != stdin)                                                 \
            if (fclose(in) < 0) {                                              \
                perror(argv0); return -2;                                      \
            }                                                                  \
        if (out && out != stdout)                                              \
            if (fclose(out) < 0) {                                             \
                perror(argv0); return -2;                                      \
            }                                                                  \
        if (tempFn != NULL && temp_fd != -1 && remove(tempFn) < 0) {           \
            perror(argv0); return -2;                                          \
        }                                                                      \
        if (tempFn != NULL)                                                    \
            free(tempFn);                                                      \
        if (_rval != 0)                                                        \
            perror(_msg);                                                      \
        return _rval;                                                          \
    }

    /* open the input file, if it isn't stdin */
    if (fn != NULL) {
        if ((in = fopen(fn, "r")) == NULL)
            BAIL (-1, fn);
        if (stat(fn, &buf) != 0)
            BAIL (-1, fn);
        if (S_ISDIR(buf.st_mode)) {
            fprintf(stderr, "%s is a directory!\n", fn);
            BAIL (0, NULL);
        }
    }
    else
        in = stdin;

    /* construct temp file name */
    if ((getuid() == geteuid()) && (getgid() == getegid()))
        if (!(tmpdir=getenv("TMPDIR")))
            tmpdir=getenv("TMP");
    if (!tmpdir)
        tmpdir = "/tmp";
    tempFn = malloc(strlen(tmpdir) + strlen(tempFnTemplate) + 1);
    if (!tempFn)
        BAIL (-3, argv0);
    strcpy(tempFn, tmpdir);
    strcat(tempFn, tempFnTemplate);

    /* open a temp file to store input */
    old_mode = umask(077);
    temp_fd = mkstemp(tempFn);
    (void) umask(old_mode);
    if (temp_fd == -1)
        BAIL (-2, argv0);
    if ((out = fdopen(temp_fd, "w")) == NULL)
        BAIL (-2, argv0);

    /* initial scan for filetype and fileflags
     * also store input to temp file for later conversion */
    while (!feof(in)) {
        count1 = fread(&buffer1, sizeof(char), BUFFER_SIZE, in);
        if ((count1 != BUFFER_SIZE) && ferror(in))
            BAIL (-4, argv0);

        for (n1 = 0; n1 < count1; n1++) {
            c = buffer1[n1];

            if (crFlag) {
                crFlag = 0;
                if (c == '\n') {
                    validDosEolFound = 1;
                    continue;
                }
                fileFlags |= FF_STRAYR;
            }

            if (c == '\r') {
                crFlag = 1;
                fileType |= FT_DOS;
            } else if (c == '\n') {
                fileType |= FT_UNIX;
            } else if ((c < 32 || c > 126) && c != '\t') {
                fileType |= FT_BINARY;
            }
        }

        if ((fwrite(&buffer1, sizeof(char), count1, out) != count1) && ferror(out))
            BAIL (-5, argv0);
    }
    if (crFlag)
        fileFlags |= FF_STRAYR;
    /* recognize Mac files: after previous scan they looks like DOS files with stray '\r'-s and with no regular DOS "eol" */
    if (((fileType & FT_DOS) != 0) && (!validDosEolFound)) {
        fileType |= FT_MAC;
        fileFlags &= !FF_STRAYR;
    }

    /* if we're not working with stdin, close source (in) */
    if (fn != NULL) {
        if (fclose(in) != 0, in = NULL)
            BAIL (-2, argv0);
    }

    /* close temp file (out) */
    if (fclose(out) != 0, out = NULL)
        BAIL (-2, argv0);


    /* figure out conversion type */
    if ((fileType & FT_BINARY) != 0 && skipbin) {
            convType = CT_SKIP;
            fileType = FT_BINARY;	/* ensure we do not treat it as another format */
    }
    if (convType == CT_AUTO) {
        if ((fileType & FT_UNIX) != 0)
            convType = CT_UNIX2DOS;
        if ((fileType & FT_DOS) != 0) {
            convType = CT_DOS2UNIX;
            fileType = fileType | !FT_UNIX;	/* to ensure FT_MIXED is handled as FT_DOS */
        }
        if ((fileType & FT_MAC) != 0)
            convType = CT_MAC2UNIX;
    }

    /* don't waste time on a null conversion */
    if ((fileType == FT_DOS && convType == CT_UNIX2DOS) ||
        (fileType == FT_DOS && convType == CT_MAC2DOS) ||
        (fileType == FT_UNIX && convType == CT_DOS2UNIX) ||
        (fileType == FT_UNIX && convType == CT_MAC2UNIX) ||
        (fileType == FT_MAC && convType == CT_DOS2MAC) ||
        (fileType == FT_MAC && convType == CT_UNIX2MAC))
    {
        convType = CT_SKIP;
    }

    /* if the source is stdin, we have to reproduce input on stdout */
    if (fn == NULL && convType == CT_SKIP) {
        convType = CT_COPY;
        outBuffer = (char *)&buffer1;
    }

    /* verbose output: input data type */
    if (verbose) {
        fprintf (stderr, "File format of '%s': ", fn);
        if ((fileType & FT_BINARY) != 0)
            fprintf (stderr, "looks binary, ");
        if ((fileFlags & FF_STRAYR) != 0)
            fprintf (stderr, "contains stray '\\r', ");
        if ((fileType & FT_MIXED) == FT_MIXED)
            fprintf (stderr, "mixed line endings found.");
        else {
	    if ((fileType & FT_MAC) != 0)
                fprintf (stderr, "Mac line endings found.");
            else if ((fileType & FT_DOS) != 0)
                fprintf (stderr, "DOS line endings found.");
	    else if ((fileType & FT_UNIX) != 0)
                fprintf (stderr, "Unix line endings found.");
        }
        fprintf (stderr, "\n");
    }

    /* another ugly little macro to handle conversion type overriding for specific case */
#define CTOVERRIDE(_ft, _oct, _rct, _msg) {                                                             \
        if ((fileType == _ft && convType == _oct)) {                                                    \
            convType = _rct;                                                                            \
            fprintf (stderr, "warning: Overriding conversion type based on detected source format.\n"); \
            if (verbose)                                                                                \
                fprintf (stderr, _msg);                                                                 \
        }                                                                                               \
    }

    /* handle some source type vs. conversion type typos (or whatever) in a way it can make sense */
    if (!force) {
        CTOVERRIDE(FT_MAC, CT_DOS2UNIX, CT_MAC2UNIX, "  (MAC file + DOS -> UNIX conversion => MAC -> UNIX conversion)\n");
        CTOVERRIDE(FT_DOS, CT_MAC2UNIX, CT_DOS2UNIX, "  (DOS file + MAC -> UNIX conversion => DOS -> UNIX conversion)\n");
        CTOVERRIDE(FT_MAC, CT_UNIX2DOS, CT_MAC2DOS, "  (MAC file + UNIX -> DOS conversion => MAC -> DOS conversion)\n");
        CTOVERRIDE(FT_DOS, CT_UNIX2MAC, CT_DOS2MAC, "  (DOS file + UNIX -> MAC conversion => DOS -> MAC conversion)\n");
        CTOVERRIDE(FT_UNIX, CT_DOS2MAC, CT_UNIX2MAC, "  (UNIX file + DOS -> MAC conversion => UNIX -> MAC conversion)\n");
        CTOVERRIDE(FT_UNIX, CT_MAC2DOS, CT_UNIX2DOS, "  (UNIX file + MAC -> DOS conversion => UNIX -> DOS conversion)\n");
    }

    /* verbose output: conversion type */
    if (verbose & !testmode) {
        fprintf (stderr, "Conversion type '%s': ", fn);
        switch (convType) {
            case CT_DOS2UNIX:
                fprintf (stderr, "DOS -> Unix");
                break;
            case CT_MAC2UNIX:
                fprintf (stderr, "Mac -> Unix");
                break;
            case CT_UNIX2DOS:
                fprintf (stderr, "Unix -> DOS");
                break;
            case CT_UNIX2MAC:
                fprintf (stderr, "Unix -> Mac");
                break;
            case CT_DOS2MAC:
                fprintf (stderr, "DOS -> Mac");
                break;
            case CT_MAC2DOS:
                fprintf (stderr, "Mac -> DOS");
                break;
            case CT_COPY:
                fprintf (stderr, "none (copying source to target)");
                break;
            case CT_SKIP:
                fprintf (stderr, "none (skipping file)");
        }
        if (force)
            fprintf (stderr, " (forced)");
        fprintf (stderr, "\n");
    }


    /* whether we are continuing conversion */
    if ((convType == CT_SKIP) || testmode)
        BAIL (0, NULL);


    /* open input data in temp file */
    if ((in = fopen(tempFn, "r")) == NULL)
        BAIL (-2, argv0);

    /* open the output file, if it isn't stdout */
    if (fn != NULL) {
        if ((out = fopen(fn, "w")) == NULL)
            BAIL (-2, fn);
    }
    else
        out = stdout;

    /* do the actual conversion */
    while (!feof(in)) {
        count1 = fread(&buffer1, sizeof(char), BUFFER_SIZE, in);
        if ((count1 != BUFFER_SIZE) && ferror(in))
            BAIL (-6, argv0);

        switch (convType) {
            case CT_DOS2UNIX:
                count2 = 0;
                for (n1 = 0; n1 < count1; n1++) {
                    c = buffer1[n1];
                    if (c == '\r')
                        continue;
                    buffer2[count2++] = c;
                }
                break;
            case CT_MAC2UNIX:
                for (n1 = 0; n1 < count1; n1++) {
                    c = buffer1[n1];
                    if (c == '\r')
                        buffer2[n1] = '\n';
                    else
                        buffer2[n1] = c;
                }
                count2 = count1;
                break;
            case CT_UNIX2DOS:
                count2 = 0;
                for (n1 = 0; n1 < count1; n1++) {
                    c = buffer1[n1];
                    if (c == '\n')
                        buffer2[count2++] = '\r';
                    buffer2[count2++] = c;
                }
                break;
            case CT_UNIX2MAC:
                for (n1 = 0; n1 < count1; n1++) {
                    c = buffer1[n1];
                    if (c == '\n')
                        buffer2[n1] = '\r';
                    else
                        buffer2[n1] = c;
                }
                count2 = count1;
                break;
            case CT_DOS2MAC:
                count2 = 0;
                for (n1 = 0; n1 < count1; n1++) {
                    c = buffer1[n1];
                    if (c == '\n')
                        continue;
                    buffer2[count2++] = c;
                }
                break;
            case CT_MAC2DOS:
                count2 = 0;
                for (n1 = 0; n1 < count1; n1++) {
                    c = buffer1[n1];
                    buffer2[count2++] = c;
                    if (c == '\r')
                        buffer2[count2++] = '\n';
                }
                break;
            case CT_COPY:
                count2 = count1;
        }

        if ((fwrite(outBuffer, sizeof(char), count2, out) != count2) && ferror(out))
            BAIL (-7, argv0);
    }

    BAIL (0, NULL);

#undef BAIL

}

void help() {
    fprintf(stderr,
            "usage:\n"
            "\t%s [--verbose|-v] [--test|-t] [--force|-f] \\\n"
            "\t\t[--<x>2<y>|--auto|-<Z>] \\\n"
            "\t\t[<file name> [...]]\n"
            "where:\n", argv0);
    fprintf(stderr,
            "\t--auto, -A\toutput will be set based upon auto-detection\n"
            "\t\t\tof source format\n"
            "\t--d2u, -U\tperform DOS -> UNIX conversion\n"
            "\t--m2u, -T\tperform MAC -> UNIX conversion\n"
            "\t--u2d, -D\tperform UNIX -> DOS conversion\n"
            "\t--u2m, -M\tperform UNIX -> MAC conversion\n"
            "\t--d2m, -O\tperform DOS -> MAC conversion\n"
            "\t--m2d, -C\tperform MAC -> DOS conversion\n"
            "\n");
    fprintf(stderr,
            "\t--force\t\tsuppress internal conversion type corrections\n"
            "\t\t\tbased on autodetected input format\n"
            "\t--skipbin, -b\tskip binary files\n"
            "\t--test, -t\tdon't write any conversion results; useful with\n"
            "\t\t\t--verbose to just report on source type\n"
            "\t--verbose, -v\tprint extra information on stderr\n"
            "\t--version, -V\tprint version information on stderr\n"
            "\n");
    fprintf(stderr,
            "- when no options are given then input format will be automatically detected\n"
            "  and converted as follows:\n"
            "\tDOS -> UNIX\n"
            "\tMAC -> UNIX\n"
            "\tUNIX -> DOS\n"
            "- same as above applies if --auto option is used\n"
            "- when no file is given, then stdin is used as input and stdout as output\n"
            "- binary files will be skipped automatically if option --skipbin\n"
            "  (or -b) is used\n"
            "- stray '\\r' characters (without a following '\\n') in files in DOS format are\n"
            "  reported but only conversion 'DOS -> Unix' affects them (they are skipped)\n");
    fprintf(stderr,
            "- stray '\\n' characters in files in MAC format are not detected for now\n");
}

void displayVersion() {
	fprintf(stderr, PACKAGE_STRING "\n");
}

int main(int argc, char *argv[]) {
    int convType = CT_AUTO;
    int o;

    argv0 = argv[0];

    /* process parameters */
    while ((o = getopt_long(argc, argv, "ACDMOTUVbfhvt", long_options, NULL)) != EOF) {
        switch (o) {
            case 'A':
                convType = CT_AUTO;
                break;
            case 'C':
                convType = CT_MAC2DOS;
                break;
            case 'D':
                convType = CT_UNIX2DOS;
                break;
            case 'M':
                convType = CT_UNIX2MAC;
                break;
            case 'O':
                convType = CT_DOS2MAC;
                break;
            case 'T':
                convType = CT_MAC2UNIX;
                break;
            case 'U':
                convType = CT_DOS2UNIX;
                break;
            case 'b':
                skipbin = 1;
                break;
            case 'f':
                force = 1;
                break;
            case 'h':
                help();
                return 0;
            case 't':
                testmode = 1;
                break;
            case 'v':
                verbose = 1;
                displayVersion();
                break;
            case 'V':
                displayVersion();
                return 0;
            case '?':
                help();
                return -1;
        }
    }

    if (optind < argc) {
        while(optind < argc)
            if ((o = convert(argv[optind++], convType)) != 0)
                break;
    }
    else
        o = convert(NULL, convType);

    return o;
}
