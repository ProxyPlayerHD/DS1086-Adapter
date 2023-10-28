#include <Wire.h>



enum{
	REG_PRSC = 0x02,
	REG_DACH = 0x08,
	REG_DACL = 0x09,
	REG_OFFS = 0x0E,
	REG_ADDR = 0x0D,
	REG_RNGE = 0x37,
	REG_WREE = 0x3F
};

uint8_t readRegister(uint8_t adr){
	uint8_t tmp;
	Wire.beginTransmission(0x58);	// Start Transfer
	Wire.write(adr);				// Write the Register's Address
	Wire.endTransmission(false);	// Do not release the line
	Wire.requestFrom(0x58, 1);		// Request the value in the Register
	tmp = Wire.read();				// And Return the read Byte
	delay(10);
	// Serial.print("Read from Register: 0x");
	// Serial.print(adr, HEX);
	// Serial.print(" a Value of: 0x");
	// Serial.println(tmp, HEX);
	return tmp;
}

void writeRegister(uint8_t adr, uint8_t val){
	Wire.beginTransmission(0x58);	// Start Transfer  
	Wire.write(adr);				// Write the Register's Address
	Wire.write(val);				// Write Value to the specified Register
	Wire.endTransmission();			// And end the Transfer
	delay(10);
	// Serial.print("Write to Register: 0x");
	// Serial.print(adr, HEX);
	// Serial.print(" a Value of: 0x");
	// Serial.println(val, HEX);
}

#define readPrescaler()				(readRegister(REG_PRSC) & 0x0F)
#define readDither()				(!!(readRegister(REG_PRSC) & 0x10))
#define readDAC()					((((uint16_t)readRegister(REG_DACL) >> 6U) & 0x0003) | (((uint16_t)readRegister(REG_DACH) << 2U) & 0x03FC))
#define readOffset()				(readRegister(REG_OFFS) & 0x1F)
#define readAddress()				(readRegister(REG_ADDR) & 0x07)
#define readWC()					(!!(readRegister(REG_ADDR) & 0x08))
#define readRange()					(readRegister(REG_RNGE) & 0x1F)

#define writePrescaler(v, d)		writeRegister(REG_PRSC, (v & 0x0F) | (d ? 0x10 : 0x00))
#define writeDACH(v)				writeRegister(REG_DACH, (v >> 2U) & 0x00FF)
#define writeDACL(v)				writeRegister(REG_DACL, (v << 6U) & 0x00C0)
#define writeOffset(v)				writeRegister(REG_OFFS, v & 0x1F)
#define writeAddress(v)				writeRegister(REG_ADDR, (readRegister(REG_ADDR) & 0x08) | (v & 0x07))
#define writeWC(v)					writeRegister(REG_ADDR, (readRegister(REG_ADDR) & 0x07) | (v ? 0x08 : 0x00))
#define writeRange(v)				writeRegister(REG_RNGE, v & 0x1F)



typedef struct{
	const char *name;
	void (*func)(uint16_t);
} cmd_t;

uint16_t readLine();
bool executeLine(uint16_t len);
void cmdHelp(uint16_t idx);
void cmdGet(uint16_t idx);
void cmdSet(uint16_t idx);
void cmdWrite(uint16_t idx);

#define BUF_SIZE					(256U)
char inbuf[BUF_SIZE];				// Input Line Buffer

char debugStr[200];

#define CMD_COUNT					(4U)
const cmd_t commands[] = {
	{.name = "help", .func = cmdHelp},
	{.name = "get", .func = cmdGet},
	{.name = "set", .func = cmdSet},
	{.name = "write", .func = cmdWrite}
};


void setup(){
	Wire.begin(); 
	Serial.begin(9600);
	delay(1000);
	Serial.println("DS1086 Programmer v1\nType \"help\" for a list of commands");
	
}

void loop(){
	
	Serial.print(">");
	
	
	// Wait for a Line of Text from Serial
	uint16_t bufLen = 0;
	while(!bufLen){
		bufLen = readLine();
	}
	
	
	// Check if it is a valid command
	// If so call the correct function to execute that command
	if (!executeLine(bufLen)){
		// If not display an error and return to start
		Serial.println("Command not found");
	}
	
	
}




// Reads a line from Serial,
// Returns a non-zero value if it was successful
uint16_t readLine(){
	uint16_t i = 0;
	char tmp;
	while(1){
		while(!Serial.available()){};	// Wait for a character
		tmp = Serial.read();			// Then read it
		Serial.print(tmp);				// Also echo it back to the Terminal
		
		if (tmp == '\n') break;			// If it's a newline, the string is done
		if (tmp == '\b'){				// If it's a backspace, move the pointer back one character
			i--;
		}else{
			inbuf[i++] = tmp;			// If it wasn't a backspace, add the character to the buffer and increment the pointer
		}
		if (i >= BUF_SIZE) return 0;	// If the amount of characters exceeds the buffer size, return early
	}
	inbuf[i] = 0;
	return i;
}

// Grabs the first word (ie all character up to a whitespace character) in the inbuf,
// Converts it to all lowercase,
// And checks if it matches any internal commands,
// If it does it passes the rest of the inbuf to the function specific for that command,
// Returns false if no matching command was found
bool executeLine(uint16_t len){
	uint16_t i = 0;
	while(1){								// Split off the first word in the buffer
		if (inbuf[i] == ' ') break;			// If a whitespace character has been reached, exit the loop
		if (inbuf[i] == 0) break;			// Alternatively If a NULL terminator has been reached, exit the loop as well
		if (i >= BUF_SIZE) return false;	// Also check if the end of the buffer has been reached in case the NULL terminator is gone for some reason
		inbuf[i] = tolower(inbuf[i]);		// If nothing have been reached, convert the character to lowercase
		i++;								// And then go to the next one
	}
	if (inbuf[i]){
		inbuf[i++] = 0;						// If the end has been found and it's not NULL terminated there, add a NULL terminator and advance the pointer to the next part of the string
	}
	
	
	// Now iterate through all commands and check if any match
	uint8_t cmd = 0;
	for (cmd = 0; cmd < CMD_COUNT; cmd++){
		//snprintf(debugStr, 199, "[debug] comparing \"%s\" and \"%s\"", commands[cmd].name, inbuf);
		//Serial.println(debugStr);
		if (!strcmp(commands[cmd].name, inbuf)){
			commands[cmd].func(i);
			return true;
		}
	}
	
	// If this point is reached, no command was found and executed
	return false;
}


void cmdHelp(uint16_t idx){
	Serial.println("Commands:");
	Serial.println("  \"help\"                   - Displays this message");
	Serial.println("  \"get\"                    - Read and display the current clock Speed");
	Serial.println("  \"set <frequency in kHz>\" - Set the clock speed to the specified frequency (or the closest approximation)");
	Serial.println("  \"write\"                  - Saves the current clock speed to the DS1086's EEPROM\n");
}

void cmdGet(uint16_t idx){
	// First set the WC bit to prevent automatic EE writes (even though this fuction does no writes)
	writeWC(true);
	
	uint16_t exponent = readPrescaler();						// Get the Exponent (values between 0-8)
	uint16_t offsetIndex = (readOffset() - readRange()) + 6;	// Get the offset index (values between 0-12)
	uint16_t DAC = readDAC();									// Get the DAC (values between 0-1023)
	
	if (exponent > 8) exponent = 8;
	
	uint32_t minOffsetFreq = (61440UL + (offsetIndex * 5120UL));
	
	float frequency = (((float)minOffsetFreq + (float)DAC * 10.0f) / (float)(1UL << exponent)) / 1000.0f;
	
	
	snprintf(debugStr, 199, "[debug]\n  Offset: %d (Index: %u)\n  Exponent: %u\n  DAC Value: %u (%04X)\n  Min Offset Frequency: %lu", readOffset(), offsetIndex, exponent, DAC, DAC, minOffsetFreq);
	Serial.println(debugStr);
	
	
	Serial.print("Current Frequency is: ");
	Serial.print(frequency, 2);
	Serial.println("MHz");
	
}

void cmdSet(uint16_t idx){
	if (inbuf[idx] == 0){		// Check if the String is empty
		Serial.println("No frequency given");
		return;					// If it is print an error and return
	}
	
	// First set the WC bit to prevent automatic EE writes
	writeWC(true);
	
	// Then get the specified target frequency
	uint32_t targetFreq = atol(&inbuf[idx]);
	
	// Check if it's within the bounds of the chip's capabilities (plus some buffer space)
	if ((targetFreq < 300UL) || (targetFreq >= 133000UL)){
		Serial.println("Target frequency out of range (300kHz - 133000kHz)");
		return;					// If it isn't, print an error and return
	}
	
	// Calculate the Prescaler value
	uint8_t exponent = 0;
	for (; exponent < 10; exponent++){
		// Check exponents until one is found that gets the target frequency into the 66-133MHz range
		if (((1U << exponent) * targetFreq) >= 66000UL) break;
	}
	
	if (exponent > 8){		// Return early if the exponent is out of range
		Serial.print("[debug] Exponent out of range");
		return;
	}
	
	// Next Calculate the Offset
	uint8_t offsetIndex = 0;
	uint32_t masterFreq = (1UL << exponent) * targetFreq;
	uint32_t minOffsetFreq;
	for (; offsetIndex < 13; offsetIndex++){
		minOffsetFreq = (61440UL + (offsetIndex * 5120UL));
		//snprintf(debugStr, 199, "[debug] checking if %lu is within range of %lu and %lu", masterFreq, minOffsetFreq, (minOffsetFreq + 10230UL));
		//Serial.println(debugStr);
		
		// if the required master frequency is within range of the current offset, exit the loop
		if ((masterFreq <= (minOffsetFreq + 10230UL)) && (masterFreq >= minOffsetFreq)) break;
	}
	if (offsetIndex > 12){	// Return early if the offset is out of range
		Serial.print("[debug] Offset Index out of range");
		return;
	}
	
	// Now check if the master frequency is closer to the current or next offset
	// To do this simply subtract the master frequency by the center frequency of the current offset and the next offset and see which delta is smaller
	// Also check if offsetIndex isn't 12 (ie OS+6) in which case it cannot be incremented anymore
	if (((masterFreq - (minOffsetFreq + 5115UL) > (masterFreq - (minOffsetFreq + 5115UL + 5120UL)))) && (offsetIndex != 12)){
		offsetIndex++;
	}
	
	uint8_t defaultRange = readRange();
	int8_t offset = (defaultRange - 6) + offsetIndex;
	
	uint16_t DAC = (masterFreq - (61440UL + (offsetIndex * 5120UL))) / 10;
	
	snprintf(debugStr, 199, "[debug]\n  Offset: %d (Index: %u)\n  Exponent: %u\n  DAC Value: %u (%04X)", offset, offsetIndex, exponent & 0x0F, DAC, DAC);
	Serial.println(debugStr);
	
	writePrescaler(exponent, true);		// Prescaler (and enable 4% Dither)
	writeOffset(offset);				// Offset
	writeDACH(DAC);
	writeDACL(DAC);						// DAC
	
	Serial.println("Frequency Successfully changed");
}

void cmdWrite(uint16_t idx){
	Wire.beginTransmission(0x58);
	Wire.write(REG_WREE);
	Wire.endTransmission();
}












