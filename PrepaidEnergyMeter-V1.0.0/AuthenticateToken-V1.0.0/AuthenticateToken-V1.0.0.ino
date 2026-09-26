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
// THIS METER'S ID
//
// Meter ID is NOT stored inside the token.
// It is reconstructed here when calculating CMAC.
// ============================================================

const uint8_t meterID = 15;


// ============================================================
// PRINT uint64_t
// ============================================================

void printUint64(uint64_t value)
{
    if (value == 0)
    {
        Serial.println("0");
        return;
    }

    char buf[21];

    buf[20] = '\0';

    int8_t i = 19;

    while (value > 0)
    {
        buf[i--] = '0' + (value % 10);
        value /= 10;
    }

    Serial.println(&buf[i + 1]);
}


// ============================================================
// CONVERT DECIMAL STRING TO uint64_t
//
// This avoids Arduino String and checks overflow.
// ============================================================

bool stringToUint64(const char *str, uint64_t *result)
{
    uint64_t value = 0;

    if (str == NULL || result == NULL)
        return false;

    if (*str == '\0')
        return false;

    while (*str)
    {
        // Must be a decimal digit

        if (*str < '0' || *str > '9')
            return false;

        uint8_t digit = *str - '0';

        // Overflow check:
        //
        // value * 10 + digit <= UINT64_MAX

        if (value > (UINT64_MAX - digit) / 10)
            return false;

        value = value * 10 + digit;

        str++;
    }

    *result = value;

    return true;
}


// ============================================================
// CREATE AUTHENTICATED MESSAGE
//
// 32-bit message:
//
// bits 31-28 : Meter ID       4 bits
// bits 27-12 : Serial        16 bits
// bits 11-2  : Recharge      10 bits
// bits 1-0   : 00
// ============================================================

uint32_t makeMessage(uint8_t meterID,
                     uint16_t serial,
                     uint16_t recharge)
{
    uint32_t message = 0;

    // Meter ID

    message |=
        ((uint32_t)(meterID & 0x0F) << 28);

    // Serial

    message |=
        ((uint32_t)serial << 12);

    // Recharge

    message |=
        ((uint32_t)(recharge & 0x03FF) << 2);

    // Bits 1-0 remain 00

    return message;
}


// ============================================================
// CALCULATE 38-BIT CMAC
// ============================================================

uint64_t calculateCMAC38(uint32_t message)
{
    uint8_t mac[16];

    AESTiny128 aes128;
    AES_CMAC cmac(aes128);

    // Convert 32-bit message to big endian

    uint8_t data_raw[4] = {
        (uint8_t)(message >> 24),
        (uint8_t)(message >> 16),
        (uint8_t)(message >> 8),
        (uint8_t)message
    };

    // Generate AES-CMAC

    cmac.generateMAC(
        mac,
        key,
        data_raw,
        sizeof(data_raw)
    );

    // --------------------------------------------------------
    // Extract first 38 bits
    //
    // mac[0] = 8 bits
    // mac[1] = 8 bits
    // mac[2] = 8 bits
    // mac[3] = 8 bits
    // mac[4] upper 6 bits
    //
    // Total = 38 bits
    // --------------------------------------------------------

    uint64_t mac38 = 0;

    mac38 |= ((uint64_t)mac[0] << 30);
    mac38 |= ((uint64_t)mac[1] << 22);
    mac38 |= ((uint64_t)mac[2] << 14);
    mac38 |= ((uint64_t)mac[3] << 6);
    mac38 |= ((uint64_t)(mac[4] >> 2));

    return mac38;
}


// ============================================================
// EXTRACT SERIAL FROM TOKEN
//
// Token:
//
// bits 63-38 = payload
// bits 37-0  = CMAC
//
// Payload:
//
// bits 25-10 = Serial
// bits 9-0   = Recharge
// ============================================================

uint16_t getSerial(uint64_t token)
{
    uint32_t payload =
        (uint32_t)(token >> 38);

    uint16_t serial =
        (uint16_t)(payload >> 10);

    return serial;
}


// ============================================================
// EXTRACT RECHARGE FROM TOKEN
// ============================================================

uint16_t getRecharge(uint64_t token)
{
    uint32_t payload =
        (uint32_t)(token >> 38);

    uint16_t recharge =
        payload & 0x03FF;

    return recharge;
}


// ============================================================
// EXTRACT CMAC FROM TOKEN
// ============================================================

uint64_t getTokenCMAC(uint64_t token)
{
    // Lower 38 bits

    uint64_t mask =
        0x3FFFFFFFFFULL;

    return token & mask;
}


// ============================================================
// VERIFY TOKEN
//
// Returns:
//
// true  = valid
// false = invalid
// ============================================================

bool verifyToken(uint64_t token,
                 uint16_t *serial,
                 uint16_t *recharge)
{
    // --------------------------------------------------------
    // Extract payload
    // --------------------------------------------------------

    uint32_t payload =
        (uint32_t)(token >> 38);

    // --------------------------------------------------------
    // Extract serial
    // --------------------------------------------------------

    uint16_t extractedSerial =
        (uint16_t)(payload >> 10);

    // --------------------------------------------------------
    // Extract recharge
    // --------------------------------------------------------

    uint16_t extractedRecharge =
        payload & 0x03FF;

    // --------------------------------------------------------
    // Extract received CMAC
    // --------------------------------------------------------

    uint64_t receivedCMAC =
        getTokenCMAC(token);

    // --------------------------------------------------------
    // Reconstruct original authenticated message
    //
    // IMPORTANT:
    //
    // Meter ID comes from THIS meter.
    //
    // It is not taken from the token.
    // --------------------------------------------------------

    uint32_t message =
        makeMessage(
            meterID,
            extractedSerial,
            extractedRecharge
        );

    // --------------------------------------------------------
    // Calculate CMAC
    // --------------------------------------------------------

    uint64_t calculatedCMAC =
        calculateCMAC38(message);

    // Return extracted values

    if (serial != NULL)
        *serial = extractedSerial;

    if (recharge != NULL)
        *recharge = extractedRecharge;

    // --------------------------------------------------------
    // Compare 38-bit CMAC
    // --------------------------------------------------------

    if (receivedCMAC == calculatedCMAC)
    {
        return true;
    }

    return false;
}


// ============================================================
// DISPLAY TOKEN INFORMATION
// ============================================================

void processToken(uint64_t token)
{
    uint16_t serial;
    uint16_t recharge;

    Serial.println();
    Serial.println("--------------------------------");
    Serial.println("TOKEN RECEIVED");
    Serial.println("--------------------------------");

    Serial.print("Token: ");
    printUint64(token);

    // --------------------------------------------------------
    // Verify
    // --------------------------------------------------------

    bool valid =
        verifyToken(
            token,
            &serial,
            &recharge
        );

    // --------------------------------------------------------
    // Display extracted values
    // --------------------------------------------------------

    Serial.print("Meter ID: ");
    Serial.println(meterID);

    Serial.print("Serial: ");
    Serial.println(serial);

    Serial.print("Recharge: ");
    Serial.println(recharge);

    // --------------------------------------------------------
    // Display result
    // --------------------------------------------------------

    if (valid)
    {
        Serial.println();
        Serial.println("TOKEN VALID");

        // ----------------------------------------------------
        // Reconstruct message
        // ----------------------------------------------------

        uint32_t message =
            makeMessage(
                meterID,
                serial,
                recharge
            );

        Serial.print("Message: 0x");

        if (message < 0x10000000UL)
            Serial.print("0");

        if (message < 0x01000000UL)
            Serial.print("0");

        if (message < 0x00100000UL)
            Serial.print("0");

        if (message < 0x00010000UL)
            Serial.print("0");

        if (message < 0x00001000UL)
            Serial.print("0");

        if (message < 0x00000100UL)
            Serial.print("0");

        if (message < 0x00000010UL)
            Serial.print("0");

        Serial.println(message, HEX);

        // ----------------------------------------------------
        // Calculate CMAC again
        // ----------------------------------------------------

        uint64_t calculatedCMAC =
            calculateCMAC38(message);

        Serial.print("Calculated CMAC-38: ");

        printUint64(calculatedCMAC);

        Serial.println();
        Serial.println("RECHARGE ACCEPTED");
    }
    else
    {
        Serial.println();
        Serial.println("TOKEN INVALID");
        Serial.println("CMAC MISMATCH");
    }

    Serial.println("--------------------------------");
}


// ============================================================
// SERIAL INPUT BUFFER
// ============================================================

#define TOKEN_BUFFER_SIZE 21

char tokenBuffer[TOKEN_BUFFER_SIZE];

uint8_t tokenIndex = 0;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(9600);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println(" PREPAID ENERGY METER");
    Serial.println(" TOKEN VERIFICATION TEST");
    Serial.println("================================");

    Serial.print("Meter ID: ");
    Serial.println(meterID);

    Serial.println();
    Serial.println("Enter 64-bit decimal token:");
    Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    while (Serial.available())
    {
        char c = Serial.read();

        // ----------------------------------------------------
        // ENTER
        // ----------------------------------------------------

        if (c == '\r' || c == '\n')
        {
            if (tokenIndex == 0)
                continue;

            tokenBuffer[tokenIndex] = '\0';

            uint64_t token;

            // ------------------------------------------------
            // Convert decimal token
            // ------------------------------------------------

            if (stringToUint64(
                    tokenBuffer,
                    &token))
            {
                processToken(token);
            }
            else
            {
                Serial.println();
                Serial.println("INVALID DECIMAL TOKEN");
                Serial.println("Overflow or non-numeric character.");
            }

            // Reset buffer

            tokenIndex = 0;

            Serial.println();
            Serial.println("Enter next token:");

            continue;
        }

        // ----------------------------------------------------
        // BACKSPACE
        // ----------------------------------------------------

        if (c == '\b')
        {
            if (tokenIndex > 0)
            {
                tokenIndex--;

                Serial.print("\b \b");
            }

            continue;
        }

        // ----------------------------------------------------
        // Accept only digits
        // ----------------------------------------------------

        if (c >= '0' && c <= '9')
        {
            if (tokenIndex < TOKEN_BUFFER_SIZE - 1)
            {
                tokenBuffer[tokenIndex++] = c;

                Serial.print(c);
            }
            else
            {
                Serial.println();
                Serial.println("TOKEN TOO LONG");

                tokenIndex = 0;
            }
        }
    }
}
