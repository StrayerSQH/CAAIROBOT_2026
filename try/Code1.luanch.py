#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, LogInfo
from launch.substitutions import LaunchConfiguration
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
    print("🚀 启动 Nav2 导航系统（完整版）")
    print("="*60)
    print(f"🗺️  地图文件: {default_map_path}")
    print(f"⚙️  参数文件: {default_params_path}")
    print("="*60 + "\n")
    
    return LaunchDescription([
        # ========== 声明启动参数 ==========
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('map', default_value=default_map_path),
        DeclareLaunchArgument('params_file', default_value=default_params_path),
        
        # ========== 1. 地图服务器 ==========
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
        
        # ========== 2. AMCL 定位 ==========
        Node(
            package='nav2_amcl',
            executable='amcl',
            name='amcl',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 3. 控制器服务器（局部规划） ==========
        Node(
            package='nav2_controller',
            executable='controller_server',
            name='controller_server',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 4. 规划器服务器（全局规划） ==========
        Node(
            package='nav2_planner',
            executable='planner_server',
            name='planner_server',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 5. 恢复服务器 ==========
        Node(
            package='nav2_behaviors',
            executable='behavior_server',
            name='behavior_server',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 6. BT 导航器 ==========
        Node(
            package='nav2_bt_navigator',
            executable='bt_navigator',
            name='bt_navigator',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 7. 速度平滑器 ==========
        Node(
            package='nav2_velocity_smoother',
            executable='velocity_smoother',
            name='velocity_smoother',
            output='screen',
            parameters=[params_file, {'use_sim_time': use_sim_time}]
        ),
        
        # ========== 8. 生命周期管理器（自动激活所有节点） ==========
        Node(
            package='nav2_lifecycle_manager',
            executable='lifecycle_manager',
            name='lifecycle_manager_navigation',
            output='screen',
            parameters=[{
                'use_sim_time': use_sim_time,
                'autostart': True,
                'node_names': [
                    'map_server',
                    'amcl',
                    'controller_server',
                    'planner_server',
                    'behavior_server',
                    'bt_navigator',
                    'velocity_smoother'
                ]
            }]
        ),
        
        # ========== 9. Rviz2 可视化 ==========
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen',
            parameters=[{'use_sim_time': use_sim_time}]
        ),
        
        # ========== 10. 备用激活脚本（确保节点被激活） ==========
        ExecuteProcess(
            cmd=[
                'bash', '-c',
                'sleep 5 && '
                'ros2 lifecycle set /map_server configure && '
                'ros2 lifecycle set /map_server activate && '
                'ros2 lifecycle set /amcl configure && '
                'ros2 lifecycle set /amcl activate && '
                'ros2 lifecycle set /controller_server configure && '
                'ros2 lifecycle set /controller_server activate && '
                'ros2 lifecycle set /planner_server configure && '
                'ros2 lifecycle set /planner_server activate && '
                'ros2 lifecycle set /behavior_server configure && '
                'ros2 lifecycle set /behavior_server activate && '
                'ros2 lifecycle set /bt_navigator configure && '
                'ros2 lifecycle set /bt_navigator activate && '
                'ros2 lifecycle set /velocity_smoother configure && '
                'ros2 lifecycle set /velocity_smoother activate && '
                'echo "✅ 所有导航节点已激活"'
            ],
            output='screen'
        ),
        
        # ========== 11. 打印使用说明 ==========
        ExecuteProcess(
            cmd=[
                'bash', '-c',
                'sleep 8 && '
                'echo "" && '
                'echo "==========================================" && '
                'echo "📌 导航系统已就绪！" && '
                'echo "==========================================" && '
                'echo "  1. 在 Rviz2 中确认 Fixed Frame = map" && '
                'echo "  2. 点击 2D Pose Estimate 设置机器人位置" && '
                'echo "  3. 点击 2D Goal Pose 设置目标点" && '
                'echo "==========================================" && '
                'echo "  📊 可用话题:" && '
                'echo "     - /global_costmap/costmap (全局代价地图)" && '
                'echo "     - /local_costmap/costmap (局部代价地图)" && '
                'echo "     - /plan (全局规划路径)" && '
                'echo "     - /local_plan (局部规划路径)" && '
                'echo "==========================================" && '
                'echo ""'
            ],
            output='screen'
        ),
    ])
