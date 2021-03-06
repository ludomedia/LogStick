EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:logstick-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "13 jun 2013"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ATTINY85-P IC1
U 1 1 519BABEE
P 5850 3300
F 0 "IC1" H 4800 3700 60  0000 C CNN
F 1 "ATTINY85-P" H 6700 2900 60  0000 C CNN
F 2 "DIP8" H 4800 2900 60  0001 C CNN
F 3 "" H 5850 3300 60  0000 C CNN
	1    5850 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 1450 7200 3100
$Comp
L R R1
U 1 1 519BAD11
P 3900 3050
F 0 "R1" V 3980 3050 40  0000 C CNN
F 1 "68R" V 3907 3051 40  0000 C CNN
F 2 "~" V 3830 3050 30  0000 C CNN
F 3 "~" H 3900 3050 30  0000 C CNN
	1    3900 3050
	0    -1   -1   0   
$EndComp
$Comp
L R R2
U 1 1 519BAD67
P 3900 3250
F 0 "R2" V 3980 3250 40  0000 C CNN
F 1 "68R" V 3907 3251 40  0000 C CNN
F 2 "~" V 3830 3250 30  0000 C CNN
F 3 "~" H 3900 3250 30  0000 C CNN
	1    3900 3250
	0    -1   -1   0   
$EndComp
$Comp
L R R3
U 1 1 519BAD76
P 2350 2600
F 0 "R3" V 2430 2600 40  0000 C CNN
F 1 "1k5" V 2357 2601 40  0000 C CNN
F 2 "~" V 2280 2600 30  0000 C CNN
F 3 "~" H 2350 2600 30  0000 C CNN
	1    2350 2600
	1    0    0    -1  
$EndComp
$Comp
L ZENER D1
U 1 1 519BAD9D
P 2350 4450
F 0 "D1" H 2350 4550 50  0000 C CNN
F 1 "3,6V" H 2350 4350 40  0000 C CNN
F 2 "~" H 2350 4450 60  0000 C CNN
F 3 "~" H 2350 4450 60  0000 C CNN
	1    2350 4450
	0    -1   -1   0   
$EndComp
$Comp
L ZENER D2
U 1 1 519BADAC
P 2650 4450
F 0 "D2" H 2650 4550 50  0000 C CNN
F 1 "3,6V" H 2650 4350 40  0000 C CNN
F 2 "~" H 2650 4450 60  0000 C CNN
F 3 "~" H 2650 4450 60  0000 C CNN
	1    2650 4450
	0    -1   -1   0   
$EndComp
$Comp
L CONN_4 K1
U 1 1 519BADEC
P 2000 1750
F 0 "K1" V 1950 1750 50  0000 C CNN
F 1 "USB" V 2050 1750 50  0000 C CNN
F 2 "" H 2000 1750 60  0000 C CNN
F 3 "" H 2000 1750 60  0000 C CNN
	1    2000 1750
	0    1    -1   0   
$EndComp
$Comp
L CONN_3 K2
U 1 1 519BAE03
P 4250 1750
F 0 "K2" V 4200 1750 50  0000 C CNN
F 1 "DHT44" V 4300 1750 40  0000 C CNN
F 2 "" H 4250 1750 60  0000 C CNN
F 3 "" H 4250 1750 60  0000 C CNN
	1    4250 1750
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4150 3050 4500 3050
Wire Wire Line
	2350 2850 2350 4250
Connection ~ 2350 3050
Wire Wire Line
	4150 3250 4500 3250
Wire Wire Line
	2050 3050 3650 3050
Wire Wire Line
	2050 3050 2050 2100
Wire Wire Line
	1950 3250 3650 3250
Wire Wire Line
	1950 3250 1950 2100
Wire Wire Line
	1850 4650 7550 4650
Wire Wire Line
	2650 4650 2350 4650
Wire Wire Line
	1850 4650 1850 2100
Connection ~ 2350 4650
Wire Wire Line
	7200 3500 7200 4700
Connection ~ 2650 4650
Wire Wire Line
	2650 3250 2650 4250
Connection ~ 2650 3250
$Comp
L R R4
U 1 1 519BAF01
P 4250 2600
F 0 "R4" V 4330 2600 40  0000 C CNN
F 1 "68R" V 4257 2601 40  0000 C CNN
F 2 "~" V 4180 2600 30  0000 C CNN
F 3 "~" H 4250 2600 30  0000 C CNN
	1    4250 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 2350 4250 2100
Wire Wire Line
	4250 2850 4250 3350
Wire Wire Line
	4250 3350 4500 3350
Wire Wire Line
	3550 2100 3550 1450
Wire Wire Line
	2350 2100 2350 2350
Wire Wire Line
	2150 2100 4150 2100
Connection ~ 3550 2100
Wire Wire Line
	3550 1450 7200 1450
Wire Wire Line
	7550 4650 7550 2100
Wire Wire Line
	7550 2100 4350 2100
Connection ~ 7200 4650
$Comp
L CONN_2 K3
U 1 1 519BB00E
P 3200 2550
F 0 "K3" V 3150 2550 40  0000 C CNN
F 1 "SWITCH" V 3250 2550 40  0000 C CNN
F 2 "" H 3200 2550 60  0000 C CNN
F 3 "" H 3200 2550 60  0000 C CNN
	1    3200 2550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4500 3150 3300 3150
Wire Wire Line
	3300 3150 3300 2900
Wire Wire Line
	3100 2900 3100 4650
Connection ~ 3100 4650
$Comp
L LED D3
U 1 1 519BB08A
P 4250 4350
F 0 "D3" H 4250 4450 50  0000 C CNN
F 1 "LED" H 4250 4250 50  0000 C CNN
F 2 "~" H 4250 4350 60  0000 C CNN
F 3 "~" H 4250 4350 60  0000 C CNN
	1    4250 4350
	0    1    1    0   
$EndComp
$Comp
L R R5
U 1 1 519BB0A3
P 4250 3800
F 0 "R5" V 4330 3800 40  0000 C CNN
F 1 "220R" V 4257 3801 40  0000 C CNN
F 2 "~" V 4180 3800 30  0000 C CNN
F 3 "~" H 4250 3800 30  0000 C CNN
	1    4250 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 4050 4250 4150
Wire Wire Line
	4500 3450 4250 3450
Wire Wire Line
	4250 3450 4250 3550
Wire Wire Line
	4250 4550 4250 4650
Connection ~ 4250 4650
NoConn ~ 4500 3550
$Comp
L GND #PWR01
U 1 1 519BB9A2
P 7200 4700
F 0 "#PWR01" H 7200 4700 30  0001 C CNN
F 1 "GND" H 7200 4630 30  0001 C CNN
F 2 "" H 7200 4700 60  0000 C CNN
F 3 "" H 7200 4700 60  0000 C CNN
	1    7200 4700
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR02
U 1 1 519BB9D5
P 7200 1450
F 0 "#PWR02" H 7200 1540 20  0001 C CNN
F 1 "+5V" H 7200 1540 30  0000 C CNN
F 2 "" H 7200 1450 60  0000 C CNN
F 3 "" H 7200 1450 60  0000 C CNN
	1    7200 1450
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG03
U 1 1 519BBA8C
P 2500 1950
F 0 "#FLG03" H 2500 2045 30  0001 C CNN
F 1 "PWR_FLAG" H 2500 2130 30  0000 C CNN
F 2 "" H 2500 1950 60  0000 C CNN
F 3 "" H 2500 1950 60  0000 C CNN
	1    2500 1950
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG04
U 1 1 519BBA9B
P 1650 2100
F 0 "#FLG04" H 1650 2195 30  0001 C CNN
F 1 "PWR_FLAG" H 1650 2280 30  0000 C CNN
F 2 "" H 1650 2100 60  0000 C CNN
F 3 "" H 1650 2100 60  0000 C CNN
	1    1650 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 2100 1650 2100
Wire Wire Line
	2500 1950 2500 2100
Connection ~ 2500 2100
Connection ~ 2350 2100
$Comp
L C C1
U 1 1 51B9BB50
P 7200 3300
F 0 "C1" H 7200 3400 40  0000 L CNN
F 1 "100nF" H 7206 3215 40  0000 L CNN
F 2 "~" H 7238 3150 30  0000 C CNN
F 3 "~" H 7200 3300 60  0000 C CNN
	1    7200 3300
	1    0    0    -1  
$EndComp
Connection ~ 7200 3050
Connection ~ 7200 3550
$EndSCHEMATC
