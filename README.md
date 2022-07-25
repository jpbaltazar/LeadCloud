# LeadCloud
## _A cloud based ledmatrix manager_
[![N|Solid](https://cldup.com/dTxpPi9lDf.thumb.png)](https://nodesource.com/products/nsolid)

[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger)

# LeadCloud

Lead cloud is a project that aims to created a distributed remote network for managing signage local systems.

## Organization

The project is composed of 2 subprojects: a webserver written in Golang and a local system made in C++. This project uses minimal libraries (apart from standard language ones, json-cpp, p5.js and a mongoDB Golang driver are the only ones used for the project itself.

## Local system
The local system is composed of a RPI 4 and a adafruit RGB power shield, some power cables and a HUB75 32x64 LED matrix.

The RPI has custom linux image built with buildroot, which contains several libraries that are useful for the composition of the server.

This project can take more chained screens but it has not been tested for it.

## Remote system
The remote system is composed solely of the server and it is connected to a NoSQL database (MongoDB).

# 1st setup

To use LeadCloud you first need to register:





[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [dill]: <https://github.com/joemccann/dillinger>
   [adddevice]: <./readme/adddevice.png>

