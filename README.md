garagedooropener
================

Garagedoor Opener with RaspberryPi
Author: smanders

These sets of files have been created to interface to the following iPhone App:

https://itunes.apple.com/us/app/mydooropener-elite/id359774310?mt=8

Originally this was supported for the Arduino from the iPhone author.  I had
a RaspberryPi looking for a purpose and this seemed like a good project to
target.  This solution is about half the cost of the Arduino code but also 
takes a fair amount of setup on the Linux side.

NOTES:	Email/SMS notifications were working when I had garagedoord working as
	a script.  I have not messed with that since moving over to a daemon.

LICENSE: GPLv2

Description of Software
=======================
php_code/		This directory contains the PHP code to reponsd and
			parse the HTTP requests from the iPhone App.  This
			direcotry also contains the executable to perform the
			aes256_decryption.  The aes code used in the iPhone App
			is non-standard.  The built-in PHP decryption code was
			not able properly decrypt the returned Challenge from
			the iPhone App.

garagedoord-0.1/	This directory contains the daemon to setup the GPIO
			pins as well as monitor the status of the garagedoor
			and send the proper notifications.  This is a full
			Linux daemon with its own init.d script included.
			Also note that this daemon also sets permissions on
			a file to allow the webserver userid to manipulate a
			GPIO pin.  This code also depends on the bcm2835 
                        library you can find here.  You must set this up before
                        you can compile this daemon.
                        http://www.open.com.au/mikem/bcm2835/index.html

aes/			This contains the code that puts a command line
			interface into the aes256_decpypt needed my PHP as
			mentioned above.

How to Build
============

aes:
cd aes
make
make install (this copies the exe to the php_code dir)

garagedoord-0.1:
cd garagedoord-0.1
(edit garagedoord with your Prowl API Key and enable Prowl notifications)
make
make install (this copies the garagedoord to /usr/sbin and 
              copies garagedoord.sh to /etc/init.d)


php_code:
(edit index.php with the Password you configure on your iPhone App)
copy the contens to your webserver base directory
i.e. /var/www/ or /var/www/base/ or /var/www/garagedoor/
I'm personally using virtual hosting with the DNS
prefix assigned to another directory.


Hardware/Software Needs outside of my software
==============================================
RaspberryPi running the latest Wheezy
	* A LAMP Setup (no MySQL required though)
        * gcc tools to compile my daemon and aes256_decrypt
	* git to pull down my repository from github.com

I also pulled a network cable from my patch panel to the garage to avoid
worrying about wireless.  This RPi will probably be an AirPlay end point
as well in the near future.

Instead of installing my own proximity sensor, I just reverse engineered
the integrated sensor on the motor control board of the garagedoor opener
and just tapped off that for my position sensor.  I did have to use a
MOSFET transitor to bring the voltage down to 3.3V where the RPi likes it.
I also bought a single channel relay from China for less than $2 shipped to
signal the remote to open the door.  My garagedoor wall mount sends an
encoded signal over to the motor board sense it can open the door, turn
on/off the light/and lockout all other remotes with just a two wire
connection.  I was not going to go there, so I just used an extra remote
and hardwared the power connector and soldered the button with some wires
to come to the relay board.  I put the relay board and the MOSFET logic 
into one of those small RadioShack project enclosures.

Estimated total cost:
iPhone App: $6 (iTunes Store)
RPi: $35 (Element14)
RPi Case: $14 (Adafriut)
PowerSupply with Cable: $5 (Amazon)
SD Card: $7 (Amazon)
MOSFET and resistor: $3 (RadioShack)
Project Enclosure and connectors: $5 (RadioShack)
Garagedoor Remote: $16 (Amazon shipped)

