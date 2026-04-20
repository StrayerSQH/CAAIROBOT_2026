#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # ========== 获取包路径 ==========
    navigation_dir = FindPackageShare('navigation').find('navigation')
    nav2_bringup_dir = FindPackageShare('nav2_bringup').find('nav2_bringup')
    
    # ========== 配置文件路径 ==========
    default_map_path = os.path.join(navigation_dir, 'maps', 'map.yaml')
    default_params_path = os.path.join(navigation_dir, 'config', 'nav2_params.yaml')
    rviz_config_path = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    
    # ========== 定义可配置参数 ==========
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    map_path = LaunchConfiguration('map', default=default_map_path)
    params_file = LaunchConfiguration('params_file', default=default_params_path)
    
    print("\n" + "="*60)
    print("🚀 启动 Nav2 导航系统")
    print("="*60)
    print(f"🗺️  地图文件: {default_map_path}")
    print(f"⚙️  参数文件: {default_params_path}")
    print("="*60 + "\n")
    
    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('map', default_value=default_map_path),
        DeclareLaunchArgument('params_file', default_value=default_params_path),
        
        # ========== 启动地图服务器 ==========
        Node(
            package='nav2_map_server',
            executable='map_server',
            name='map_server',
            output='screen',
            parameters=[{
                'yaml_filename': map_path,
                'use_sim_time': use_sim_time
            }]
        ),
        
        # ========== 启动 AMCL（关键！） ==========
        Node(
            package='nav2_amcl',
            executable='amcl',
            name='amcl',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 启动生命周期管理器 ==========
        Node(
            package='nav2_lifecycle_manager',
            executable='lifecycle_manager',
            name='lifecycle_manager_navigation',
            output='screen',
            parameters=[{
                'use_sim_time': use_sim_time,
                'autostart': True,
                'node_names': ['map_server', 'amcl']
            }]
        ),
        
        # ========== 启动 Rviz2 ==========
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen',
            parameters=[{'use_sim_time': use_sim_time}]
        ),
    ])
