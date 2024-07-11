# LeadCloud
## _A cloud based ledmatrix manager_

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

# How it works

## The local system
The local system is composed of 5 main threads/processes:

- LeadCloud
	- JobManager
		- Schedule manager thread
		- Job manager thread
		- Display thread
		
The LeadCloud process deals with external http/tcp communications with the server. It receives the new schedules, reports its status, reports schedule metadata (semi-implemented), replies to a screenshot request among other features. On a new schedule arrival, it sends a linux signal to the JobManager to restart itself, by use of execv, essentially wiping the stack and restarting all the functionality (the breif LeadCloud logo you see between schedules in when it restarts and hasn't loaded the schedule yet).

The JobManager starts its threads to handle, well, job management.

The schedule manager thread reads all the schedule and builds a linked list of events and their times. These events can be the start of a job, the end of a job, setting the event time, etc..

The job manager thread is the one who starts the processes of the programs to execute (i.e. pong). It forks from the process and loads the code of the program to execute, while passing it the correct arguments. The part of the process that didn't fork registers the child PID into a table of jobs. This table is holds the references to all running jobs. These jobs are stopped/resumed with unix signals. They may not be accurate, but since the observers will be human, such an insignificant inaccuracy will not be noticed by many.

The display thread takes the framebuffer the job manager processes are writing to and actually writes it to the driver.

There is a special driver (also authored by me) that allows for the 32x64 matrix to be partitioned into parts to be displayed independently, allowing even 2 instances of LeadCloud to run in 2 halves of the screen.

When LeadCloud starts, the RPI will look for a configuration file that contains properties of the system. If no file has been found, it means it hasn't been configurated. If not, it will connect to the server and attempt to get a configuration key. LeadCloud will display the key on the screen for it to be inputed in the site.

## The webserver

The webserver is fairly simple, it uses a NoSQL database to hold the users, the devices and the schedules. Basic login/register rules apply and the password is hashed before being saved in the database.

The device self register essentially writes the key and the IP of the device in a table for it to be reclaimed later by an user registering a device.

The frontend was my first try with html and javascript. It is very basic and not very pretty to look at, but is enough to demonstrate the functionality of the solution. It works mostly with POST requests and forms.

The schedule editor was made with the P5.js library and it was built with very basic control (stuff like clicking events was made by calculating position in relation to an events table), which lead to a lengthy development (4x 18h days of straight coding, pushed by a close deadline) and complicated implementation (~700 handwritten javascript lines). The schedule creator refers a daily/monthly/yearly schedule but those were not implemented as they would require an even more complicated schedule editor.

# Usage

![login or register](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/loginregister.png)


To use LeadCloud you first need to register or login:

![login](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/login.png)

After logging in, we're met with an empty list of devices.

![devices](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/nodevices.png)

Let's connect a device. To do this, connect the device to the internet (for hosted server) or locally with an ethernet cable. The device should start (not implemented) and start trying to connect to the server by itself to retreive a configuration key.

![configkey]()

Let's register the device.

![adddevice](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/adddevice.png)

With the device added, our device list is now not empty. You can see the device name, its status (real time), its configuration properties, its location, a screenshot of what it is displaying and some options to either remove the device and to assign it a schedule.

![deviceadded](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/deviceadded.png)

Let's check the schedules. Some schedules are here already.

![assignaschedule](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/assignschedule.png)

Let's create a schedule.

![nothingscheduled](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/nothingscheduled.png)

It is currently empty, meaning nothing but "LeadCloud" will play on screen. Let's watch some pong. Fill out the fields (the programs are limited to what is already in there):

![beforepong](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/beforepong.png)

Now press "add event":

![afterpong](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/afterpong.png)

Now create another one with the scrolltext program:

![creatingschedule](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/creatingschedule.png)

Great! Now save it. Note you can also edit the info fields of the schedule.

![editingscheduleinfofields](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/editingscheduleinfofields.png)

Back to the schedules. Now we can see "Text+Pong" as an option.

![scheduletoassign](https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/scheduletoassign.png)

Try assigning it!

![]()

You should see the schedule load for a breif second and the new schedule programs taking over.

So with this solution it is possible to define what is to be played at what time in a week and configure new devices.

Note: Daily/Monthy/Yearly schedule options were also offered, but are not implemented as the weekly option was implemented with a very basic javascript library, resulting in a ~700 lines of javascript I do not intend to revisit.




[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

[adddevice]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/adddevice.png>
[addedschedule]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/addedschedule.png>
[assignschedule]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/assignschedule.png>
[creatingschedule]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/creatingschedule.png>
[deviceadded]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/deviceadded.png>
[editingscheduleinfofields]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/editingscheduleinfofields.png>
[loginregister]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/loginregister.png>
[nodevices]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/nodevices.png>
[scheduletoassign]: <https://github.com/ZeD4805/LeadCloud/blob/30a6f01630c47534f1d189dbc73da724857bffe6/readme/scheduletoassign.png>
