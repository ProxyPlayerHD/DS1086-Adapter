# DS1086-Adapter
SMT to DIP-8 Adapter for the DS1086 Programmable Clock Generator, Including an Arduino Sketch to program the chip

![Clock Compare](https://github.com/ProxyPlayerHD/DS1086-Adapter/assets/13576843/87c2b05f-d628-474a-8636-fb0a0fa41ba7)

This repo has the full Kicad Project including gerbers, and a PDF of the schematic. Of course it also includes the Arduino Sketch which contains the code nessesary to program the chip over a CLI like interface via serial.

It's fairly simple:

first make sure that whatever terminal you use for the Arduino uses LF characters for new lines (no CR or CR+LF)  
then connect the 4 pins of the Adapter (5V, GND, SDA, SCL) to the appropriate pins on whatever 5V arduino board you're using  
finally power the thing up and wait for the small splash screen  
after that you simply enter commands like with any other CLI, the supported commands are as follows:  

* help - displays a list of all commands and what they do
* get - displays the current clock speed of the DS1086
* set <freq. in kHz> - sets the clock speed of the DS1086 to the specified value
* write - saves the current clock speed to the EEPROM

note that the DAC Register always reads the same value regardless of the contents of the Register, so the read out can be off by up to 5MHz. if you want to be sure the speed is correct just use an Oscilloscope.

On a different note the headers for the programming interface are split because of space constraints as i wanted the whole thing to fit into the footprint of an actual DIP-8 Oscillator can.
If vertial space is a premium, the programming headers can be left unpopulated in which case you just put some wires into the holes and hold them with your fingers while programming it.

