std_msgs/Header header

# Global position of robot
float64 latitude
float64 longitude

# Global position of NED origin
float64 origin_latitude
float64 origin_longitude

# Robot pose in NED
amphix_ros_wrapper/NEDPose pose
amphix_ros_wrapper/NEDPose pose_variance
float64 altitude

# Robot body velocity
geometry_msgs/Vector3 body_velocity
geometry_msgs/Vector3 rpy_rate