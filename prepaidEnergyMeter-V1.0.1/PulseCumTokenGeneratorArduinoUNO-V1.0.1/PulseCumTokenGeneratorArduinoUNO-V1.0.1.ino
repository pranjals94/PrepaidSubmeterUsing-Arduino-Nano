#include <Crypto.h>
#include <AES.h>
#include <AES_CMAC.h>

// ============================================================
// AES-128 KEY
// ============================================================

const uint8_t key[16] = {
    0x2b, 0x7e, 0x15, 0x16,
    0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88,
    0x09, 0xcf, 0x4f, 0x3c
};


// ============================================================
// METER ID
//
// Meter ID is NOT transmitted in the token.
// It is included in the data used for CMAC.
// ============================================================

const uint8_t meterID = 15;


void setup_pulse_generator()
{
    // D9 = PB1 = OC1A
    DDRB |= (1 << PB1);
    // Timer1:
    // CTC mode
    // Toggle OC1A automatically on compare match
    TCCR1A = (1 << COM1A0);

    TCCR1B =
        (1 << WGM12) |   // CTC, TOP = OCR1A
        (1 << CS10);      // No prescaler

    // Approximately 65,535 pulses/minute
    OCR1A = 7323;
}

// ============================================================
// PRINT uint64_t AS DECIMAL
// ============================================================

void printUint64(uint64_t value)
{
    char buf[21];

    // Convert uint64_t to exactly 20 digits
    for (int8_t i = 19; i >= 0; i--)
    {
        buf[i] = '0' + (value % 10);
        value /= 10;
    }

    // Print with '-' after every 4 digits
    for (uint8_t i = 0; i < 20; i++)
    {
        Serial.print(buf[i]);

        if ((i % 4) == 3 && i != 19)
        {
            Serial.print('-');
        }
    }

    Serial.println();
}


// ============================================================
// CREATE CMAC MESSAGE
//
// 32-bit CMAC input:
//
// bit 31 - 28 : Meter ID       4 bits
// bit 27 - 12 : Serial        16 bits
// bit 11 -  2 : Recharge      10 bits
// bit  1 -  0 : Reserved       2 bits
//
// The complete 32-bit value is authenticated,
// but Meter ID is NOT included in the transmitted token.
// ============================================================

uint32_t makeMessage(uint8_t meterID,
                     uint16_t serial,
                     uint16_t recharge)
{
    uint32_t message = 0;

    // Meter ID: bits 31-28
    message |= ((uint32_t)(meterID & 0x0F) << 28);

    // Serial: bits 27-12
    message |= ((uint32_t)serial << 12);

    // Recharge: bits 11-2
    message |= ((uint32_t)(recharge & 0x03FF) << 2);

    // Bits 1-0 remain 00

    return message;
}


// ============================================================
// CREATE 38-BIT CMAC
//
// AES-CMAC produces 128 bits.
//
// We use the first 38 bits:
//
// mac[0]       = 8 bits
// mac[1]       = 8 bits
// mac[2]       = 8 bits
// mac[3]       = 8 bits
// mac[4] >> 2  = 6 bits
//
// Total = 38 bits
// ============================================================

uint64_t makeCMAC38(uint32_t message)
{
    uint8_t mac[16];

    AESTiny128 aes128;
    AES_CMAC cmac(aes128);

    // Convert 32-bit message to big-endian byte array

    uint8_t data_raw[4] = {
        (uint8_t)(message >> 24),
        (uint8_t)(message >> 16),
        (uint8_t)(message >> 8),
        (uint8_t)message
    };

    // Calculate AES-CMAC

    cmac.generateMAC(
        mac,
        key,
        data_raw,
        sizeof(data_raw)
    );

    // Extract first 38 bits

    uint64_t mac38 = 0;

    mac38 |= ((uint64_t)mac[0] << 30);
    mac38 |= ((uint64_t)mac[1] << 22);
    mac38 |= ((uint64_t)mac[2] << 14);
    mac38 |= ((uint64_t)mac[3] << 6);
    mac38 |= ((uint64_t)(mac[4] >> 2));

    return mac38;
}


// ============================================================
// CREATE TOKEN
//
// Token:
//
// bits 63 - 38 : Serial + Recharge = 26 bits
// bits 37 -  0 : CMAC             = 38 bits
//
// Payload:
//
// bits 25 - 10 : Serial           16 bits
// bits  9 -  0 : Recharge         10 bits
//
// Meter ID is NOT stored in token.
// ============================================================

uint64_t makeToken(uint8_t meterID,
                   uint16_t serial,
                   uint16_t recharge)
{
    // --------------------------------------------------------
    // Check limits
    // --------------------------------------------------------

    if (meterID > 15)
    {
        return 0;
    }

    if (recharge > 1023)
    {
        return 0;
    }

    // --------------------------------------------------------
    // Create authenticated message
    // --------------------------------------------------------

    uint32_t message =
        makeMessage(
            meterID,
            serial,
            recharge
        );

    // --------------------------------------------------------
    // Calculate 38-bit CMAC
    // --------------------------------------------------------

    uint64_t mac38 =
        makeCMAC38(message);

    // --------------------------------------------------------
    // Create transmitted 26-bit payload
    //
    // bits 25-10 = serial
    // bits 9-0   = recharge
    // --------------------------------------------------------

    uint32_t payload =
        ((uint32_t)serial << 10) |
        (recharge & 0x03FF);

    // --------------------------------------------------------
    // Combine:
    //
    // [26-bit payload][38-bit CMAC]
    // --------------------------------------------------------

    uint64_t token =
        ((uint64_t)payload << 38) |
        mac38;

    return token;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    delay(1000);
    setup_pulse_generator();
}


// ============================================================
// LOOP
// ============================================================

uint16_t serial   = 65500;
uint16_t recharge = 1023;

void loop()
{
      uint64_t token =
        makeToken(
            meterID,
            serial,
            recharge
        );

    Serial.println();
    Serial.println("================================");
    Serial.println("      PREPAID TOKEN");
    Serial.println("================================");

    Serial.print("Meter ID : ");
    Serial.println(meterID);

    Serial.print("Serial   : ");
    Serial.println(serial);

    Serial.print("Recharge : ");
    Serial.println(recharge);

    Serial.print("Token    : ");

    printUint64(token);

    Serial.println("================================");

    serial++;
    recharge++;

    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(500);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(500);                      // wait fo
}