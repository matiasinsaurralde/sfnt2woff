#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef WIN32
#include <io.h>
#endif

#include "woff.h"

static void
die(const char * msg)
{
  fprintf(stderr, "# fatal error: %s\n", msg);
  exit(2);
}

static void
reportErr(uint32_t status)
{
  woffPrintStatus(stderr, status, "### ");
  exit(status & 0xff);
}

static void
usage(const char * progName)
{
  fprintf(stderr, "Usage:\n"
                  "  %s [-v | -m | -p] <woff>\n"
                  "    decode WOFF file <woff>, writing OpenType data to stdout\n"
                  "Options (instead of decoding to OpenType format)\n"
                  "    -v   write font version to stdout\n"
                  "    -m   write WOFF metadata block to stdout\n"
                  "    -p   write private data block to stdout\n"
                  "Note: only one of -v, -m, -p may be used at a time.\n"
                  , progName);
}

const uint8_t *
readFile(const char * name, uint32_t * len)
{
  FILE * inFile = fopen(name, "rb");
  if (!inFile) {
    char buf[200];
    sprintf(buf, "unable to open file %s", name);
    die(buf);
  }

  if (fseek(inFile, 0, SEEK_END) != 0)
    die("seek failure");
  *len = ftell(inFile);
  if (fseek(inFile, 0, SEEK_SET) != 0)
    die("seek failure");

  uint8_t * data = (uint8_t *) malloc(*len);
  if (!data)
    die("malloc failure");
  if (fread(data, 1, *len, inFile) != *len)
    die("file read failure");
  fclose(inFile);

  return data;
}

int
main(int argc, char *argv[])
{
  const char * progName = argv[0];
  uint32_t status = eWOFF_ok;

  int opt;
  int option = 0;
  while ((opt = getopt(argc, argv, "vmph")) != -1) {
    switch (opt) {
    case 'v':
      if (option)
        fprintf(stderr, "# ignoring option '%c', already got '%c'\n", opt, option);
      else
        option = 'v';
      break;
    case 'm':
      if (option)
        fprintf(stderr, "# ignoring option '%c', already got '%c'\n", opt, option);
      else
        option = 'm';
      break;
    case 'p':
      if (option)
        fprintf(stderr, "# ignoring option '%c', already got '%c'\n", opt, option);
      else
        option = 'p';
      break;
    case 'h':
    case '?':
      usage(progName);
      exit(0);
    default:
      fprintf(stderr, "# unknown option '%c'\n", opt);
      break;
    }
  }
  argc -= optind;
  argv += optind;

  if (argc != 1) {
    usage(progName);
    exit(1);
  }

  uint32_t woffLen;
  const uint8_t * woffData = readFile(argv[0], &woffLen);

  uint32_t len;
  const uint8_t * data;

  data = woffGetMetadata(woffData, woffLen, &len, &status);

  if (WOFF_FAILURE(status)) {
    reportErr(status);
  }

  if (data) {
    if (fwrite(data, 1, len, stdout) != len) {
      die("error writing metadata to output");
      free((void *) data);
    } else {
      printf("<!-- No WOFF metadata available. -->\n");
    }
  }

  if (WOFF_WARNING(status)) {
    woffPrintStatus(stderr, status, "### ");
  }

  free((void *) woffData);

  return 0;
}
