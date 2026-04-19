#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
import serial
import struct
from geometry_msgs.msg import Twist

# 定义消息格式常量
FRAME_HEAD = 0x52  # ROS发送给STM32的帧头
FRAME_LEN = 25     # 帧总长度
DATA_START = 1     # 数据起始位置
DATA_LEN = 24      # 数据长度 (6个float * 4字节)

class STM32Sender(Node):
    def __init__(self):
        super().__init__('stm32_sender_node')
        
        # 声明参数
        self.declare_parameter('port', '/dev/ttyUSB0')
        self.declare_parameter('baudrate', 230400)
        self.declare_parameter('send_rate', 10)  # 发送频率（Hz）
        
        port = self.get_parameter('port').value
        baudrate = self.get_parameter('baudrate').value
        send_rate = self.get_parameter('send_rate').value
        
        # 打开串口
        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=baudrate,
                timeout=0.1
            )
            self.get_logger().info(f"✅ 成功打开串口 {port} @ {baudrate}bps")
        except serial.SerialException as e:
            self.get_logger().error(f"❌ 无法打开串口 {port}: {e}")
            raise e
        
        # 在这里直接对变量进行赋值（测试用）
        # 修改这些值来改变发送的数据
        self.linear_x = 0.1   # 前进速度 (m/s)
        self.linear_y = 0.1
        self.linear_z = 0.0
        self.angular_x = 0.0
        self.angular_y = 0.0
        self.angular_z = 0.0  # 旋转速度 (rad/s)
        
        self.send_count = 0
        
        # 创建定时器用于定时发送
        self.timer = self.create_timer(1.0 / send_rate, self.timer_callback)
        
        # 打印启动信息
        self.get_logger().info("=" * 60)
        self.get_logger().info("📤 STM32发送节点（测试模式）")
        self.get_logger().info(f"📡 串口: {port} @ {baudrate}bps")
        self.get_logger().info(f"⏱️  发送频率: {send_rate} Hz")
        self.get_logger().info(f"📊 发送数据: 线性X={self.linear_x}, 角速Z={self.angular_z}")
        self.get_logger().info("=" * 60)
    
    def timer_callback(self):
        """定时器回调函数 - 定时发送数据"""
        try:
            # 构建发送帧
            frame = bytearray(FRAME_LEN)
            frame[0] = FRAME_HEAD
            
            # 打包6个float数据 (小端格式)
            struct.pack_into('<ffffff', frame, DATA_START,
                           self.linear_x,
                           self.linear_y,
                           self.linear_z,
                           self.angular_x,
                           self.angular_y,
                           self.angular_z)
            
            # 发送到STM32
            bytes_written = self.serial_port.write(frame)
            self.send_count += 1
            
            # 每10帧显示一次
            if self.send_count % 10 == 0:
                self.get_logger().info(f"已发送 {self.send_count} 帧 | 当前数据: X={self.linear_x:.2f}, Z角={self.angular_z:.2f}")
            
        except Exception as e:
            self.get_logger().error(f"❌ 发送失败: {e}")
    
    def destroy_node(self):
        """关闭串口"""
        if hasattr(self, 'serial_port') and self.serial_port.is_open:
            self.serial_port.close()
            self.get_logger().info("串口已关闭")
        
        self.get_logger().info(f"总计发送: {self.send_count} 帧")
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = STM32Sender()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        print("\n\n👋 程序被用户中断")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
