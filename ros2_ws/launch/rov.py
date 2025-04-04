from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pid_config = os.path.join(
      'config',
      'pid.yaml'
    )

    return LaunchDescription([
        Node(
            package='hardware_integration',
            executable='dvl_driver',
        ),
        Node(
            package='realsense2_camera',
            executable='realsense2_camera_node',
            remappings=[
                ('/camera/camera/depth/image_rect_raw', '/camera/depth'),
                ('/camera/camera/color/image_raw', '/camera/rgb'),
            ],
            parameters=[{
                'unite_imu_method': 2,
                'enable_gyro': 'true',
                }]
        ),
       Node(
            package='navigation',
            executable='dead_reckoning',
        )   ,
        Node(
            package='sensor_processing',
            executable='pid',
            name='yaw_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/yaw/target'),
                ('/PID/XXX/actual', '/PID/yaw/actual'),
                ('/PID_correction/XXX', '/PID_correction/yaw')
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='pitch_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/pitch/target'),
                ('/PID/XXX/actual', '/PID/pitch/actual'),
                ('/PID_correction/XXX', '/PID_correction/pitch')
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='roll_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/roll/target'),
                ('/PID/XXX/actual', '/PID/roll/actual'),
                ('/PID_correction/XXX', '/PID_correction/roll')
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='surge_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/surge/target'),
                ('/PID/XXX/actual', '/PID/surge/actual'),
                ('/PID_correction/XXX', '/PID_correction/surge')
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='sway_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/sway/target'),
                ('/PID/XXX/actual', '/PID/sway/actual'),
                ('/PID_correction/XXX', '/PID_correction/sway')
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='heave_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/heave/target'),
                ('/PID/XXX/actual', '/PID/heave/actual'),
                ('/PID_correction/XXX', '/PID_correction/heave')
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='x_translation_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/translation/target'),#goal translation is always 0 for now, so each translational PID controller has the same target
                ('/PID/XXX/actual', '/PID/x_translation/actual'),
                ('/PID_correction/XXX', '/PID/surge/target')#cascading
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='y_translation_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/translation/target'),
                ('/PID/XXX/actual', '/PID/y_translation/actual'),
                ('/PID_correction/XXX', '/PID/sway/target')#cascading
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid',
            name='z_translation_pid_controller',
            remappings=[
                ('/PID/XXX/target', '/PID/translation/target'),
                ('/PID/XXX/actual', '/PID/z_translation/actual'),
                ('/PID_correction/XXX', '/PID/heave/target')#cascading
            ],
            parameters=[pid_config]
        ),
        Node(
            package='sensor_processing',
            executable='pid_combiner',
        ),
        Node(
            package='hardware_integration',
            executable='serial_output',
        ),
        Node(
            package='teleop0',
            executable='teleop_node',
        ),
        Node(
            package='object_detection',
            executable='lid_detector',
        ),
         Node(
            package='object_detection',
            executable='data_recorder',
        ),       ])

