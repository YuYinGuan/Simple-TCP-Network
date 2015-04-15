README

Name: Yu Yin Guan
Email: yguan5@ucsc.edu
Section: Wednesday 4PM - 6PM
Date: 5/9/2014
Assignment: Lab #3

File Descriptions:

   Documentation:
      - Client_FlowChart:              A flow chart on the client side of things, did not include what happens in thread
      - Server_FlowChart:              A flow chart on the Server side of things
      - Pthread_FlowChart:             overall description of what is happening in the pthread inside client
      - Packet_Transfer_Diagram:       What my program is intended/does do in term of packet transfer between client and server
      - Document:                      A report on my program with explanation to how, what, why on the design of program along with some answer to question and notes
      - README:                        Notes and description of what is in these files
      
   Source:
      - Client.c:                      Client side of the Reliable UDP Client/Server, uses pthread for concurrency
      - Server.c:                      Server side of the Reliable UDP Client/Server, uses select for concurrency
      - Makefile:                      creates binaries for Client.c and Server.c when command "make" is entered, "make clean" will clean the other object files
      - server-info.text:              Contain a list of Ip and port number of servers, modified by me for the ease of debugging on my end 
      
   Transfer_files:
      -testfile1.txt - testfile6.txt:  Documents I have been testing with, by transferring them over through the UDP program. The data in these files are news 
                                       articles copied from fox news website. They are not code that tests my program but the files used to transfer using my program.

Instructions:
   -  Please place the testfile1.txt - testfile6.txt into the same folder as the .c files if want to test the UDP Client/Server program. 
   -  Run make to create the object files Client and Server.
   -  To run the program for Server:   ./Server <port>
   -  To run the program for Server:   ./Client <file-name> <number of chunks>
   -  Be sure the server-info.text have the correct ip and port of server. Also, please do not include new line after the last ip and port as it will make
      my program think there is an extra server.
   
Note:
   I do not have test code for them program as I did my testing with print statements. These print statements are later 
   removed due to the fact that the code was unreadable with them. I did however, left the sleep functions which can be turned 
   on with the defines by commenting them. That debugMode is just delays I hard coded into the server to simulate lost packets
   which aided me in testing. The way I tested out the reliability protocols are basically just that where I introduce delay so
   the client thinks packets has been lost. Also, the IP and port of the server-info.text was modified by me for the ease of 
   debugging on my end. Not sure if that is okay, if not just change the file back to the one on the Lab 3 instructions
   
   *Important: Do not use files for transfer which contains : as this is a indicator I used to parse my arguments to clinet/servers.
   
Tested:
      - Time-outs of Client, drops if over 10 has been consequentially happening
      - Time-outs of Server, drops all sockets if none of them has been talking for a while
      - small byte data where it does not exceed UDP buffer
      - if data receives correctly
      - ignore wrong packet
      - retransmission of data and requests
      - Time-out timings
      - request for data that was not complete due to disconnection
      
Error:
   I have tested recently and found that the program will do weird stuff such as memory problems and false data when the transfer files 
   is too large. Not sure if it will get better with more servers but I have only tested with 1 or 2 servers for transfer.
