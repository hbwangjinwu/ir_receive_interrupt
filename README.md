# IR Receive interrupt Example

This  is  a custom soft IP to interrupt the HPS example. The custom ir IP receive the data from the IR remote controller and interrupt the HPS. The linux driver reponse the interrupt and read the data. The application in the user space get the IR identify data when interrupt reached then  print the data on the screen.
![mark](http://ogtvbbrfk.bkt.clouddn.com/blog/20170208/122142333.png)
# Compile the Quartus project
Open the hardware project and compile the project to generate the .sof file.

# Linux distribution
Git clone the SoCFPGA linux source and compile the kernel. Copy the zImage file  to the fat partion in your boot SD card.

# Device tree

## generate the dts file
The PIO button information must be added to the device tree. In the SoCEDS command shell, typing the following command and get the dtb file

*sopc2dts --input soc_system.sopcinfo\
  --output socfpga.dts\
  --type dts\
  --board soc_system_board_info.xml\
  --board hps_common_board_info.xml\
  --bridge-removal all\
  --clocks*

## change the dts 
Open the socfpga.dts add the ir_receive IP description in the dts file.
find the terasic_irda and change the content as  follow£º
*		
			terasic_irda: irm@0x100010010 {
				compatible = "terasic,irm";
				reg = <0x00000001 0x00010010 0x00000004>;
				interrupt-parent = <&hps_0_arm_gic_0>;
				interrupts = <0 43 4>;
			}; //end irm@0x100010010 (terasic_irda)
*
## generate the dtb file

*dtc -I dts -o dtb -o socfpga.dtb socfpga.dts*

copy the socfpga.dtb file to the fat partion in your boot SD card.

# Compile the driver 
Edit the Makefile to change the correct kenel source directory before compiling the teraic_ir driver. Copy the terasic_ir.ko to the /home/root on your boot sd card.

# Compile the test application
Use the Make command in the SoCEDS command shell to compile the application. Copy the ir_read_test file to the  /home/root  on your boot sd card.

# Demo setup
1. Boot you SD card and configure the FPGA with the soc_system.sof file.
2. login the linux system in the putty.
3. insmod terasic_ir.ko
4. chmod +x ir_read_test
5. ./ir_read_test
6. Press the keys on the remote controller and teminal will show the identify data of the key.
![mark](http://ogtvbbrfk.bkt.clouddn.com/blog/20170208/121708287.png)