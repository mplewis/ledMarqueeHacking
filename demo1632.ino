/*
 * demo16x24.c - Arduino demo program for Holtek HT1632 LED driver chip,
 *            As implemented on the Sure Electronics DE-DP016 display board
 *            (16*24 dot matrix LED module.)
 * Nov, 2008 by Bill Westfield
 */

#include "ht1632.h"

/*
 * Set these constants to the values of the pins connected to the SureElectronics Module
 */
static const byte ht1632_data = 10;  // Data pin (pin 7)
static const byte ht1632_wrclk = 11; // Write clock pin (pin 5)
static const byte ht1632_cs = 12;    // Chip Select (1, 2, 3, or 4)
                                     // The should also be a common GND.
                                     // The module with all LEDs like draws about 200mA,
                                     //  which makes it PROBABLY powerable via Arduino +5V
/*
 * ht1632_writebits
 * Write bits (up to 8) to h1632 on pins ht1632_data, ht1632_wrclk
 * Chip is assumed to already be chip-selected
 * Bits are shifted out from MSB to LSB, with the first bit sent
 * being (bits & firstbit), shifted till firsbit is zero.
 */
void ht1632_chipselect(byte chipno)
{
  Serial.print("\nHT1632(");
  Serial.print(chipno);
  Serial.print(") ");
  digitalWrite(chipno, 0);
}

void ht1632_chipfree(byte chipno)
{
  digitalWrite(ht1632_wrclk, LOW);
  Serial.print(" [done ");
  Serial.print(chipno);
  Serial.print("]");
  digitalWrite(chipno, 1);
}

void ht1632_writebits (byte bits, byte firstbit)
{
  Serial.print(" ");
  while (firstbit) {
    Serial.print((bits&firstbit ? "1" : "0"));
    digitalWrite(ht1632_wrclk, LOW);
    if (bits & firstbit) {
      digitalWrite(ht1632_data, HIGH);
    } 
    else {
      digitalWrite(ht1632_data, LOW);
    }
    digitalWrite(ht1632_wrclk, HIGH);
    firstbit >>= 1;
  }
}

static void ht1632_sendcmd (byte command)
{
  ht1632_chipselect(ht1632_cs);  // Select chip
  ht1632_writebits(HT1632_ID_CMD, 1<<2);  // send 3 bits of id: COMMMAND
  ht1632_writebits(command, 1<<7);  // send the actual command
  ht1632_writebits(0, 1);       /* one extra dont-care bit in commands. */
  ht1632_chipfree(ht1632_cs); //done
}

static void ht1632_senddata (byte address, byte data)
{
  ht1632_chipselect(ht1632_cs);  // Select chip
  ht1632_writebits(HT1632_ID_WR, 1<<2);  // send ID: WRITE to RAM
  ht1632_writebits(address, 1<<6); // Send address
  ht1632_writebits(data, 1<<3); // send 4 bits of data
  ht1632_chipfree(ht1632_cs); // done
}

void setup ()  // flow chart from page 17 of datasheet
{
  pinMode(ht1632_cs, OUTPUT);
  digitalWrite(ht1632_cs, HIGH);       /* unselect (active low) */
  pinMode(ht1632_wrclk, OUTPUT);
  pinMode(ht1632_data, OUTPUT);
  ht1632_sendcmd(HT1632_CMD_SYSDIS);  // Disable system
  ht1632_sendcmd(HT1632_CMD_COMS10);  // Set display size and drivers
  ht1632_sendcmd(HT1632_CMD_MSTMD);       /* Master Mode */
  ht1632_sendcmd(HT1632_CMD_SYSON);       /* System on */
  ht1632_sendcmd(HT1632_CMD_LEDON);       /* LEDs on */
  for (byte i=0; i<128; i++)
    ht1632_senddata(i, 0);  // clear the display!
  Serial.begin(115200);
  Serial.println("Ready to go!");
  delay(1000);
}

void loop ()
{
  byte bits;
  byte addr;
  for (addr=0; addr < 96; addr++) {        // Shift in ON bits
    for (bits=8; ; bits=(bits>>1)+8) {
      ht1632_senddata(addr, bits);
      delay(30);
      if (bits == 15) break;
    }
  }
  delay(1000);
  for (addr=0; addr < 96; addr++) {      // Now shift in OFF bits
    for (bits=15; ; bits=(bits>>1)) {
      ht1632_senddata(addr, bits);
      delay(10);
      if (bits == 0) break;
    }
  }
  delay(1000);
}