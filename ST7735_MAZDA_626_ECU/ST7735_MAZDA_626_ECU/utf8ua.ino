String utf8ua(String source) {
  int i = 0;
  int length = source.length();
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  while (i < length) {
    n = source[i++];
    
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
            n = source[i++];
            if (n == 0x81) {
              n = 0xA8;
            } else if (n >= 0x90 && n <= 0xBF) {
              n += 0x30;
            }
            break;
          }
        case 0xD1: {
            n = source[i++];
            if (n == 0x91) {
              n = 0xB8;
            } else if (n >= 0x80 && n <= 0x8F) {
              n += 0x70;
            }
            break;
          }
        case 0xC2: {
            n = source[i++];
            if (n != 0xB0) {
              n = 0xC2;
              i--;
            }
            break;
          }
      }
    }
    m[0] = n;
    target += String(m);
  }
  return target;
}
