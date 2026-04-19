#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
完整的 SLAM 建图启动文件
包含：雷达驱动 + 里程计节点 + SLAM Toolbox + Rviz2
"""

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # ========== 1. 雷达驱动节点 ==========
    ldlidar_node = Node(
        package='ldlidar_sl_ros2',
        executable='ldlidar_sl_ros2_node',
        name='ldlidar_publisher_ld14',
        output='screen',
        parameters=[
            {'product_name': 'LDLiDAR_LD14P'},
            {'laser_scan_topic_name': 'scan'},
            {'point_cloud_2d_topic_name': 'pointcloud2d'},
            {'frame_id': 'base_laser'},
            {'port_name': '/dev/ttyACM0'},
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
        arguments=['0', '0', '0.18', '0', '0', '0', 'base_link', 'base_laser']
    )
    
    # ========== 3. 里程计节点（从 STM32 接收数据） ==========
    # 如果你有 stm32_odom_receiver.py，取消下面的注释
    # odom_node = Node(
    #     package='serial_communication',
    #     executable='stm32_odom_receiver',
    #     name='stm32_odom_receiver',
    #     output='screen',
    #     parameters=[
    #         {'port': '/dev/ttyUSB0'},
    #         {'baudrate': 230400},
    #         {'publish_odom': True},
    #         {'odom_frame': 'odom'},
    #         {'base_frame': 'base_link'}
    #     ]
    # )
    
    # 如果没有真实的里程计，使用假里程计
    fake_odom_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='fake_odom',
        arguments=['0', '0', '0', '0', '0', '0', 'odom', 'base_link'],
        output='screen'
    )
    
    # ========== 4. SLAM Toolbox 节点 ==========
    # SLAM 参数配置
    slam_params = {
        # === 基本参数 ===
        'use_sim_time': False,
        'scan_topic': '/scan',
        'odom_frame': 'odom',
        'map_frame': 'map',
        'base_frame': 'base_link',
        'mode': 'mapping',
        
        # === QoS 设置（关键！） ===
        'qos_overrides': {
            'scan': {
                'subscription': {
                    'reliability': 'best_effort'
                }
            }
        },
        
        # === 地图更新参数 ===
        'map_update_interval': 0.2,           # 地图更新间隔（秒）
        'resolution': 0.05,                   # 地图分辨率（米/像素）
        
        # === 处理频率参数 ===
        'throttle_scans': 1,                  # 每1帧处理一次
        'minimum_time_interval': 0.05,        # 最小时间间隔
        'minimum_travel_distance': 0.05,      # 移动超过5cm才处理
        'minimum_travel_heading': 0.05,       # 转动超过0.05弧度才处理
        
        # === 扫描匹配参数 ===
        'max_laser_range': 12.0,              # 激光最大范围（米）
        'max_iterations': 100,                # 最大迭代次数
        'transform_timeout': 0.5,             # TF 变换超时
        'tf_buffer_duration': 30.0,           # TF 缓冲时间
        
        # === 闭环检测参数 ===
        'loop_search_maximum_distance': 5.0,  # 闭环搜索最大距离
        'loop_match_minimum_chain_size': 10,  # 最小闭环链长度
        'loop_match_maximum_variance': 0.05,  # 最大匹配方差
        
        # === 性能参数 ===
        'ceres_num_threads': 4,               # Ceres 求解器线程数
        'ceres_num_linear_solver_threads': 4,
        
        # === 调试参数 ===
        'debug_logging': False,
        'enable_interactive_mode': False
    }
    
    slam_toolbox_node = Node(
        package='slam_toolbox',
        executable='sync_slam_toolbox_node',  # 使用同步模式
        name='slam_toolbox',
        output='screen',
        parameters=[slam_params],
        arguments=['--ros-args', '--log-level', 'info']
    )
    
    # ========== 5. Rviz2 可视化节点 ==========
    # 使用默认的 SLAM Toolbox Rviz 配置
    rviz_config = os.path.join(
        get_package_share_directory('slam_toolbox'),
        'config',
        'slam_toolbox_default.rviz'
    )
    
    # 如果使用自己的 Rviz 配置，取消注释下面这行
    # rviz_config = os.path.join(
    #     get_package_share_directory('ldlidar_sl_ros2'),
    #     'rviz2',
    #     'ldlidar.rviz'
    # )
    
    rviz2_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config],
        output='screen'
    )
    
    # ========== 6. 打印启动信息 ==========
    print("\n" + "="*60)
    print("🚀 SLAM 建图系统启动中...")
    print("="*60)
    print("📡 雷达驱动: /dev/ttyACM0 @ 230400bps")
    print("🧭 里程计: odom -> base_link (静态变换)")
    print("🗺️  SLAM: 同步模式，地图更新间隔 0.2秒")
    print("🎨 Rviz2: 正在启动...")
    print("="*60 + "\n")
    
    # ========== 返回启动描述 ==========
    ld = LaunchDescription()
    
    # 添加所有节点
    ld.add_action(ldlidar_node)
    ld.add_action(base_link_to_laser_tf)
    ld.add_action(fake_odom_node)      # 使用假里程计
    # ld.add_action(odom_node)          # 如果有真实里程计，取消注释这行，注释上面那行
    ld.add_action(slam_toolbox_node)
    ld.add_action(rviz2_node)
    
    return ld
