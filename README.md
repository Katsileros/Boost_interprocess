BOOST IPC
=========

Description
===========
The code implements shared memory interprocess communication between two processes. 
The main process, is using threads in order to independent messaging between the processes and data write/read to shared memory.

Libraries
=========
This project uses boost::interprocess and boost::thread libraries.

Basic Testing
=======
Simple testing scenarios been written into source code files.
	-1st test is checking if the two processes read exactly the same data
	-2nd test checking if there are race conditions from asynchronous random messaging

Execute
=======
	mkdir build
	cd build
	cmake ..
	make

	run ipc_0 and then ipc_1

