#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # ========== 获取包路径 ==========
    navigation_dir = get_package_share_directory('navigation')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    
    # ========== 配置文件路径 ==========
    default_map_path = os.path.join(navigation_dir, 'maps', 'map.yaml')
    default_params_path = os.path.join(navigation_dir, 'config', 'nav2_params.yaml')
    
    # 🔧 关键：使用你自己的 RViz 配置文件（保存你之前配置好的界面）
    # 如果你还没有保存，先创建这个文件
    rviz_config_path = os.path.join(navigation_dir, 'config', 'navigation.rviz')
    
    # 如果自己的 RViz 配置不存在，则使用 nav2 默认的
    if not os.path.exists(rviz_config_path):
        rviz_config_path = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    
    # ========== 定义可配置参数 ==========
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')  # 实体机器人用 false
    map_yaml_path = LaunchConfiguration('map', default=default_map_path)
    nav2_param_path = LaunchConfiguration('params_file', default=default_params_path)
    
    print("\n" + "="*60)
    print("🚀 启动 Nav2 导航系统（使用 nav2_bringup 方式）")
    print("="*60)
    print(f"🗺️  地图文件: {default_map_path}")
    print(f"⚙️  参数文件: {default_params_path}")
    print(f"🎨 RViz 配置: {rviz_config_path}")
    print("="*60 + "\n")
    
    return LaunchDescription([
        # ========== 声明启动参数 ==========
        DeclareLaunchArgument('use_sim_time', default_value='false', 
                              description='Use simulation (Gazebo) clock if true'),
        DeclareLaunchArgument('map', default_value=default_map_path, 
                              description='Full path to map file to load'),
        DeclareLaunchArgument('params_file', default_value=default_params_path, 
                              description='Full path to param file to load'),
        
        # ========== 核心：直接复用 nav2_bringup 的启动文件 ==========
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')
            ),
            launch_arguments={
                'map': map_yaml_path,
                'use_sim_time': use_sim_time,
                'params_file': nav2_param_path,
                'autostart': 'true',  # 自动启动所有节点
            }.items(),
        ),
        
        # ========== RViz2 可视化 ==========
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
            parameters=[{'use_sim_time': use_sim_time}],
            output='screen'),
    ])
