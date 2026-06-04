#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
简单的 SLAM Toolbox Python 启动脚本
"""

import subprocess
import sys
import time
import signal

def signal_handler(sig, frame):
    print("\n\n🛑 正在停止 SLAM...")
    sys.exit(0)

def main():
    # 注册 Ctrl+C 信号处理
    signal.signal(signal.SIGINT, signal_handler)
    
    print("="*60)
    print("🚀 启动 SLAM Toolbox")
    print("="*60)
    
    # SLAM Toolbox 参数
    slam_params = [
        'ros2', 'run', 'slam_toolbox', 'async_slam_toolbox_node',
        '--ros-args',
        '-p', 'qos_overrides.scan.subscription.reliability:=best_effort',
        '-p', 'resolution:=0.025',
        '-p', 'scan_topic:=/scan',
        '-p', 'mode:=mapping',
        '-p', 'map_frame:=map',
        '-p', 'odom_frame:=odom',
        '-p', 'base_frame:=base_link',
        '-p', 'map_update_interval:=0.1',
        '-p', 'minimum_travel_distance:=0.00',
        '-p', 'minimum_travel_heading:=0.00',
        '-p', 'transform_timeout:=0.5',
        '-p', 'max_laser_range:=12.0'
    ]
    
    print("📡 等待雷达和里程计数据...")
    print("🗺️  启动参数:")
    print("   - 地图更新间隔: 0.2秒")
    print("   - 最小移动距离: 0.05米")
    print("   - 最小转动角度: 0.05弧度")
    print("-"*60)
    
    try:
        # 启动 SLAM 进程
        process = subprocess.Popen(
            slam_params,
            stdout=sys.stdout,
            stderr=sys.stderr
        )
        
        print("✅ SLAM Toolbox 已启动！")
        print("💡 提示: 按 Ctrl+C 停止\n")
        
        # 等待进程结束
        process.wait()
        
    except KeyboardInterrupt:
        print("\n\n🛑 用户中断")
        process.terminate()
        process.wait()
    except Exception as e:
        print(f"❌ 错误: {e}")
        return 1
    
    print("👋 SLAM 已停止")
    return 0

if __name__ == '__main__':
    sys.exit(main())
