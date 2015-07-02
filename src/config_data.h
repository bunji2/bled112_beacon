typedef struct {
  uint8  uuid [16];
  uint16 major;
  uint16 minor;
  int8   txpower;
  uint16 adv_interval_min;
  uint16 adv_interval_max;
  uint8  adv_channels;
} config_data_t;


int config_data_load (const char *filename);
void config_data_print ();
