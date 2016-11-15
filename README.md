1. This genom3 module is based on the ROS package called *find_object_2d* (http://wiki.ros.org/find_object_2d). **It is absolutely necessary to install it before proceeding.** 

  1. To install **find_object_2d**, open a new terminal and type: 

        ```
        sudo apt-get install ros-indigo-find-object-2d
        ```

2. Clone this repository and compile the genom3 module. To do so, enter the following commands:

        genom3 skeleton -l c++ -i objectdetection.gen
        ./bootstrap.sh
        mkdir build
        cd build
        ../configure --prefix=$ROBOTPKG_BASE --with-templates=ros/server,ros/client/c
        make
        make install

3. Go to the directory where find_object_2d was installed:

        roscd find_object_2d/

4. Create a file called `stereo.launch` in the *launch* directory where find_object_2d was installed and include the following:

        <launch>
        	<!-- Nodes -->
            <group ns="find_objects_2d_Left">
        	    <node name="find_object_2d" pkg="find_object_2d" type="find_object_2d" output="screen">
        		    <remap from="image" to="/stereo/left/image_rect_color"/>
        		    <param name="gui" value="false" type="bool"/>
        		    <param name="session_path" value="/home/USER_PATH/objectdetection/sessionOBJECTS.bin" type="str"/>
        	    </node>
            </group>
        
            <group ns="find_objects_2d_Right">
        	    <node name="find_object_2d" pkg="find_object_2d" type="find_object_2d" output="screen">
        		    <remap from="image" to="/stereo/right/image_rect_color"/>
        		    <param name="gui" value="false" type="bool"/>
        		    <param name="session_path" value="/home/USER_PATH/objectdetection/sessionOBJECTS.bin" type="str"/>
        	    </node>
            </group>
        </launch>

5. Modify `image` if needed and `session_path` accordingly to your setup (after finishing step 8).

6. Run ros

        roscore &

7. "Learn" all the objects that you want to detect.
  1. Check that your (mono) camera is publishing its output to a ros topic (sensor_msgs/Image message).
  2. Considering that your camera publishes to /stereo/left/image_rect_color, launch the **find_object_2d gui**, keeping in mind that the image topic has to be remapped accordingly to your settings.

        ```
        rosrun find_object_2d find_object_2d image:=/stereo/left/image_rect_color
        ```
   3. On the GUI, check that *nextObjID* under 'General' is set to 0.
   4. Go to  Edit->'Add object from scene...'
   5. When you are satisfied with the position of the object in the scene, click 'Take picture'.
   6. With the mouse, select the region of the object you want to detect (it's better that it contains a large number of keypoints to have a robust detection afterwards), click next and then end.
   7. It is recommended to follow steps 7.iv and 7.vi for each object from several angles and distances.

8. Save the session by going to file->save session. Check that the name has to be *.bin. This saves all the objects learned in previous steps.

9. Once all the objects have been 'learned' (preferably from different angles and distances), create one text file (*.txt) per object and include in each one the ID number captured by the GUI for each angle. For example, if the captured images for object named 'phone' go from 0 to 2 and for 'fan' go from 3 to 5, then:

        phone.txt should look like this:
        0
        1
        2
        
        fan.txt should look like this:
        3
        4
        5

    *Keep in mind that in each file, only one ID per line should be written and there should not be an empty line at the end.*

10. All the .txt files have to be saved **in the same folder**.

11. Edit stereo.launch so it complies with your setting:
  1. Check that the two image topics remaps are according to your left and right topics from your cameras.
  2. Change the **session_path** parameters for both nodes to match the path to your session file (*.bin) saved in step 8.

12. Launch the stereo.launch file. It will start two instances of the find_object_2d (without the GUI)

        roslaunch find_object_2d stereo.launch 

13. Run the genom3 module

        objectdetection-ros -b

14. Run genomix

        genomixd &

15. Start TCL

        eltclsh

14. Load objecdetection module

        package require genomix
        ::genomix::connect
        genomix1 load objectdetection

15. Connect the necessary ports (Keep in mind that CameraL and CameraR have to comply with your cameras's setting)

        ::objectdetection::connect_port RightCameraParameters /stereo/right/camera_info
        ::objectdetection::connect_port CameraL /stereo/left/image_rect_color
        ::objectdetection::connect_port inObjectsL find_objects_2d_Left/objectsStamped
        ::objectdetection::connect_port CameraR /stereo/right/image_rect_color
        ::objectdetection::connect_port inObjectsR find_objects_2d_Right/objectsStamped

18. Run the detection activity

        ::objectdetection::Start {objectPath /path_to/objects/textfiles/}
