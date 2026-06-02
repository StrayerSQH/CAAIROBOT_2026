#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
import serial
import struct
import threading
import math
from geometry_msgs.msg import Twist, TransformStamped
from nav_msgs.msg import Odometry
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy
from tf2_ros import TransformBroadcaster
import time

# 定义消息格式常量
FRAME_HEAD = 0x54
FRAME_LEN = 25
DATA_START = 1
DATA_LEN = 24

class STM32OdomReceiver(Node):
    def __init__(self):
        super().__init__('stm32_odom_receiver_node')
        
        # 参数设置
        self.declare_parameter('port', '/dev/ttyUSB0')
        self.declare_parameter('baudrate', 230400)
        self.declare_parameter('wheel_base', 0.5)
        self.declare_parameter('publish_odom', True)
        self.declare_parameter('odom_frame', 'odom')
        self.declare_parameter('base_frame', 'base_link')  # 保持 base_link 用于建图
        
        port = self.get_parameter('port').value
        baudrate = self.get_parameter('baudrate').value
        self.wheel_base = self.get_parameter('wheel_base').value
        self.publish_odom = self.get_parameter('publish_odom').value
        self.odom_frame = self.get_parameter('odom_frame').value
        self.base_frame = self.get_parameter('base_frame').value
        
        # 创建QoS配置
        qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )
        
        # 创建发布者
        self.vehicle_info_pub = self.create_publisher(Twist, 'Vehicle_Receiver_Topic', qos)
        self.odom_pub = self.create_publisher(Odometry, 'odom', qos)
        
        # 创建TF广播器
        self.tf_broadcaster = TransformBroadcaster(self)
        
        # 打开串口
        try:
            self.serial_port = serial.Serial(port=port, baudrate=baudrate, timeout=0.01)
            self.get_logger().info(f"✅ 串口 {port} 已打开 @ {baudrate}bps")
        except Exception as e:
            self.get_logger().error(f"❌ 打开串口失败: {e}")
            raise e
        
        # 缓冲区
        self.buffer = bytearray()
        self.frame_count = 0
        self.error_count = 0
        
        # 里程计状态变量
        self.x = 0.0
        self.y = 0.0
        self.theta = 0.0
        self.last_time = None
        
        # 启动串口读取线程
        self.running = True
        self.serial_thread = threading.Thread(target=self.serial_reader_thread)
        self.serial_thread.daemon = True
        self.serial_thread.start()
        
        # 启动里程计计算定时器
        self.create_timer(0.02, self.publish_odom_transform)
        
        # 打印启动信息
        self.get_logger().info("=" * 60)
        self.get_logger().info("🚀 STM32里程计接收节点已启动")
        self.get_logger().info(f"📡 串口: {port} @ {baudrate}bps")
        self.get_logger().info(f"📏 轮距: {self.wheel_base}m")
        self.get_logger().info(f"🧭 里程计: {self.odom_frame} -> {self.base_frame}")
        self.get_logger().info("=" * 60)
        self.print_header()
    
    def print_header(self):
        """打印表头"""
        print("\n" + "=" * 120)
        print(f"{'帧数':>6} | {'线性 X':>10} | {'线性 Y':>10} | {'线性 Z':>10} | "
              f"{'角速 X':>10} | {'角速 Y':>10} | {'角速 Z':>10} | "
              f"{'位置 X':>8} | {'位置 Y':>8} | {'朝向':>8}")
        print("=" * 120)
    
    def serial_reader_thread(self):
        """串口读取线程"""
        while self.running and rclpy.ok():
            try:
                if self.serial_port.in_waiting > 0:
                    data = self.serial_port.read(self.serial_port.in_waiting)
                    self.buffer.extend(data)
                    
                    while len(self.buffer) >= FRAME_LEN:
                        if self.buffer[0] == FRAME_HEAD:
                            frame = self.buffer[:FRAME_LEN]
                            self.process_one_frame(frame)
                            self.buffer = self.buffer[FRAME_LEN:]
                        else:
                            self.buffer.pop(0)
                            self.error_count += 1
                else:
                    time.sleep(0.001)
            except Exception as e:
                self.get_logger().error(f"读取串口数据错误: {e}")
                time.sleep(0.1)
    
    def process_one_frame(self, frame):
        """处理一条完整的数据帧"""
        try:
            float_data = struct.unpack('<ffffff', frame[DATA_START:DATA_START + DATA_LEN])
            
            twist_msg = Twist()
            twist_msg.linear.x = float_data[0]
            twist_msg.linear.y = float_data[1]
            twist_msg.linear.z = float_data[2]
            twist_msg.angular.x = float_data[3]
            twist_msg.angular.y = float_data[4]
            twist_msg.angular.z = float_data[5]
            
            # 计算里程计位置
            self.update_odometry(twist_msg)
            
            # 显示数据
            self.frame_count += 1
            print(f"{self.frame_count:6d} | "
                  f"{twist_msg.linear.x:10.3f} | "
                  f"{twist_msg.linear.y:10.3f} | "
                  f"{twist_msg.linear.z:10.3f} | "
                  f"{twist_msg.angular.x:10.3f} | "
                  f"{twist_msg.angular.y:10.3f} | "
                  f"{twist_msg.angular.z:10.3f} | "
                  f"{self.x:8.3f} | {self.y:8.3f} | {self.theta:8.3f}")
            
            # 发布原始Twist消息
            self.vehicle_info_pub.publish(twist_msg)
            
        except struct.error as e:
            self.get_logger().debug(f"数据解析失败: {e}")
            self.error_count += 1
        except Exception as e:
            self.get_logger().debug(f"未知错误: {e}")
            self.error_count += 1
    
    def update_odometry(self, twist):
        """根据Twist更新里程计位置"""
        current_time = self.get_clock().now()
        
        if self.last_time is None:
            self.last_time = current_time
            return
        
        dt = (current_time - self.last_time).nanoseconds / 1e9
        
        if dt <= 0 or dt > 0.1:
            self.last_time = current_time
            return
        
        v = twist.linear.x
        omega = twist.angular.z
        
        if abs(omega) < 1e-6:
            delta_x = v * dt * math.cos(self.theta)
            delta_y = v * dt * math.sin(self.theta)
            delta_theta = 0.0
        else:
            delta_theta = omega * dt
            delta_x = (v / omega) * (math.sin(self.theta + delta_theta) - math.sin(self.theta))
            delta_y = (v / omega) * (math.cos(self.theta) - math.cos(self.theta + delta_theta))
        
        self.x += delta_x
        self.y += delta_y
        self.theta += delta_theta
        self.theta = math.atan2(math.sin(self.theta), math.cos(self.theta))
        
        self.last_time = current_time
    
    def publish_odom_transform(self):
        """发布odom变换和里程计消息 - 同时发布到 base_link 和 base_footprint"""
        if self.last_time is None:
            return
        
        current_time = self.get_clock().now()
        
        # ========== 1. 发布 odom -> base_link（用于建图）==========
        t_link = TransformStamped()
        t_link.header.stamp = current_time.to_msg()
        t_link.header.frame_id = self.odom_frame
        t_link.child_frame_id = 'base_link'
        
        t_link.transform.translation.x = self.x
        t_link.transform.translation.y = self.y
        t_link.transform.translation.z = 0.0
        t_link.transform.rotation.z = math.sin(self.theta / 2.0)
        t_link.transform.rotation.w = math.cos(self.theta / 2.0)
        
        self.tf_broadcaster.sendTransform(t_link)
        
        # ========== 2. 发布 odom -> base_footprint（用于 Nav2）==========
        t_footprint = TransformStamped()
        t_footprint.header.stamp = current_time.to_msg()
        t_footprint.header.frame_id = self.odom_frame
        t_footprint.child_frame_id = 'base_footprint'
        
        t_footprint.transform.translation.x = self.x
        t_footprint.transform.translation.y = self.y
        t_footprint.transform.translation.z = 0.0
        t_footprint.transform.rotation.z = math.sin(self.theta / 2.0)
        t_footprint.transform.rotation.w = math.cos(self.theta / 2.0)
        
        self.tf_broadcaster.sendTransform(t_footprint)
        
        # ========== 3. 发布 Odometry 消息 ==========
        if self.publish_odom:
            odom_msg = Odometry()
            odom_msg.header.stamp = current_time.to_msg()
            odom_msg.header.frame_id = self.odom_frame
            odom_msg.child_frame_id = 'base_link'
            
            odom_msg.pose.pose.position.x = self.x
            odom_msg.pose.pose.position.y = self.y
            odom_msg.pose.pose.position.z = 0.0
            odom_msg.pose.pose.orientation.z = math.sin(self.theta / 2.0)
            odom_msg.pose.pose.orientation.w = math.cos(self.theta / 2.0)
            
            self.odom_pub.publish(odom_msg)
    
    def destroy_node(self):
        """关闭串口和线程"""
        self.running = False
        if hasattr(self, 'serial_thread') and self.serial_thread.is_alive():
            self.serial_thread.join(timeout=1.0)
        
        if hasattr(self, 'serial_port') and self.serial_port.is_open:
            self.serial_port.close()
            self.get_logger().info("串口已关闭")
        
        print("\n" + "=" * 60)
        print("📊 最终统计")
        print(f"总接收帧数: {self.frame_count}")
        print(f"错误帧数: {self.error_count}")
        if self.frame_count > 0:
            print(f"错误率: {(self.error_count/self.frame_count*100):.2f}%")
        print(f"最终位置: X={self.x:.3f}m, Y={self.y:.3f}m, Theta={self.theta:.3f}rad")
        print("=" * 60)
        
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = STM32OdomReceiver()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        print("\n\n👋 程序被用户中断")
    except Exception as e:
        print(f"❌ 程序错误: {e}")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
