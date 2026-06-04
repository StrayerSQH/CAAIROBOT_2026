#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import TimerAction
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    navigation_dir = get_package_share_directory('navigation')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    
    default_map_path = os.path.join(navigation_dir, 'maps', '2JL.yaml')
    default_params_path = os.path.join(navigation_dir, 'config', 'nav2_params.yaml')
    
    rviz_config_path = os.path.join(navigation_dir, 'config', 'navigation.rviz')
    if not os.path.exists(rviz_config_path):
        rviz_config_path = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    
    print("\n" + "="*60)
    print("🤖 测试：自动初始化 + 自动导航")
    print("="*60)
    print(f"🗺️  地图: {default_map_path}")
    print(f"⚙️  参数: {default_params_path}")
    print("="*60 + "\n")
    
    return LaunchDescription([
        # 启动Nav2
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')
            ),
            launch_arguments={
                'map': default_map_path,
                'use_sim_time': 'false',
                'params_file': default_params_path,
                'autostart': 'true',
            }.items(),
        ),
        
        # 启动RViz
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen',
        ),
        
        # 延迟8秒启动自动导航节点
        TimerAction(
            period=8.0,
            actions=[
                Node(
                    package='navigation',
                    executable='auto_navigator',
                    name='auto_navigator',
                    output='screen',
                    emulate_tty=True,
                )
            ]
        ),
    ])
