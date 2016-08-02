
int8_t MuLaw_Encode(int16_t number){
    const uint16_t MULAW_MAX = 0x1FFF;   // Maximum value 2^13 -1  = 8191
    const uint16_t MULAW_BIAS = 33;      // Bias value
    uint16_t mask = 0x1000;
    uint8_t mu_sign = 0;
    uint8_t position = 12;
    uint8_t lsb = 0;
    if(number < 0)
    {
        number = -number;
        mu_sign   = 0x80;   // 1000 0000
    } // Getting mu-sgn bit

    number += MULAW_BIAS;   // Get magnitude + BIAS value
    printf("bias integer %d\n", number);
    if(number > MULAW_MAX){
        number= MULAW_MAX;   // Ensure number does not exceed MAX value for 8 bit encoding
    }
    printf("number =  %d\n", number);


    for(; ((number & mask) != mask && position >= 5); mask >>= 1, position--  );
    lsb = (number >> (position -4)) & 0x0f;
    return (~(mu_sign | ((position) << 4) | lsb));
}


int16_t MuLaw_Decode(int8_t number)
{
   const uint16_t MULAW_BIAS = 33;
   uint8_t sign = 0, position = 0;
   int16_t decoded = 0;
   number = ~number;
   if (number & 0x80)
   {
      number &= ~(1 << 7);
      sign = -1;
   }
   position = ((number & 0xF0) >> 4) + 5;
   decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
             | (1 << (position - 5))) - MULAW_BIAS;
   return (sign == 0) ? (decoded) : (-(decoded));
}
