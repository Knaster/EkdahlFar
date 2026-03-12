myprogram = src/main.cpp
output_filename = solenoid
builddir = build
#bindir = bin
#mcu = atmega2560
mcu = samd21
speed = 16000000UL
farincludes =	-I../../include/solenoid/ \
				-I../../include/ \
				-I../../include/base/ \
				-I../../src/base/

libincludes =	-I../../lib/SafeString/src/

safestringdir = ../../lib/SafeString/src/

avrincludes =	-I../ArduinoSTL/src/

mainsrcdir = ../../src/
gccflags = -fpermissive -Wno-sized-deallocation -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -fno-rtti -MMD -Wl,--gc-sections -flto
# -std=gnu++11

#-std=gnu++11
eabicompileflags = -mcpu=cortex-m0plus -mthumb -c -g -Os -Werror=return-type -ffunction-sections -fdata-sections -fno-threadsafe-statics -nostdlib \
	--param max-inline-insns-single=500 -fno-rtti -fno-exceptions -fpermissive

eabimergeflags = -w -x c++ -E -CC

feathm0defs = -DARDUINO_SAMD_ZERO -DARDUINO_ARCH_SAMD -DARDUINO_SAMD_ADAFRUIT -DADAFRUIT_FEATHER_M0 -DARDUINO_SAMD_ZERO -DUSB_VID=0x239A -DUSB_PID=0x800B -DUSB_MANUFACTURER="Adafruit" \
	-DUSB_PRODUCT="Feather M0" -DADAFRUIT_FEATHER_M0

mkrzerodefs =  -DARDUINO_SAMD_MKRZERO -DUSE_ARDUINO_MKR_PIN_LAYOUT -DARDUINO_ARCH_SAMD -DUSB_VID=0x2341 -DUSB_PID=0x804f -DUSBCON -DUSB_MANUFACTURER="Arduino LLC" \
	-DUSB_PRODUCT="Arduino MKRZero"

eabicompiledefs =-DF_CPU=48000000L -DARDUINO=10607 -D__SAMD21G18A__ -DUSBCON
# -DUSB_CONFIG_POWER=100
#-DARM_MATH_CM0PLUS -DARM

eabicompiledefs2 = -Os -D__SAMD21G18A__ -DARDUINO_SAMD_ZERO -DARM_MATH_CM0PLUS
# -DUSBCON
#-DUSB_CONFIG_POWER=100 \
#-DUSB_VID=0x239A -DUSB_PID=0x800B
#-DUSB_MANUFACTURER="Adafruit" -DUSB_PRODUCT="Feather M0"

eabicompiler = /home/knas/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4/bin/arm-none-eabi-g++
#/home/knas/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-g++

eabiarduino = -I/home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/libraries/Adafruit_TinyUSB_Arduino/src/arduino

featherm0includes =  -I/home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/libraries/Adafruit_TinyUSB_Arduino/src/arduino \
				-I/home/knas/.arduino15/packages/adafruit/tools/CMSIS/5.4.0/CMSIS/Core/Include/ \
				-I/home/knas/.arduino15/packages/adafruit/tools/CMSIS/5.4.0/CMSIS/DSP/Include/  \
				-I/home/knas/.arduino15/packages/adafruit/tools/CMSIS-Atmel/1.2.2/CMSIS/Device/ATMEL/ \
				-I/home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/cores/arduino  \
				-I/home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/variants/feather_m0 \

mkrzeroincludes = -I/home/knas/.arduino15/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Include/ \
				  -I/home/knas/.arduino15/packages/arduino/tools/CMSIS-Atmel/1.2.0/CMSIS/Device/ATMEL/ \
				  -I/home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/cores/arduino/api/deprecated \
				  -I/home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/cores/arduino/api/deprecated-avr-comp \
				  -I/home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/cores/arduino \
				  -I/home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrzero

#				  -I/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/cores/arduino/api/deprecated \
#				  -I/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/cores/arduino/api/deprecated-avr-comp \
#				  -I/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/cores/arduino \
#				  -I/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/variants/mkrzero


feathervariant_cpp = /home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/variants/feather_m0/variant.cpp
feathervariant_name = featherm0
feathervariant_ld = /home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/variants/feather_m0/linker_scripts/gcc/flash_with_bootloader.ld
feathervariant_csmis = /home/knas/.arduino15/packages/adafruit/tools/CMSIS/5.4.0/CMSIS/Lib/GCC/
feathervariant_dir = /home/knas/.arduino15/packages/adafruit/hardware/samd/1.7.16/variants/feather_m0
feathervariant_lib = ../processor_cores/featherm0.a

mkrzvariant_cpp = /home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrzero/variant.cpp
#/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/variants/mkrzero/variant.cpp
mkrzvariant_name = mkrzero
mkrzvariant_ld = /home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrzero/linker_scripts/gcc/flash_with_bootloader.ld
#/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/variants/mkrzero/linker_scripts/gcc/flash_with_bootloader.ld
mkrzvariant_csmis = /home/knas/.arduino15/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Lib/GCC/
mkrzvariant_dir = /home/knas/.arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrzero
#/home/knas/Arduino/hardware/arduino-git/ArduinoCore-samd/variants/mkrzero
mkrzvariant_lib = ../processor_cores/mkrzerov2.a
#/home/knas/.cache/arduino/sketches/DB7FA82C0C89B17BA8EA2115886D836D/../../cores/arduino-git_ArduinoCore-samd_mkrzero_f6b9acb749c49ebe6a6d58e6b2b1bde7/core.a

variant_cpp = $(mkrzvariant_cpp)
variant_name = $(mkrzvariant_name)
variant_ld = $(mkrzvariant_ld)
variant_csmis = $(mkrzvariant_csmis)
variant_dir = $(mkrzvariant_dir)
variant_lib = $(mkrzvariant_lib)
eabiincludes = $(mkrzeroincludes)
eabicompiledefs := $(eabicompiledefs) $(mkrzerodefs)

eabilinkflags = -Os -Wl,--gc-sections -save-temps

eabilinkflags2 = --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
				 -Wl,--warn-section-align

eabiobjcopy = /home/knas/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-objcopy
eabisize = /home/knas/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-size

arduinodir = ../arduino
arduinobd = $(arduinodir)/$(builddir)/$(mcu)
farcored = ../farcore
farcoredf = $(farcored)/$(builddir)/$(mcu)/
arduinocore = $(arduinobd)/HardwareSerial.o $(arduinobd)/HardwareSerial0.o $(arduinobd)/Print.o $(arduinobd)/wiring.o $(arduinobd)/hooks.o $(arduinobd)/WString.o $(arduinobd)/new.o \
	$(arduinobd)/wiring_analog.o $(arduinobd)/wiring_digital.o $(arduinobd)/Stream.o

farbasesrc = $(mainsrcdir)/base/
farbase = $(farcoredf)/commanditem.o $(farcoredf)/commandparser.o $(farcoredf)/modulesystembasics.o $(farcoredf)/module.o $(farcoredf)/basemodule.o

Debug: avr
Release: avr

Atmega328p:
	gccx=avr-gcc gppx=avr-g++ mcu=atmega328p make avr
Atmega2560:
	gccx=avr-gcc gppx=avr-g++ mcu=atmega2560 make avr

cleanRelease: clean
cleanDebug: clean

Samd21:
	rm -r $(builddir)/samd21
	mkdir -p $(builddir)
	mkdir -p $(builddir)/samd21
	$(eabicompiler) $(eabicompileflags) $(eabimergeflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) $(myprogram) -o $(builddir)/samd21/sketch_merged.cpp
	@echo "Sketch merged"
	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) $(myprogram) -o $(builddir)/samd21/$(output_filename).o
	@echo "Initial main build"
	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) \
		 $(variant_cpp) -o $(builddir)/samd21/$(variant_name).o
	@echo "Variant build"

	mkdir -p $(farcoredf)

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) \
		$(farbasesrc)commanditem.cpp -o $(farcoredf)commanditem.o

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) \
		$(farbasesrc)commandparser.cpp -o $(farcoredf)commandparser.o

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) \
		$(farbasesrc)modulesystembasics.cpp -o $(farcoredf)modulesystembasics.o

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) \
		$(farbasesrc)module.cpp  -o $(farcoredf)module.o

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) \
		$(farbasesrc)basemodule.cpp  -o $(farcoredf)basemodule.o

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiarduino) $(eabicompiledefs2) $(eabiincludes) $(farincludes) \
		$(mainsrcdir)solenoid/solenoid.cpp -o $(farcoredf)solenoid.o

		# $(eabiarduino) $(eabicompiledefs2) removed from all following lines

	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiincludes) $(farincludes) \
		$(safestringdir)BufferedInput.cpp -o $(builddir)/samd21/BufferedInput.o
	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiincludes) $(farincludes) \
		$(safestringdir)BufferedOutput.cpp -o $(builddir)/samd21/BufferedOutput.o
	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiincludes) $(farincludes) \
		$(safestringdir)SafeString.cpp -o $(builddir)/samd21/SafeString.o
	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiincludes) $(farincludes) -I$(safestringdir)  \
		$(safestringdir)SafeStringReader.cpp -o $(builddir)/samd21/SafeStringReader.o
	$(eabicompiler) $(eabicompileflags) $(eabicompiledefs) $(eabiincludes) $(farincludes) \
		$(safestringdir)SafeStringStream.cpp -o $(builddir)/samd21/SafeStringStream.o

	@echo "Built FAR objects"

	$(eabicompiler) $(eabilinkflags) -T$(variant_ld) \
	-Wl,-Map,$(builddir)/samd21/$(output_filename).map $(eabilinkflags2) -o $(builddir)/samd21/$(output_filename).elf $(builddir)/samd21/$(output_filename).o \
	$(builddir)/samd21/$(variant_name).o \
	$(builddir)/samd21/BufferedInput.o $(builddir)/samd21/BufferedOutput.o $(farbase) $(builddir)/samd21/SafeString.o $(builddir)/samd21/SafeStringReader.o $(builddir)/samd21/SafeStringStream.o \
	$(farcoredf)solenoid.o \
	-Wl,--start-group -L$(variant_csmis) \
	-larm_cortexM0l_math -L$(variant_dir) \
	-lm $(variant_lib) -Wl,--end-group
	@echo "Linking"
	$(eabiobjcopy) -O binary $(builddir)/samd21/$(output_filename).elf $(builddir)/samd21/$(output_filename).bin
	$(eabiobjcopy) -O ihex -R .eeprom $(builddir)/samd21/$(output_filename).elf $(builddir)/samd21/$(output_filename).hex
	$(eabisize) -A $(builddir)/samd21/$(output_filename).elf

	#stty -F /dev/ttyACM0 1200;stty -F /dev/ttyACM0 1200;"/home/knas/.arduino15/packages/arduino/tools/bossac/1.7.0-arduino3/bossac" -i -d --port=ttyACM0 -U true -i -e -w -v \
	#$(builddir)/samd21/$(output_filename).bin -R
	#stty -a -F /dev/ttyACM0
	#stty -a -F /dev/ttyACM0
	#
avr:
	clear
	mkdir -p $(builddir)/$(mcu)
#	mkdir -p $(bindir)
	mkdir -p $(arduinodir)/$(builddir)/$(mcu)
	mkdir -p $(farcoredf)
	cp $(arduinodir)/main.cpp $(builddir)/$(mcu)/main.cpp
	head $(builddir)/$(mcu)/main.cpp -n 21 > $(builddir)/$(mcu)/combined_main.cpp
	echo 'extern "C" void __cxa_pure_virtual() { while(1);};' >> $(builddir)/$(mcu)/combined_main.cpp
	cat $(myprogram) >> $(builddir)/$(mcu)/combined_main.cpp
	tail $(builddir)/$(mcu)/main.cpp -n 30 >> $(builddir)/$(mcu)/combined_main.cpp

	avr-g++ -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c $(arduinodir)/*.cpp -I$(arduinodir)
	$(gccx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c $(arduinodir)/*.c -I$(arduinodir)
	mv *.o $(arduinobd)

	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(farcoredf)/debugprint.o $(mainsrcdir)/base/debugprint.cpp -I$(arduinodir) $(farincludes) $(avrincludes) -DssOutput=Serial
	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(farcoredf)/commandparser.o $(mainsrcdir)/base/commandparser.cpp -I$(arduinodir) $(farincludes) $(avrincludes) $(gccflags)
	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(farcoredf)/commanditem.o $(mainsrcdir)/base/commanditem.cpp -I$(arduinodir) $(farincludes) $(avrincludes) $(gccflags)
	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(farcoredf)/modulesystembasics.o $(mainsrcdir)/base/modulesystembasics.cpp -I$(arduinodir) $(farincludes) $(avrincludes) $(gccflags)
	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(farcoredf)/module.o $(mainsrcdir)/base/module.cpp -I$(arduinodir) $(farincludes) $(avrincludes) $(gccflags)

	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(farcoredf)/solenoid.o $(mainsrcdir)/solenoid/solenoid.cpp -I$(arduinodir) $(farincludes) $(avrincludes) $(gccflags)

	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -c -o $(builddir)/$(mcu)/main.o $(builddir)/$(mcu)/combined_main.cpp -I$(arduinodir) $(farincludes) $(avrincludes) $(gccflags)
#$(builddir)/usb.o
	$(gppx) -Os -mmcu=$(mcu) -DF_CPU=$(speed) -o $(builddir)/$(mcu)/$(output_filename).elf $(arduinocore) $(farbase) $(farcoredf)/solenoid.o \
	$(builddir)/$(mcu)/main.o $(gccflags) -Wl,-Map=$(builddir)/$(mcu)/$(output_filename).map

	avr-objcopy -O ihex -R .eeprom $(builddir)/$(mcu)/$(output_filename).elf $(builddir)/$(mcu)/$(output_filename).hex
	avr-nm -S --size-sort $(builddir)/$(mcu)/$(output_filename).elf
	avr-size -A  $(builddir)/$(mcu)/$(output_filename).elf

	#avrdude -C /etc/avrdude.conf -v -V -patmega328p -carduino -b57600 -P/dev/ttyUSB0 -D -Uflash:w:bin/hello.hex:i
clean:
	rm $(builddir)/*
