#include "rsaref.h"
#include "md5.h"
#include <string.h>

void
gen_md5(char *content, const char *str)
{
  MD5_CTX context;
  char digest[16];
  int i;
  int j;

  MD5Init(&context);
  MD5Update(&context, (unsigned char *)str, strlen(str));
  MD5Final((unsigned char *)digest, &context);

  // hashhex conversion
  for(i = 0; i < 16; i++) {
    j = (digest[i] >> 4) & 0xf;
    if(j <= 9)
      content[i * 2] = (j + '0');
    else
      content[i * 2] = (j + 'a' - 10);
    j = digest[i] & 0xf;
    if(j <= 9)
      content[i * 2 + 1] = (j + '0');
    else
      content[i * 2 + 1] = (j + 'a' - 10);
  }
  content[32] = 0;
}
