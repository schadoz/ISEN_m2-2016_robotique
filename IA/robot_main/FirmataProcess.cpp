#include "FirmataProcess.h"

/* utility functions */
void FirmataProcess::wireWrite(byte data)
{
#if ARDUINO >= 100
	Wire.write((byte)data);
#else
	Wire.send(data);
#endif
}

byte FirmataProcess::wireRead()
{
#if ARDUINO >= 100
	return Wire.read();
#else
	return Wire.receive();
#endif
}

void FirmataProcess::readAndReportData(byte address, int theRegister, byte numBytes) {
	// allow I2C requests that don't require a register read
	// for example, some devices using an interrupt pin to signify new data available
	// do not always require the register read so upon interrupt you call Wire.requestFrom()
	if (theRegister != I2C_REGISTER_NOT_SPECIFIED) {
		Wire.beginTransmission(address);
		wireWrite((byte)theRegister);
		Wire.endTransmission();
		// do not set a value of 0

		if (i2cReadDelayTime > 0) {
			// delay is necessary for some devices such as WiiNunchuck
			delayMicroseconds(i2cReadDelayTime);
		}
	}
	else {
		theRegister = 0;  // fill the register with a dummy value
	}

	Wire.requestFrom(address, numBytes);  // all bytes are returned in requestFrom

										  // check to be sure correct number of bytes were returned by slave
	if (numBytes < Wire.available()) {
		Firmata.sendString("I2C: Too many bytes received");
	}
	else if (numBytes > Wire.available()) {
		Firmata.sendString("I2C: Too few bytes received");
	}

	i2cRxData[0] = address;
	i2cRxData[1] = theRegister;

	for (int i = 0; i < numBytes && Wire.available(); i++) {
		i2cRxData[2 + i] = wireRead();
	}

	// send slave address, register and received bytes
	Firmata.sendSysex(SYSEX_I2C_REPLY, numBytes + 2, i2cRxData);
}

void FirmataProcess::outputPort(byte portNumber, byte portValue, byte forceSend)
{
	// pins not configured as INPUT are cleared to zeros
	portValue = portValue & portConfigInputs[portNumber];
	// only send if the value is different than previously sent
	if (forceSend || previousPINs[portNumber] != portValue) {
		Firmata.sendDigitalPort(portNumber, portValue);
		previousPINs[portNumber] = portValue;
	}
}

void FirmataProcess::checkDigitalInputs(void)
{
	/* -----------------------------------------------------------------------------
	* check all the active digital inputs for change of state, then add any events
	* to the Serial output queue using Serial.print() */

	/* Using non-looping code allows constants to be given to readPort().
	* The compiler will apply substantial optimizations if the inputs
	* to readPort() are compile-time constants. */
	if (TOTAL_PORTS > 0 && reportPINs[0]) outputPort(0, readPort(0, portConfigInputs[0]), false);
	if (TOTAL_PORTS > 1 && reportPINs[1]) outputPort(1, readPort(1, portConfigInputs[1]), false);
	if (TOTAL_PORTS > 2 && reportPINs[2]) outputPort(2, readPort(2, portConfigInputs[2]), false);
	if (TOTAL_PORTS > 3 && reportPINs[3]) outputPort(3, readPort(3, portConfigInputs[3]), false);
	if (TOTAL_PORTS > 4 && reportPINs[4]) outputPort(4, readPort(4, portConfigInputs[4]), false);
	if (TOTAL_PORTS > 5 && reportPINs[5]) outputPort(5, readPort(5, portConfigInputs[5]), false);
	if (TOTAL_PORTS > 6 && reportPINs[6]) outputPort(6, readPort(6, portConfigInputs[6]), false);
	if (TOTAL_PORTS > 7 && reportPINs[7]) outputPort(7, readPort(7, portConfigInputs[7]), false);
	if (TOTAL_PORTS > 8 && reportPINs[8]) outputPort(8, readPort(8, portConfigInputs[8]), false);
	if (TOTAL_PORTS > 9 && reportPINs[9]) outputPort(9, readPort(9, portConfigInputs[9]), false);
	if (TOTAL_PORTS > 10 && reportPINs[10]) outputPort(10, readPort(10, portConfigInputs[10]), false);
	if (TOTAL_PORTS > 11 && reportPINs[11]) outputPort(11, readPort(11, portConfigInputs[11]), false);
	if (TOTAL_PORTS > 12 && reportPINs[12]) outputPort(12, readPort(12, portConfigInputs[12]), false);
	if (TOTAL_PORTS > 13 && reportPINs[13]) outputPort(13, readPort(13, portConfigInputs[13]), false);
	if (TOTAL_PORTS > 14 && reportPINs[14]) outputPort(14, readPort(14, portConfigInputs[14]), false);
	if (TOTAL_PORTS > 15 && reportPINs[15]) outputPort(15, readPort(15, portConfigInputs[15]), false);
}

void FirmataProcess::setPinModeCallback(byte pin, int mode)
{
	// -----------------------------------------------------------------------------
	/* sets the pin mode to the correct state and sets the relevant bits in the
	* two bit-arrays that track Digital I/O and PWM status
	*/

	if (pinConfig[pin] == IGNORE)
		return;

	if (pinConfig[pin] == I2C && isI2CEnabled && mode != I2C) {
		// disable i2c so pins can be used for other functions
		// the following if statements should reconfigure the pins properly
		disableI2CPins();
	}
	if (IS_PIN_ANALOG(pin)) {
		reportAnalogCallback(PIN_TO_ANALOG(pin), mode == ANALOG ? 1 : 0); // turn on/off reporting
	}
	if (IS_PIN_DIGITAL(pin)) {
		if (mode == INPUT) {
			portConfigInputs[pin / 8] |= (1 << (pin & 7));
		}
		else {
			portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
		}
	}
	pinState[pin] = 0;
	switch (mode) {
	case ANALOG:
		if (IS_PIN_ANALOG(pin)) {
			if (IS_PIN_DIGITAL(pin)) {
				pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
				digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
			}
			pinConfig[pin] = ANALOG;
		}
		break;
	case INPUT:
		if (IS_PIN_DIGITAL(pin)) {
			pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
			digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
			pinConfig[pin] = INPUT;
		}
		break;
	case OUTPUT:
		if (IS_PIN_DIGITAL(pin)) {
			digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable PWM
			pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
			pinConfig[pin] = OUTPUT;
		}
		break;
	case PWM:
		if (IS_PIN_PWM(pin)) {
			pinMode(PIN_TO_PWM(pin), OUTPUT);
			analogWrite(PIN_TO_PWM(pin), 0);
			pinConfig[pin] = PWM;
		}
		break;
	case I2C:
		if (IS_PIN_I2C(pin)) {
			// mark the pin as i2c
			// the user must call I2C_CONFIG to enable I2C for a device
			pinConfig[pin] = I2C;
		}
		break;
	default:
		Firmata.sendString("Unknown pin mode"); // TODO: put error msgs in EEPROM
	}
	// TODO: save status to EEPROM here, if changed
}

void FirmataProcess::analogWriteCallback(byte pin, int value)
{
	if (pin < TOTAL_PINS) {
		switch (pinConfig[pin]) {
		case PWM:
			if (IS_PIN_PWM(pin))
				analogWrite(PIN_TO_PWM(pin), value);
			pinState[pin] = value;
			break;
		}
	}
}

void FirmataProcess::digitalWriteCallback(byte port, int value)
{
	byte pin, lastPin, mask = 1, pinWriteMask = 0;

	if (port < TOTAL_PORTS) {
		// create a mask of the pins on this port that are writable.
		lastPin = port * 8 + 8;
		if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
		for (pin = port * 8; pin < lastPin; pin++) {
			// do not disturb non-digital pins (eg, Rx & Tx)
			if (IS_PIN_DIGITAL(pin)) {
				// only write to OUTPUT and INPUT (enables pullup)
				// do not touch pins in PWM, ANALOG, SERVO or other modes
				if (pinConfig[pin] == OUTPUT || pinConfig[pin] == INPUT) {
					pinWriteMask |= mask;
					pinState[pin] = ((byte)value & mask) ? 1 : 0;
				}
			}
			mask = mask << 1;
		}
		writePort(port, (byte)value, pinWriteMask);
	}
}

void FirmataProcess::reportAnalogCallback(byte analogPin, int value)
{
	if (analogPin < TOTAL_ANALOG_PINS) {
		if (value == 0) {
			analogInputsToReport = analogInputsToReport & ~(1 << analogPin);
		}
		else {
			analogInputsToReport = analogInputsToReport | (1 << analogPin);
			// prevent during system reset or all analog pin values will be reported
			// which may report noise for unconnected analog pins
			if (!isResetting) {
				// Send pin value immediately. This is helpful when connected via
				// ethernet, wi-fi or bluetooth so pin states can be known upon
				// reconnecting.
				Firmata.sendAnalog(analogPin, analogRead(analogPin));
			}
		}
	}
	// TODO: save status to EEPROM here, if changed
}

void FirmataProcess::reportDigitalCallback(byte port, int value)
{
	if (port < TOTAL_PORTS) {
		reportPINs[port] = (byte)value;
		// Send port value immediately. This is helpful when connected via
		// ethernet, wi-fi or bluetooth so pin states can be known upon
		// reconnecting.
		if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
	}
	// do not disable analog reporting on these 8 pins, to allow some
	// pins used for digital, others analog.  Instead, allow both types
	// of reporting to be enabled, but check if the pin is configured
	// as analog when sampling the analog inputs.  Likewise, while
	// scanning digital pins, portConfigInputs will mask off values from any
	// pins configured as analog
}

void FirmataProcess::enableI2CPins()
{
	byte i;
	// is there a faster way to do this? would probaby require importing
	// Arduino.h to get SCL and SDA pins
	for (i = 0; i < TOTAL_PINS; i++) {
		if (IS_PIN_I2C(i)) {
			// mark pins as i2c so they are ignore in non i2c data requests
			setPinModeCallback(i, I2C);
		}
	}

	isI2CEnabled = true;

	Wire.begin();
}

void FirmataProcess::disableI2CPins() 
{
	/* disable the i2c pins so they can be used for other functions */
	isI2CEnabled = false;
	// disable read continuous mode for all devices
	queryIndex = -1;
}

void FirmataProcess::systemResetCallback()
{
	isResetting = true;

	// initialize a defalt state
	// TODO: option to load config from EEPROM instead of default

	if (isI2CEnabled) {
		disableI2CPins();
	}

	for (byte i = 0; i < TOTAL_PORTS; i++) {
		reportPINs[i] = false;    // by default, reporting off
		portConfigInputs[i] = 0;  // until activated
		previousPINs[i] = 0;
	}

	for (byte i = 0; i < TOTAL_PINS; i++) {
		// pins with analog capability default to analog input
		// otherwise, pins default to digital output
		if (IS_PIN_ANALOG(i)) {
			// turns off pullup, configures everything
			setPinModeCallback(i, ANALOG);
		}
		else {
			// sets the output to 0, configures portConfigInputs
			setPinModeCallback(i, OUTPUT);
		}

	}
	// by default, do not report any analog inputs
	analogInputsToReport = 0;


	/* send digital inputs to set the initial state on the host computer,
	* since once in the loop(), this firmware will only send on change */
	/*
	TODO: this can never execute, since no pins default to digital input
	but it will be needed when/if we support EEPROM stored config
	for (byte i=0; i < TOTAL_PORTS; i++) {
	outputPort(i, readPort(i, portConfigInputs[i]), true);
	}
	*/
	isResetting = false;
}

/*==============================================================================
* SYSEX-BASED commands
*============================================================================*/

void FirmataProcess::sysexCallback(byte command, byte argc, byte *argv)
{
	byte mode;
	byte slaveAddress;
	byte data;
	int slaveRegister;
	unsigned int delayTime;

	switch (command) {
	case I2C_REQUEST:
		mode = argv[1] & I2C_READ_WRITE_MODE_MASK;
		if (argv[1] & I2C_10BIT_ADDRESS_MODE_MASK) {
			Firmata.sendString("10-bit addressing not supported");
			return;
		}
		else {
			slaveAddress = argv[0];
		}

		switch (mode) {
		case I2C_WRITE:
			Wire.beginTransmission(slaveAddress);
			for (byte i = 2; i < argc; i += 2) {
				data = argv[i] + (argv[i + 1] << 7);
				wireWrite(data);
			}
			Wire.endTransmission();
			delayMicroseconds(70);
			break;
		case I2C_READ:
			if (argc == 6) {
				// a slave register is specified
				slaveRegister = argv[2] + (argv[3] << 7);
				data = argv[4] + (argv[5] << 7);  // bytes to read
			}
			else {
				// a slave register is NOT specified
				slaveRegister = I2C_REGISTER_NOT_SPECIFIED;
				data = argv[2] + (argv[3] << 7);  // bytes to read
			}
			readAndReportData(slaveAddress, (int)slaveRegister, data);
			break;
		case I2C_READ_CONTINUOUSLY:
			if ((queryIndex + 1) >= I2C_MAX_QUERIES) {
				// too many queries, just ignore
				Firmata.sendString("too many queries");
				break;
			}
			if (argc == 6) {
				// a slave register is specified
				slaveRegister = argv[2] + (argv[3] << 7);
				data = argv[4] + (argv[5] << 7);  // bytes to read
			}
			else {
				// a slave register is NOT specified
				slaveRegister = (int)I2C_REGISTER_NOT_SPECIFIED;
				data = argv[2] + (argv[3] << 7);  // bytes to read
			}
			queryIndex++;
			query[queryIndex].addr = slaveAddress;
			query[queryIndex].reg = slaveRegister;
			query[queryIndex].bytes = data;
			break;
		case I2C_STOP_READING:
			byte queryIndexToSkip;
			// if read continuous mode is enabled for only 1 i2c device, disable
			// read continuous reporting for that device
			if (queryIndex <= 0) {
				queryIndex = -1;
			}
			else {
				// if read continuous mode is enabled for multiple devices,
				// determine which device to stop reading and remove it's data from
				// the array, shifiting other array data to fill the space
				for (byte i = 0; i < queryIndex + 1; i++) {
					if (query[i].addr == slaveAddress) {
						queryIndexToSkip = i;
						break;
					}
				}

				for (byte i = queryIndexToSkip; i < queryIndex + 1; i++) {
					if (i < I2C_MAX_QUERIES) {
						query[i].addr = query[i + 1].addr;
						query[i].reg = query[i + 1].reg;
						query[i].bytes = query[i + 1].bytes;
					}
				}
				queryIndex--;
			}
			break;
		default:
			break;
		}
		break;
	case I2C_CONFIG:
		delayTime = (argv[0] + (argv[1] << 7));

		if (delayTime > 0) {
			i2cReadDelayTime = delayTime;
		}

		if (!isI2CEnabled) {
			enableI2CPins();
		}

		break;
	case SAMPLING_INTERVAL:
		if (argc > 1) {
			samplingInterval = argv[0] + (argv[1] << 7);
			if (samplingInterval < MINIMUM_SAMPLING_INTERVAL) {
				samplingInterval = MINIMUM_SAMPLING_INTERVAL;
			}
		}
		else {
			//Firmata.sendString("Not enough data");
		}
		break;
	case EXTENDED_ANALOG:
		if (argc > 1) {
			int val = argv[1];
			if (argc > 2) val |= (argv[2] << 7);
			if (argc > 3) val |= (argv[3] << 14);
			analogWriteCallback(argv[0], val);
		}
		break;
	case CAPABILITY_QUERY:
		Firmata.write(START_SYSEX);
		Firmata.write(CAPABILITY_RESPONSE);
		for (byte pin = 0; pin < TOTAL_PINS; pin++) {
			if (IS_PIN_DIGITAL(pin)) {
				Firmata.write((byte)INPUT);
				Firmata.write(1);
				Firmata.write((byte)OUTPUT);
				Firmata.write(1);
			}
			if (IS_PIN_ANALOG(pin)) {
				Firmata.write(ANALOG);
				Firmata.write(10); // 10 = 10-bit resolution
			}
			if (IS_PIN_PWM(pin)) {
				Firmata.write(PWM);
				Firmata.write(8); // 8 = 8-bit resolution
			}
			if (IS_PIN_DIGITAL(pin)) {
				Firmata.write(SERVO);
				Firmata.write(14);
			}
			if (IS_PIN_I2C(pin)) {
				Firmata.write(I2C);
				Firmata.write(1);  // TODO: could assign a number to map to SCL or SDA
			}
			Firmata.write(127);
		}
		Firmata.write(END_SYSEX);
		break;
	case PIN_STATE_QUERY:
		if (argc > 0) {
			byte pin = argv[0];
			Firmata.write(START_SYSEX);
			Firmata.write(PIN_STATE_RESPONSE);
			Firmata.write(pin);
			if (pin < TOTAL_PINS) {
				Firmata.write((byte)pinConfig[pin]);
				Firmata.write((byte)pinState[pin] & 0x7F);
				if (pinState[pin] & 0xFF80) Firmata.write((byte)(pinState[pin] >> 7) & 0x7F);
				if (pinState[pin] & 0xC000) Firmata.write((byte)(pinState[pin] >> 14) & 0x7F);
			}
			Firmata.write(END_SYSEX);
		}
		break;
	case ANALOG_MAPPING_QUERY:
		Firmata.write(START_SYSEX);
		Firmata.write(ANALOG_MAPPING_RESPONSE);
		for (byte pin = 0; pin < TOTAL_PINS; pin++) {
			Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
		}
		Firmata.write(END_SYSEX);
		break;

	case LED:
		//std::vector<Led*> ledList = 
			this->robot->getLedList[argv[0]].setColor(argv[1], argv[2], argv[3]);
		break;
	}

}

/*==============================================================================
* PROCESS INPUTS
*============================================================================*/

void FirmataProcess::processInput()
{
	byte pin, analogPin;

	/* DIGITALREAD - as fast as possible, check for changes and output them to the
	* FTDI buffer using Serial.print()  */
	checkDigitalInputs();

	/* STREAMREAD - processing incoming messagse as soon as possible, while still
	* checking digital inputs.  */
	while (Firmata.available())
		Firmata.processInput();

	// TODO - ensure that Stream buffer doesn't go over 60 bytes

	currentMillis = millis();
	if (currentMillis - previousMillis > samplingInterval) {
		previousMillis += samplingInterval;
		/* ANALOGREAD - do all analogReads() at the configured sampling interval */
		for (pin = 0; pin < TOTAL_PINS; pin++) {
			if (IS_PIN_ANALOG(pin) && pinConfig[pin] == ANALOG) {
				analogPin = PIN_TO_ANALOG(pin);
				if (analogInputsToReport & (1 << analogPin)) {
					Firmata.sendAnalog(analogPin, analogRead(analogPin));
				}
			}
		}
		// report i2c data for all device with read continuous mode enabled
		if (queryIndex > -1) {
			for (byte i = 0; i < queryIndex + 1; i++) {
				readAndReportData(query[i].addr, query[i].reg, query[i].bytes);
			}
		}
	}
}

void FirmataProcess::init(Program *newRobot)
{
	this->robot = newRobot;

	Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);

	Firmata.attach(ANALOG_MESSAGE, this->analogWriteCallback);
	Firmata.attach(DIGITAL_MESSAGE, this->digitalWriteCallback);
	Firmata.attach(REPORT_ANALOG, this->reportAnalogCallback);
	Firmata.attach(REPORT_DIGITAL, this->reportDigitalCallback);
	Firmata.attach(SET_PIN_MODE, this->setPinModeCallback);
	Firmata.attach(START_SYSEX, this->sysexCallback);
	Firmata.attach(SYSTEM_RESET, this->systemResetCallback);

	// to use a port other than Serial, such as Serial1 on an Arduino Leonardo or Mega,
	// Call begin(baud) on the alternate serial port and pass it to Firmata to begin like this:
	// Serial1.begin(57600);
	// Firmata.begin(Serial1);
	// then comment out or remove lines 701 - 704 below

	Firmata.begin(57600);
	while (!Serial) {
		; // wait for serial port to connect. Only needed for ATmega32u4-based boards (Leonardo, etc).
	}
	systemResetCallback();  // reset to default config
}

FirmataProcess::FirmataProcess()
{

}
