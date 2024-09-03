'''
Name: Tharshigan Vithiyananthan
Date: Apr.15,2024
Purpose: To read from data transmitted through UART, extract coordinate data, and to generate 3D visualization
'''

import serial
import numpy as np
import open3d as o3d

# Variables
dataCollect = 0
num_pts_collected = 0
num_planes = 0
pts_per_plane = 32
pts_labels_array = []

# Function to generate lines per YZ slice
def plot_YZ_Slice(num_pts, pts_per_plane, yz_slice_vertex):
    lines = []

    # Index offset "plane" for individual plane plotted
    for plane in range(0,num_pts,pts_per_plane):
        # "i" for each point in pcd
        for i in range(pts_per_plane):
            # If statement to plot last point to first point of plane
            if i == pts_per_plane-1:
                lines.append([yz_slice_vertex[plane+i], yz_slice_vertex[plane]])
            # Otherwise plot line from current point to next point
            else:
                lines.append([yz_slice_vertex[plane+i], yz_slice_vertex[plane+i+1]])

    return lines

# Function to generate lines to connect separate YZ slices together
def connect_Slices(num_planes, pts_per_plane, yz_slice_vertex, lines_array):
    # For loop to generate lines for each slice except for last one
    for plane in range(0,num_planes-1):
        for i in range(pts_per_plane):
            lines_array.append([yz_slice_vertex[(pts_per_plane*plane)+i], yz_slice_vertex[(pts_per_plane*(plane+1))+i]])

    return lines_array

if __name__ == "__main__":

    #   Currently set COM5 as serial port at 115.2kbps 8N1
    #   "Timeout" - allows the program to proceed if after a defined timeout period. The default = 0, which means wait forever.
    s = serial.Serial(port = 'COM5', baudrate = 115200, timeout = 10)

    print("Opening: " + s.name)

    # reset the buffers of the UART port to delete the remaining data in the buffers
    s.reset_output_buffer()
    s.reset_input_buffer()

    # Wait for user's signal to start the program
    input("Press Enter to start communication...")

    # Opening file in write mode
    f = open("tof_radar.xyz","w")

    # While loop to continuously read from port
    while(True):
        # Reading from UART port
        x = s.readline()

        print(x.decode())
        
        # If receive message indicating system on
        if x.decode() == "DAQ ON!\r\n":
            # Setting flag to begin writing to textfile
            dataCollect = 1
            print("Flag set\n")

        # Elif for when message indicating system off is received, exit loop
        elif x.decode() == "DAQ OFF!\r\n":
            print("Closing file\n")
            break

        # Elif for when other status messages are sent to UART to be ignored
        elif x.decode() == "" or x.decode().startswith("Error") or x.decode() == "Regaining lost data point!\r\n" or x.decode().startswith("RangeStatus") or x.decode().startswith("Faulty") or x.decode().startswith("Resetting"):
            print("Ignored error\n")
            continue

        else:
            if dataCollect:
                # Write to file if transmission started
                f.write(x.decode())
                print("Data collected\n")
        
    # Close file
    f.close()

    #close the port
    print("Closing: " + s.name)
    s.close()

    # Read the test data in from the file we created        
    print("Read in the prism point cloud data (pcd)")
    pcd = o3d.io.read_point_cloud("tof_radar.xyz", format="xyz")

    # Point cloud data array       
    print("The PCD array:")
    pts_array = np.asarray(pcd.points)
    print(pts_array)

    num_pts_collected = len(pts_array)
    num_planes = num_pts_collected//pts_per_plane

    #Lets see what our point cloud data looks like graphically       
    print("Lets visualize the PCD: (spawns seperate interactive window)")
    o3d.visualization.draw_geometries([pcd])

    # Add some lines to connect the vertices

    # Give each point a unique number
    for x in range(0,num_pts_collected):
        pts_labels_array.append([x])

    plot_lines = plot_YZ_Slice(num_pts_collected, pts_per_plane, pts_labels_array)

    plot_lines = connect_Slices(num_planes, pts_per_plane, pts_labels_array, plot_lines)

    # This line maps the lines to the 3d coordinate vertices
    line_set = o3d.geometry.LineSet(points=o3d.utility.Vector3dVector(pts_array),lines=o3d.utility.Vector2iVector(plot_lines))

    # View point cloud data with lines graphically       
    o3d.visualization.draw_geometries([line_set])     