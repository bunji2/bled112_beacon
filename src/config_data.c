#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef signed char    int8;

#include "config_data.h"

#ifdef LOADINI_MAIN
config_data_t cf;
#else
extern config_data_t cf;
#endif

void config_data_set(const char * key, const char *value) {
  int i;
  if (!strcmp("uuid", key)) {
    if (strlen(value) >= 16) {
      for (i=0; i<16; i++) {
        if (sscanf(value+i*2, "%02hx", cf.uuid+i) != 1) {
          break;
        }
      }
    }
  } else if (!strcmp("major", key)) {
    if (sscanf(value, "0x%04hx", &cf.major) != 1) {
      sscanf(value, "%d", &cf.major);
    }
//printf("major:%d\n", cf.major);
  } else if (!strcmp("minor", key)) {
    if (sscanf(value, "0x%04hx", &cf.minor) != 1) {
      sscanf(value, "%d", &cf.minor);
    }
  } else if (!strcmp("txpower", key)) {
    sscanf(value, "%d", &cf.txpower);
  } else if (!strcmp("adv_interval_min", key)) {
    if (sscanf(value, "0x%04hx", &cf.adv_interval_min) != 1) {
      sscanf(value, "%d", &cf.adv_interval_min);
    }
  } else if (!strcmp("adv_interval_max", key)) {
    if (sscanf(value, "0x%04hx", &cf.adv_interval_max) != 1) {
      sscanf(value, "%d", &cf.adv_interval_max);
    }
  } else if (!strcmp("adv_channels", key)) {
    if (sscanf(value, "0x%02hx", &cf.adv_channels) != 1) {
      sscanf(value, "%d", &cf.adv_channels);
    }
  } else {
    // ignore
  }
}

void config_data_print() {
  int i;
  printf("uuid=");
  for(i=0; i<16; i++) {
    printf("%02hx", cf.uuid[i]);
  }
  printf("\nmajor=0x%04hx\n", cf.major);
  printf("minor=0x%04hx\n", cf.minor);
  printf("txpower=%d\n", cf.txpower);
  printf("adv_interval_min=%d\n", cf.adv_interval_min);
  printf("adv_interval_max=%d\n", cf.adv_interval_max);
  printf("adv_channels=0x%02hx\n", cf.adv_channels);
}

void snip_CRLF(char * s) {
  while (*s) {
    switch (*s) {
    case '\r':
    case '\n':
      *s = 0;
      return;
    }
    s++;
  }
}

int config_data_load (const char *filename) {
  FILE *fp = NULL;
  char buf[81] = { 0x0 };
  char *p = NULL;
  if (NULL == (fp = fopen(filename, "r"))) {
    printf ("open error: %s\n", filename);
    return 0;
  }

  memset(&cf, 0, sizeof(config_data_t));

  while (NULL != fgets(buf, 81, fp)) {
    if (buf[0] != '#') {
      snip_CRLF(buf);
//      printf("[%s]\n", buf);
      if (NULL != (p = strchr(buf, '='))) {
        *p = 0;
//        printf("[%s] = [%s]\n", buf, p+1);
        config_data_set(buf, p+1);
      }
    }
  }
  
  fclose(fp);
  return 1;
}

#ifdef LOADINI_MAIN
int main (int ac, char **av) {

  if (ac < 2) {
    fprintf(stderr, "usage: %s config_file\n", av[0]);
    return -1;
  }
  
  config_data_load(av[1]);
  config_data_print();
  return 0;
}
#endif
