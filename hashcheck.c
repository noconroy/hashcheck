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

#include <getopt.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN SHA256_DIGEST_LENGTH
#define DEFAULT_ITERATIONS 4194304 /* 1024*1024*4 */
#define DEFAULT_OFFSET 1

#ifndef VERSION
#define VERSION "undefined"
#endif

#include "util.h"

static void version() {
  fprintf(stderr, "version: %s\n", VERSION);
  exit(EXIT_SUCCESS);
}

/**
 * Prints out help for program
 *
 * Does not return
 */
static void help() {
  extern char *__progname;

  fprintf(stderr, "hashcheck precommitment verification: %s\n", VERSION);
  fprintf(stderr, "Usage: %s [-h] [-V] [-c] [-v] [-p] [-a public_key] "
                  "[-b private_key] [-i iterations] [-o offset]\n",
          __progname);
  fprintf(stderr, "Choose from the following modes:\n");
  fprintf(stderr, "  -V print version info\n");
  fprintf(stderr, "  -c create keypair\n");
  fprintf(stderr, "  -v verify\n");
  fprintf(stderr, "  -p prove you own a certain hash\n");

  fprintf(stderr, "Other options:\n");
  fprintf(stderr, "  -a hexadecimal representation of public key\n");
  fprintf(stderr, "  -b hexadecimal representation of private key\n");
  fprintf(stderr, "  -i global number of iterations (default: %d)\n",
          DEFAULT_ITERATIONS);
  fprintf(stderr, "  -o offset for verification (default: 1)\n");

  exit(EXIT_SUCCESS);
}

/**
 * Hashes *private until either `iter` iterations have completed or it has
 * hashed to *public. If `public` is NULL, it will be ignored.
 *
 * Returns 0 on failure or number of iterations on success
 *
 */

size_t hash_until(unsigned char *private, unsigned char *public, size_t iter) {
  size_t i = 0;
  while (i++ < iter) {
    SHA256(private, BUFLEN, private);
    if (public && !memcmp(private, public, BUFLEN))
      return i;
  }
  return 0;
}

/**
 * Generates a pair of keys that where hashing Private `iter` times will  result
 * in public.
 *
 * No return value
 */
void generate_pair(size_t iter, unsigned char *private_key_b) {
  unsigned char buf[BUFLEN];
  fprintf(stderr, "Generating pair with %lu iterations\n", iter);

  /* generate pair from specific private key */
  if (private_key_b) {
    memcpy(buf, private_key_b, BUFLEN);
  } else { /* generate pair from randomness */
    randombytes(buf, BUFLEN);
  }
  printf("Private:\n\t");
  print_bytes(buf, BUFLEN);

  hash_until(buf, NULL, iter);

  printf("Public:\n\t");
  print_bytes(buf, BUFLEN);
}

/**
 * Verifies a pair of keys is valid
 *
 * Returns 0 on failure, number of iterations on success
 */

size_t verify_pair(unsigned char *private, unsigned char *public, size_t iter) {

  printf("Verifying: (public)\n\t");
  print_bytes(public, BUFLEN);
  printf("With: (private)\n\t");
  print_bytes(private, BUFLEN);

  /* set default value to iter */
  if (!iter)
    iter = 1;

  return hash_until(private, public, iter);
}

/**
 * Main function parses args passed to it. Then parses hex strings and runs each
 * mode.
 *
 * Return status code
 */

int main(int argc, char *argv[]) {
  char choice;
  char *public_key = NULL;
  char *private_key = NULL;
  unsigned char public_key_b[BUFLEN];
  unsigned char private_key_b[BUFLEN];

  int mode_create = 0;
  int mode_verify = 0;
  int mode_prove = 0;
  int iterations = DEFAULT_ITERATIONS;
  int offset = DEFAULT_OFFSET;

  while ((choice = getopt(argc, argv, "Vhcvpi:a:b:o:")) != -1) {
    switch (choice) {
    case 'c':
      mode_create = 1;
      break;
    case 'v':
      mode_verify = 1;
      break;
    case 'p':
      mode_prove = 1;
      break;
    case 'i':
      iterations = atoi(optarg);
      break;
    case 'o':
      offset = atoi(optarg);
      break;
    case 'a':
      public_key = optarg;
      break;
    case 'b':
      private_key = optarg;
      break;
    case 'V':
      version();
      /* NOTREACHED */
      break;
    case 'h':
      help();
      /* NOTREACHED */
      break;
    default:
      help();
      /* NOTREACHED */
    }
  }

  if (mode_create) {
    if (private_key) {
      load_bytes(private_key, private_key_b, BUFLEN,
                 "Please enter your private key: ");
      generate_pair(iterations, private_key_b);
    } else {
      generate_pair(iterations, NULL);
    }

    exit(EXIT_SUCCESS);
  }

  if (mode_prove) {
    size_t current_iterations;
    int verification_iterations;
    unsigned char private_tmp[BUFLEN];

    load_bytes(private_key, private_key_b, BUFLEN,
               "Please enter your private key: ");
    load_bytes(public_key, public_key_b, BUFLEN,
               "Please enter known public key: ");

    memcpy(private_tmp, private_key_b, BUFLEN);

    current_iterations = hash_until(private_tmp, public_key_b, iterations);
    verification_iterations = current_iterations - offset;

    if (verification_iterations <= 0 ||
        verification_iterations >= current_iterations) {
      fprintf(stderr, "Invalid offset: %d. Can't verify\n",
              verification_iterations);
      exit(EXIT_FAILURE);
    }

    memcpy(private_tmp, private_key_b, BUFLEN);
    hash_until(private_tmp, NULL, verification_iterations);

    if (verify_pair(private_tmp, public_key_b, 1)) {
      printf("Success!\n");
      exit(EXIT_SUCCESS);
    } else {
      printf("Fail.\n");
      exit(EXIT_FAILURE);
    }
  }

  if (mode_verify) {
    size_t result = 0;

    load_bytes(public_key, public_key_b, BUFLEN,
               "Please enter known public key: ");
    load_bytes(private_key, private_key_b, BUFLEN,
               "Please enter verification key: ");

    result = verify_pair(private_key_b, public_key_b, iterations);

    if (result) {
      printf("Success, %lu iterations!\n", result);
      exit(EXIT_SUCCESS);
    } else {
      printf("Could not verify pair after %d iterations\n", iterations);
      exit(EXIT_FAILURE);
    }
  }

  help();

  return 0;
}
