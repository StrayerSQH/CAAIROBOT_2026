#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Nav2 导航独立启动文件
功能：启动雷达、里程计、TF变换、AMCL、导航栈，并自动加载地图
"""

import os
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    LogInfo
)
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # ========== 参数定义 ==========
    # 地图文件路径
    map_file = DeclareLaunchArgument(
        'map',
        default_value='/home/strayer/ros2_ws/map.yaml',
        description='Full path to map yaml file'
    )
    
    # 雷达串口
    lidar_port = DeclareLaunchArgument(
        'lidar_port',
        default_value='/dev/ttyACM0',
        description='Serial port for LiDAR'
    )
    
    # 里程计串口
    odom_port = DeclareLaunchArgument(
        'odom_port',
        default_value='/dev/ttyUSB0',
        description='Serial port for STM32 odometry'
    )
    
    # 是否使用仿真时间
    use_sim_time = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation time'
    )
    
    # ========== 1. 雷达驱动节点 ==========
    ldlidar_node = Node(
        package='ldlidar_sl_ros2',
        executable='ldlidar_sl_ros2_node',
        name='ldlidar_publisher',
        output='screen',
        parameters=[
            {'product_name': 'LDLiDAR_LD14P'},
            {'laser_scan_topic_name': 'scan'},
            {'point_cloud_2d_topic_name': 'pointcloud2d'},
            {'frame_id': 'base_laser'},
            {'port_name': LaunchConfiguration('lidar_port')},
            {'serial_baudrate': 230400},
            {'laser_scan_dir': True},
            {'enable_angle_crop_func': False},
            {'angle_crop_min': 135.0},
            {'angle_crop_max': 225.0}
        ]
    )
    
    # ========== 2. 静态 TF：base_link -> base_laser ==========
    base_link_to_laser_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_base_laser',
        arguments=['0', '0', '0.18', '0', '0', '0', 'base_link', 'base_laser'],
        output='screen'
    )
    
    # ========== 3. 静态 TF：base_footprint -> base_link ==========
    base_footprint_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_footprint_to_base_link',
        arguments=['0', '0', '0', '0', '0', '0', 'base_footprint', 'base_link'],
        output='screen'
    )
    
    # ========== 4. 里程计节点 ==========
    odom_node = Node(
        package='serial_communication',
        executable='stm32_reciever',
        name='stm32_odom_receiver',
        output='screen',
        parameters=[
            {'port': LaunchConfiguration('odom_port')},
            {'baudrate': 230400},
            {'publish_odom': True},
            {'odom_frame': 'odom'},
            {'base_frame': 'base_link'}
        ]
    )
    
    # ========== 5. AMCL 定位节点 ==========
    amcl_node = Node(
        package='nav2_amcl',
        executable='amcl',
        name='amcl',
        output='screen',
        parameters=[{
            'use_sim_time': LaunchConfiguration('use_sim_time'),
            'map_topic': '/map',
            'odom_frame_id': 'odom',
            'base_frame_id': 'base_link',
            'global_frame_id': 'map',
            'set_initial_pose': True,
            'initial_pose.x': 0.0,
            'initial_pose.y': 0.0,
            'initial_pose.yaw': 0.0
        }]
    )
    
    # ========== 6. 地图服务器 ==========
    map_server_node = Node(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        output='screen',
        parameters=[{
            'yaml_filename': LaunchConfiguration('map'),
            'use_sim_time': LaunchConfiguration('use_sim_time')
        }]
    )
    
    # ========== 7. 生命周期管理器 ==========
    lifecycle_manager_node = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_navigation',
        output='screen',
        parameters=[{
            'use_sim_time': LaunchConfiguration('use_sim_time'),
            'autostart': True,
            'node_names': ['map_server', 'amcl']
        }]
    )
    
    # ========== 8. Rviz2 可视化 ==========
    rviz_config = os.path.join(
        os.path.dirname(__file__), '..', 'rviz', 'nav2_config.rviz'
    )
    
    rviz2_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config],
        output='screen'
    )
    
    # ========== 9. 打印启动信息 ==========
    startup_message = LogInfo(
        msg=[
            '\n' + '='*60 + '\n',
            '🚀 Nav2 导航系统启动中...\n',
            '='*60 + '\n',
            '📡 雷达串口: ', LaunchConfiguration('lidar_port'), '\n',
            '🔗 里程计串口: ', LaunchConfiguration('odom_port'), '\n',
            '🗺️  地图文件: ', LaunchConfiguration('map'), '\n',
            '='*60 + '\n',
            '📌 下一步操作:\n',
            '   1. 在 Rviz2 中确认 Fixed Frame = map\n',
            '   2. 点击 "2D Pose Estimate" 设置机器人初始位置\n',
            '   3. 点击 "2D Goal Pose" 设置导航目标点\n',
            '='*60 + '\n'
        ]
    )
    
    # ========== 返回启动描述 ==========
    ld = LaunchDescription()
    
    # 添加参数声明
    ld.add_action(map_file)
    ld.add_action(lidar_port)
    ld.add_action(odom_port)
    ld.add_action(use_sim_time)
    
    # 添加启动信息
    ld.add_action(startup_message)
    
    # 添加节点
    ld.add_action(ldlidar_node)
    ld.add_action(base_link_to_laser_tf)
    ld.add_action(base_footprint_tf)
    ld.add_action(odom_node)
    ld.add_action(map_server_node)
    ld.add_action(amcl_node)
    ld.add_action(lifecycle_manager_node)
    ld.add_action(rviz2_node)
    
    return ld
