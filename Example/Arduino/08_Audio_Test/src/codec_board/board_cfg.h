#ifndef BOARD_CFG_H
#define BOARD_CFG_H


const char board_cfg_data[] = {
  "# support in, out, in_out type\n"
  "# support i2c_port, i2s_port settings\n"
  "# support pa_gain, i2c_addr setting\n"
  "\n"
  "Board: C6_AMOLED_1_43\n"
  "i2c: {sda: 18, scl: 8}\n"
  "i2s: {bclk: 21, ws: 22, dout: 23, din: 20, mclk: 19}\n"
  "out: {codec: ES8311, pa: -1, use_mclk: 1, pa_gain:6}\n"
  "in: {codec: ES7210}\n"
  "Board: S3_ePaper_1_54\n"
  "i2c: {sda: 47, scl: 48}\n"
  "i2s: {bclk: 15, ws: 38, dout: 45, din: 16, mclk: 14}\n"
  "in_out: {codec: ES8311, pa: 46, use_mclk: 1, pa_gain:6}\n"
};
const char *board_cfg_start = board_cfg_data;
const char *board_cfg_end = board_cfg_data + sizeof(board_cfg_data) - 1;  // -1去掉末尾\0

#endif