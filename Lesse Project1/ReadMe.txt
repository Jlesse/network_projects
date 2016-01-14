README

Julian Lesse
COSC 439 Fall 2014
program1 - UG - UDP Socket Programming in C

To compile, user the terminal to go to directory containing both files [UDPserv.c] and [UDPclient.c],and enter the commands [gcc -o Server UDPServ.c] and [gcc -o Client UDPClient.c]. This will create two executable files in the same directory, named Server and Client.  Next, run the server and one or more clients, each in their own window.  To run the executables, enter the command [./UDPServer {port}] 
for the server and [./UDPClient {port}] for the clients.

The client will display a menu with options 0-3. Exit, Check Capacity, Check Availability, and  Purchase. If any option besides exit is selected, The user is then prompted for the Flight ID input. If the ID is not in the data array, the server will notify the user. If the ID is found the user will be prompted further for more options. If the user chose to check Capacity or Availability, they will be prompted for the choice of Premium, Economy, or all seating. The server will then return the appropriate number of seats. If the user chose to purchase seats, they will be prompted for the number of seats they wish to purchase. If the user attempts to purchase more seats than are available, the server will notify the user of the issue. If the are enough seats available the server will return the number of available seats after purchase and modify the data file to reflect the changes. If the user chooses to exit the program a message will be sent to the server to exit as well.
 

