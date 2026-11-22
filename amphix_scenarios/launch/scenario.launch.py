from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    scenarios_share = FindPackageShare('amphix_scenarios')

    scenario_arg = DeclareLaunchArgument(
        'scenario',
        default_value='bluerov2_tank.scn',
        description='Scenario file name in amphix_scenarios/scenarios'
    )
    simulation_rate_arg = DeclareLaunchArgument('simulation_rate', default_value='100.0')
    window_res_x_arg = DeclareLaunchArgument('window_res_x', default_value='1280')
    window_res_y_arg = DeclareLaunchArgument('window_res_y', default_value='720')
    rendering_quality_arg = DeclareLaunchArgument('rendering_quality', default_value='high')

    simulator = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution([
            FindPackageShare('amphix_ros_wrapper'), 'launch', 'amphix_simulator.launch.py'
        ])),
        launch_arguments={
            'simulation_data': PathJoinSubstitution([scenarios_share, 'data']),
            'scenario_desc': PathJoinSubstitution([scenarios_share, 'scenarios', LaunchConfiguration('scenario')]),
            'simulation_rate': LaunchConfiguration('simulation_rate'),
            'window_res_x': LaunchConfiguration('window_res_x'),
            'window_res_y': LaunchConfiguration('window_res_y'),
            'rendering_quality': LaunchConfiguration('rendering_quality'),
        }.items()
    )

    return LaunchDescription([
        scenario_arg,
        simulation_rate_arg,
        window_res_x_arg,
        window_res_y_arg,
        rendering_quality_arg,
        simulator,
    ])
