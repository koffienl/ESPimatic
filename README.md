# ESPimatic
read/control DS18B20/DHT/IR/Relay from and to Pimatic on ESP8266

# Hardware requirements
- ESP8266 (highly recommende a version with 4mb flash size)
- Optional: DS18B20
- Optional: DHTxx
- Optional: Relays (up to 4)
- Optional: IR LED
- Optional: 2 8x8 LED Matrix displays (max7219)
- Working Pimatic environment

# Installation
Compile the sketch in Arduino and upload to your ESP
It will start in AP mode, ip address 192.168.4.1
Connect your browser to the IP and upload all the HTML files (one by one)
Reboot the ESP and connect to AP again, refresh webpage, enter wifi credentials. Wait for reboot
Your ESP should now be fully functional. In the menu go to System -> GPIO and check all the available GPIO's.
Go to System -> Pimatic and configure your Pimatic API credentials.
Now you can configure your sensors connected to ESP.

# Connecting MAX7219
The LED Matrix has 5 wires:
VCC
GND
DIN
CS
CLK
You should connect CLK to GPIO14 and DIN to GPIO13 , CS is free to configure in the website.
When enabling LED Matrix, GPIO 14 and GPIO13 are automaticly set is used. 
Select what you want to see on the matrix. Nothing will keep the matrix displaying "OK" or "AP" after startup. 
When the matrix is enabled, it will fill up the rows during wifi connecting.
