#
"""
This script is for reading sumo configuration file to get simulation time, simulation steps and other external parameters
Then it runs Sumo in CLI mode to get vehicles initial positions
"""
#	@file	 DevicePlacement.py
#	@author  Kshitij Singh
#	@date    02-07-2016
#	@update  Nitin Yerra
#   @data    02-03-2024
#   @version 14.1.11 (Python 3.11)
"""
This file is a part of Netsim code. It uses libraries to establish TCP connection between Sumo and python, and get/modify data dynamically
Sumo is a free software, however Netsim is a commercial software. Users are not advised to modify the code, however they can use this code
to learn about the Sumo libraries, basic file reading/writing operations in python etc.
"""

#Common Libraries
#	<os>
"""
This module provides a portable way of using operating system dependent functionality. If you just want to read or write a file see open(),
if you want to manipulate paths, see the os.path module, and if you want to read all the lines in all the files on the command line see the file input
module. For creating temporary files and directories see the temp file module, and for high-level file and directory handling see the shutil module.
"""

#	<sys>
"""
This module provides access to some variables used or maintained by the interpreter and to functions that interact strongly with the interpreter.
 It is always available
"""

#	<time>
"""
This module provides various time-related functions. For related functionality, see also the date time and calendar modules.
Although this module is always available, not all functions are available on all platforms. Most of the functions defined in this module call platform C library
functions with the same name. It may sometimes be helpful to consult the platform documentation, because the semantics of these functions varies among platforms.
"""

#	<subprocess>
"""
The subprocess module allows you to spawn new processes, connect to their input/output/error pipes, and obtain their return codes. This module intends to replace
several older modules and functions:
"""
import subprocess
import sys

# List of required modules
required_modules = ['matplotlib', 'pywin32']

def check_and_install_modules(required_modules):
    installed_modules = subprocess.check_output(['pip', 'list']).decode('utf-8')
    installed_modules = [line.split()[0] for line in installed_modules.splitlines()[2:]]

    missing_modules = [module_name for module_name in required_modules if module_name not in installed_modules]

    if missing_modules:
        print("Installing the requirements...")
        for module_name in missing_modules:
            # Redirect stdout and stderr to suppress output during pip install
            subprocess.call([sys.executable, '-m', 'pip', 'install', module_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print("Requirements installed successfully.")
    else:
        print("All required modules are already installed. Proceeding with the script.")

def get_traffic_lights(max_x,max_y):
    # Get the list of traffic light IDs in the simulation
    try:
        traffic_light_ids = traci.trafficlight.getIDList()

        # Define the initial grid size and the percentage of the grid size for the image size
        initial_grid_size = max(max_x, max_y)
        image_size_percentage = 0.02  # 1.5% of the grid size

        # Calculate the image size based on the initial grid size and percentage
        image_size = initial_grid_size * image_size_percentage

        # Create a Matplotlib figure and axis
        fig, ax = plt.subplots(figsize=(15, 15))  # Adjust the figure size as needed for your grid

        # Iterate through the traffic lights and get their coordinates
        print("\nreading the traffic lights at junctions")
        for traffic_light_id in traffic_light_ids:
            # Use traci.junction.getPosition to retrieve coordinates
            tl_position = traci.junction.getPosition(traffic_light_id)
            x, y = tl_position

            # Get the directory of the current script
            script_dir = os.path.dirname(os.path.abspath(__file__))

            # Construct the path to the "VANET-SUMO" folder
            vanet_sumo_folder = os.path.join(script_dir, 'VANET_Interface')

            # Construct the image path relative to the "VANET-SUMO" folder
            image_path = os.path.join(vanet_sumo_folder, 'traffic_light.png')

            # Load the image from the dynamically constructed path
            img = mpimg.imread(image_path)

            # Calculate the extent for the image based on the grid size and image size
            image_extent = [
                x - image_size / 2,
                x + image_size / 2,
                y - image_size / 2,
                y + image_size / 2
            ]

            # Display the image on the plot at the traffic light coordinates with the calculated extent
            ax.imshow(img, extent=image_extent)
            # Create a Matplotlib figure and axis
    except Exception as e:
        pass
# The below commented code is to fill the junction region

# def get_junction(max_x, max_y):
    # junction_ids = traci.trafficlight.getIDList()

    # # Create a Matplotlib figure and axis
    # fig, ax = plt.subplots(figsize=(15, 15))  # Adjust the figure size as needed for your grid

    # # Iterate through the junctions and get their shape coordinates
    # for junction_id in junction_ids:
        # junction_shape = traci.junction.getShape(junction_id)
        # print(f"Junction ID: {junction_id}, Shape Coordinates: {junction_shape}")

        # # Convert the shape coordinates to Matplotlib polygons and fill with a color
        # polygon = patches.Polygon(junction_shape, closed=True, facecolor='black', edgecolor='black', alpha=0.5)
        # ax.add_patch(polygon)

# Check and install missing modules without user notification
check_and_install_modules(required_modules)


import importlib
import os
import time
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import matplotlib.patches as patches
import re
import math

#Command Arguments passed as python DevicePlacement.py <path/sumo_config_file_name.sumo.cfg> <path/File_to_be_written_name.txt>
config_file=sys.argv[1]        							#Configuration File name
file_for_writing=sys.argv[2]							#File to be written on

print("Configuration file =", config_file)
print('')
print("File_for_writing =", file_for_writing)
print('')

B= [str(x) for x in file_for_writing.split("\\")]

C=len(B[len(B)-1])

#This will create roadsplacement file, by removing last term of path (Namely devicePlacement.txt and adding RoadsPlacement.txt (Same path reserved)
grid_setting=file_for_writing[:-C]
imgpath = file_for_writing[:-C] + "GridBackground.png"                                        # Creates the image of sumo road scenario
grid_setting = grid_setting +"GridSettings.txt"	                                             # Gives the grid origin and width same as the sumo road scenario

#Specify SUMO_HOME for sumo directory in Environment variables of your system
print("Sumo Directory =",os.environ['SUMO_HOME'])
SUMO_HOME = os.environ['SUMO_HOME']

#Stripping for removing \n, \0"
config_file_stripped=config_file.strip()

#Lets check whether Sumo is present or not, and then go to the tools folder of Sumo
print("Checking Sumo...")
try:
    sys.path.append(os.path.join(SUMO_HOME, "tools")) 			#go to tools folder
    from sumolib import checkBinary								#A sumo back-end library to check if sumo is present or not
except ImportError:
    sys.exit("Please declare environment variable 'SUMO_HOME' as the root directory of your sumo installation (it should contain folders 'bin', 'tools' and 'docs')")

# Specify a Port for TCP connection. Python will interact with Sumo on this port. Make sure no other application is using this port
PORT = 8813

sumoBinary = checkBinary('sumo')

#Open Sumo on the specified port. It is opened in parallel to python program, with standard-in and standard-out for connections
sumoProcess = subprocess.Popen([sumoBinary,  "-c", config_file,'--start','--remote-port', str(PORT)], stdout=sys.stdout, stderr=sys.stderr)

#open the file for writing specified in argument
print("File to be written ",file_for_writing)
fw = open(file_for_writing, 'w')
fgrid = open(grid_setting,'w')

#Import Sumo backend libraries for TCP connection
import traci, string
import traci.constants as tc
Temp =1 # NetSim origin either top to bottom or bottom to top 
""
#Initiate connection on Specified Port
print("Initiating port...")
traci.init(PORT)

# write #Device Placement File in file_to_be_written
fw.write("#Device Placement File\n")

#Get the Boundary of Sumo Simulation. This would be used in Netsim to specify Environment
r=traci.simulation.getNetBoundary()

file=config_file
config_read = open(file, 'r')   #Open the sumo Config file
k=0
config_file_r="c"

print("Reading file...")

while (config_file_r):
	config_file_r=config_read.readline()  #read the config file line by line
	if config_file_r.find("step")>0:	  #if you find step, stop
		break


print("Writing Simulation Steps:")

A=config_file_r.strip()				#Remove \n \0 \t characters
B = [str(x) for x in A.split('"')]	#Split on double quotes ""
fgrid.write("#UI_PARAMETERS\n") 
fgrid.write("Step = ")					#Write "Step =" in the file
fgrid.write(B[1])						#Write the Simulation Step value
fgrid.write('\n')						#Next line

config_read = open(file, 'r')		#Again open the file

print("Reading file again")

k=0
config_file_r="c"
while (config_file_r):
    config_file_r=config_read.readline()	#Read File line by line
	#print config_file
	#time.sleep(1)
    if config_file_r.find("end")>0:			#Exit on finding end
        break
print("Writing Simulation time")

A=config_file_r.strip()				#Remove \n \0 \t characters
B2 = [str(x) for x in A.split('"')]	#Split on double quotes "" 
fgrid.write("Simulation time = ")		#Write "Simulation time =" in the file
fgrid.write(B2[1])						#Write the Simulation time value
fgrid.write('\n')						#Next line

	#	break

vehicle_pos=''						#This variable is the string. It works on the principal that it will Concatenate for each vehicles starting position
departed_no=0						#This variable adds up on finding a vehicle that depart at each point of sumo time
#time.sleep(5.0)
step = 0							#Simulation step counter
l=1

curr_list=''


print("Running Simulation...")
while step < float(B2[1]):			#As long as it is less than Simulation time
   #time.sleep(1.0)
	traci.simulationStep()			#Proceed a Simulation step in Sumo
	curr_departed_list=traci.simulation.getDepartedIDList()		#List of Vehicles departed in this simulation step
	curr_departed_no=traci.simulation.getDepartedNumber()		#No. of vehicles departed in rthis simulation step
	departed_no=departed_no+curr_departed_no					#Add it in the Total vehicles departed

	#Lets get the positions of vehicles departed in this simulation step
	for i in curr_departed_list:
		#print str(l)+" "+  str(i) +" " + "(" +  str(abs(traci.vehicle.getPosition(i)[0])) + ", " + str(abs(traci.vehicle.getPosition(i)[1])) + ")"+"\n"
		vehicle_pos=vehicle_pos+ str(i) + "," + str(traci.vehicle.getPosition(i)[0])+","+str(traci.vehicle.getPosition(i)[1])
		vehicle_pos=vehicle_pos+'\n'
		l=l+1

	step += float(B[1])			#Counter increase
	#time.sleep(1)

#Write road positions in file
print ("Getting edges\n")
edge=traci.lane.getIDList()
roadpos=''
road_coordinates = []
i=0
#Loop for each edge
for j in edge:
	if j[0] != ':':
		i=i+1
		x1 = traci.lane.getShape(j)[0][0]
		y1 = traci.lane.getShape(j)[0][1]
		x2 = traci.lane.getShape(j)[1][0]
		y2 = traci.lane.getShape(j)[1][1]
		road_coordinates.append(((x1, y1), (x2, y2)))
#		roadpos=roadpos+str(i)+' '+'('+ str (abs(traci.lane.getShape(j)[0][0])*factor+88) +','+ str(abs(traci.lane.getShape(j)[0][1])*factor+88)+ ',' + str(abs(traci.lane.getShape(j)[1][0])*factor+88)+ ',' + str(abs(traci.lane.getShape(j)[1][1])*factor+88)+') \n'
		roadpos=roadpos+str(i)+' '+'('+ str(traci.lane.getShape(j)[0][0]) +','+ str(traci.lane.getShape(j)[0][1])+ ',' + str(traci.lane.getShape(j)[1][0])+ ',' + str(traci.lane.getShape(j)[1][1])+') \n'


######################### Graph Representation ###########################################################################
print('Generating SUMO image\n')
# Initialize lists for x and y coordinates
xl = []
yl = []

# Calculate the minimum and maximum x and y values
for coord in road_coordinates:
    xl.extend([coord[0][0], coord[1][0]])
    yl.extend([coord[0][1], coord[1][1]])

min_x, min_y, max_x, max_y = min(xl), min(yl), max(xl), max(yl)
min_x = math.floor(min_x)
min_y = math.floor(min_y)
min_x = min_x-20
min_y = min_y-20
if max_x > 0:
    max_x = math.ceil(max_x)
    max_x = max_x + 20
else:
    max_x = math.floor(max_x)
    max_x = max_x + 20
    
if max_y >0:
    max_y = math.ceil(max_y)
    max_y = max_y + 20
else:
    max_y = math.floor(max_y)
    max_y = max_y + 20
#get_junction()    Uncomment this for filling the junction over the area
get_traffic_lights(max_x,max_y)

# Plot road segments
for road_coord in road_coordinates:
    x_values, y_values = zip(*road_coord)
    plt.plot(x_values, y_values, 'k-')  # 'k-' for black lines without filling; customize the style as needed

# Set the x-axis and y-axis limits based on the full range of coordinates
if (max_y - min_y or max_x-min_x) >44: # To assess if it's a straight lane (for extending the image view).
    plt.xlim(min_x, max_x)
    plt.ylim(min_y, max_y)
    plt.gca().set_aspect('equal', adjustable='box') # To maintain an equal aspect ratio for the plot.
else:
    plt.xlim(min_x, max_x)
    if max_y - min_y > 44:
        min_x = min_x-500
        max_x = max_x+500
    else:
        min_y = min_y-500
        max_y = max_y+500
        
    plt.ylim(min_y, max_y)
    plt.gca().set_aspect('equal', adjustable='box')

# Calculate the minimum and maximum x and y values from road coordinates
for coord in road_coordinates:
    xl.extend([coord[0][0], coord[1][0]])
    yl.extend([coord[0][1], coord[1][1]])


# Remove axis labels and ticks
plt.xticks([])
plt.yticks([])

# Remove the title
plt.title('')

# Remove the axis
plt.axis('off')

# Show only the grid and road segments
plt.grid(True)

# Save the downsampled image
plt.savefig(imgpath, dpi=300, bbox_inches='tight', pad_inches=0,transparent=True)

########################### Graph Representation #######################################################################################
print("Writing file Ending part")
fgrid.write("OriginPosition=")
fgrid.write(str(Temp))
fgrid.write("\n")
fgrid.write("#Grid Origin\n")
fgrid.write("X min (m)= {}\n".format(min_x))
fgrid.write("Y min (m)= {}\n".format(min_y))
# Reading the Grid size coordinates
fgrid.write("#Grid Size\n")
fgrid.write("Width (m)= {}\n".format(max_x - min_x))
fgrid.write("Length (m)= {}\n".format(max_y - min_y))


#curr_junctions=traci.junction.getIDList()	#Junctions present currently
fw.write("#Number of device placed = ")		
fw.write(str(departed_no))					#Write the number of vehicles, Which is same as total vehicles departed
fw.write('\n')								
fw.write("#Device_Name,X-Pos,Y-Pos")	#Write "#Device_Number (X-Pos, Y-Pos)"
fw.write("\n")								
fw.write(vehicle_pos)						#Write Vehicle positions


print("Closing Connection")
traci.close()								#Close TCP Connection
print("Sumo closed")
print("Python is exiting")
fw.close()									#close files
fgrid.close()

config_read.close()
