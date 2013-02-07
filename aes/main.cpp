/*
*   Byte-oriented AES-256 Command Line Interface
*
*   Copyright (c) 2013-2013 smanders software @ smanders d0t com
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose with or without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies.
*
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "aes256.h"

#define AES256_CRYPTO_KEY_SIZE 32
#define PASSWORD_SIZE          16 
#define CHALLENGE_TOKEN_SIZE   16

int main(int argc, char *argv[]) {

  char currentChallengeToken[CHALLENGE_TOKEN_SIZE + 1];
  char submitPassword[AES256_CRYPTO_KEY_SIZE + 1];
  int c;
  int i;
  int j;
  uint8_t cryptoKey[AES256_CRYPTO_KEY_SIZE + 1];
  uint8_t password[PASSWORD_SIZE + 1];
  char hexValue[3];
  aes256_context ctx;
  char passwordAsChar[PASSWORD_SIZE + 1];

  if (argc != 5) {
    printf("Not Enough Parameters\n");
    return 0;
  }
     
  while ((c = getopt (argc, argv, "k:d:")) != -1) {
    switch (c) {
    case 'k':
      strncpy(currentChallengeToken,optarg,sizeof(currentChallengeToken));
      break;
    case 'd':
      strncpy(submitPassword,optarg,sizeof(submitPassword));
      break;
    default:
      abort();
    }
  }

  // the presence of an HTTP GET password param results in a request
  // to trigger the relay (used to be triggered by an HTTP request of type POST)

  if(strlen(submitPassword) > 0)
  {
    // decrypt password using latest challenge token as cypher key
    memset(&cryptoKey, 0, sizeof(cryptoKey));

    for (i = 0; i < strlen(currentChallengeToken); ++i)
      cryptoKey[i] = currentChallengeToken[i];

    memset(&password, 0, sizeof(password));

    // convert password from hex string to ascii decimal

    i = 0;
    j = 0;
    while (1)
    {
      if (!submitPassword[j])
        break;

      hexValue[0] = submitPassword[j];
      hexValue[1] = submitPassword[j+1];
      hexValue[2] = '\0';
      password[i] = (int) strtol(hexValue, NULL, 16);

      i += 1;
      j += 2;
    }

    // proceed with AES256 password decryption

    aes256_init(&ctx, cryptoKey);
    aes256_decrypt_ecb(&ctx, password);
    aes256_done(&ctx);

    memset(&passwordAsChar, 0, sizeof(passwordAsChar));

    for (i = 0; i < sizeof(password); ++i)
      passwordAsChar[i] = password[i];

    printf("%s\n",passwordAsChar);
  }
  return 0;
}
