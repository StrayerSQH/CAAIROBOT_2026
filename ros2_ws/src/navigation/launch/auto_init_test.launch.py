#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, TimerAction
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # 获取包路径
    navigation_dir = get_package_share_directory('navigation')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    
    # 配置文件路径
    default_map_path = os.path.join(navigation_dir, 'maps', '2JL.yaml')
    default_params_path = os.path.join(navigation_dir, 'config', 'nav2_params.yaml')
    
    # RViz配置
    rviz_config_path = os.path.join(navigation_dir, 'config', 'navigation.rviz')
    if not os.path.exists(rviz_config_path):
        rviz_config_path = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    
    # 参数
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    map_yaml_path = LaunchConfiguration('map', default=default_map_path)
    nav2_param_path = LaunchConfiguration('params_file', default=default_params_path)
    
    print("\n" + "="*60)
    print("🧪 测试：自动初始化机器人位姿")
    print("="*60)
    print(f"🗺️  地图: {default_map_path}")
    print(f"⚙️  参数: {default_params_path}")
    print("="*60 + "\n")
    
    return LaunchDescription([
        # 声明参数
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('map', default_value=default_map_path),
        DeclareLaunchArgument('params_file', default_value=default_params_path),
        
        # 启动Nav2
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')
            ),
            launch_arguments={
                'map': map_yaml_path,
                'use_sim_time': use_sim_time,
                'params_file': nav2_param_path,
                'autostart': 'true',
            }.items(),
        ),
        
        # 启动RViz（可视化）
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
            parameters=[{'use_sim_time': use_sim_time}],
            output='screen',
        ),
        
        # 延迟5秒启动自动初始化节点（等待Nav2完全启动）
        TimerAction(
            period=5.0,
            actions=[
                Node(
                    package='navigation',
                    executable='auto_initializer',
                    name='auto_initializer',
                    output='screen',
                    emulate_tty=True,
                )
            ]
        ),
    ])
