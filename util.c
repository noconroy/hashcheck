/* Copyright (c) 2017 Matt Dunwoodie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

void read_random(unsigned char *buf, size_t bytes) {
  int r = open("/dev/urandom", O_RDONLY);
  size_t len = 0;
  while (len < bytes) {
    int a = read(r, buf + len, bytes - len);
    if (a < 0) {
      fprintf(stderr, "Could not read random bytes\n");
      exit(EXIT_FAILURE);
    }
    len += a;
  }
  close(r);
}

void print_bytes(unsigned char *b, size_t len) {
  while (len--) {
    printf("%02x", *(b++));
  }
  printf("\n");
}

static int hex_scan(char *str, unsigned char *buf, size_t n) {
  size_t i;
  char tmp[] = {'0', '0', '\0'};
  memset(buf, 0, n);
  for (i = 0; i < n; i++) {
    tmp[0] = str[2 * i];
    tmp[1] = str[2 * i + 1];
    if (tmp[0] == '\0' || tmp[1] == '\0')
      return 0;
    buf[i] = strtol(tmp, NULL, 16);
  }
  return n;
}

static int read_hex_input(unsigned char *buf, size_t n) {
  char *newline;

  char _input[INPUT_BUFLEN];
  char *input = _input;

  input++;

  fgets(input, INPUT_BUFLEN - 1, stdin);
  fflush(stdin);

  newline = strchr(input, '\n');

  if (newline)
    *newline = '\0';

  if (strlen(input) == 2 * n - 1) {
    input--;
    *input = '0';
  }

  if (strlen(input) == 2 * n) {
    hex_scan(input, buf, n);
    return n;
  } else {
    fprintf(stderr, "Please enter %lu bytes\n", n);
    return 0;
  }
}

void load_bytes(char *string_in, unsigned char *buf, size_t n, char *message) {
  if (!message)
    message = "Please enter %lu bytes\n";

  if (!string_in) {
    fprintf(stderr, message, n);
    if (read_hex_input(buf, n) != n) {
      fprintf(stderr, "Input must be %lu bytes long\n", n);
      exit(EXIT_FAILURE);
    }
  } else if (!hex_scan(string_in, buf, n)) {
    fprintf(stderr, "Input must be %lu bytes long\n", n);
    exit(EXIT_FAILURE);
  }
}
