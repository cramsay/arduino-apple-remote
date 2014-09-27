arduino-apple-remote-api
========================

An arduino sketch to imitate an apple remote with the controls exposed through a simple web API.
Exposing the controls through the local network lets you keep the apple tv in a closed unit.


## Requirements

The only software requirement of this sketch is the [Arduino-IRremote](https://github.com/shirriff/Arduino-IRremote)
library. Just clone it to your arduino "libraries" folder.


## Hardware

The web API uses the ethernet shield.
There is also an IR LED (with suitable current-limiting resistor) between pin 3 and ground.


## Web API Structure

The different remote commands can be induced by making different GET requests to the arduino.
By default the arduino will try to use the ip address 192.168.0.69. You can type the addresses
into a web browser manually, or see the next section for a prototype web interface.

Command | URL
--------|--------------------
Up      | 192.168.0.69/up
Down    | 192.168.0.69/down
Left    | 192.168.0.69/left
Right   | 192.168.0.69/right
Centre  | 192.168.0.69/centre
Menu    | 192.168.0.69/menu


## Using the web interface

The arduino only exposes the controls through the simple API - any UI for practical use must be hosted
elsewhere. For example, I've included a simple web page which has buttons for each of the commands.
Using ajax, requests will be sent to the arduino over the local network. A demo of this very simple 
web interface is hosted [here](http://cramsay.co.uk/apple-remote-control.html).
