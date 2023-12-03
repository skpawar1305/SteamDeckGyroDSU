import os
from subprocess import Popen, PIPE
import re
import math

import rclpy
from sensor_msgs.msg import Imu

UNITS_TO_DEG_PER_SEC = 16
ACC_1G = 16384


def read_current_output(process, node, publisher):
    while rclpy.ok():
        output = process.stdout.readline()
        if output == b'' and process.poll() is not None:
            break
        if output:
            output = str(output.decode().strip())
            output = output.split(",")
            if len(output) == 6:
                output[0] = re.search(r'-?\d+', output[0]).group()
                AccelAxisRightToLeft = (float(output[0]) / ACC_1G) * 9.8
                AccelAxisTopToBottom = (float(output[1]) / ACC_1G) * 9.8
                AccelAxisFrontToBack = (float(output[2]) / ACC_1G) * 9.8
                GyroAxisRightToLeft = float(output[3]) / UNITS_TO_DEG_PER_SEC
                GyroAxisTopToBottom = float(output[4]) / UNITS_TO_DEG_PER_SEC
                GyroAxisFrontToBack = float(output[5]) / UNITS_TO_DEG_PER_SEC

                msg = Imu()
                msg.header.frame_id = "deck"
                msg.header.stamp = node.get_clock().now().to_msg()

                # Adjusting the axis assignments for the desired orientation, topside pointing z, backside pointing x
                msg.angular_velocity.x = -math.radians(GyroAxisFrontToBack)
                msg.angular_velocity.y = -math.radians(GyroAxisRightToLeft)
                msg.angular_velocity.z = math.radians(GyroAxisTopToBottom)

                msg.linear_acceleration.x = -AccelAxisFrontToBack # Steam deck in rest, x should point down
                msg.linear_acceleration.y = -AccelAxisRightToLeft
                msg.linear_acceleration.z = AccelAxisTopToBottom

                publisher.publish(msg)


def create_process(cmd, node, publisher):
    process = Popen([cmd],
                    stdin=PIPE, stdout=PIPE, stderr=PIPE,
                    shell=True, preexec_fn=os.setsid)
    read_current_output(process, node, publisher)
    return process

def run_inference(audio_path, process):
    process.stdin.write("{}\n".format(audio_path).encode())
    process.stdin.flush()
    read_current_output(process)

rclpy.init()
node = rclpy.create_node("deck_gyro")
pub = node.create_publisher(Imu, "/deck/gyro_raw", 1)
inference_process = create_process("./gyro", node, pub)
node.destroy_node()
rclpy.shutdown()

# ros2 run imu_complementary_filter complementary_filter_node --ros-args -r /imu/data_raw:=/deck/gyro_raw -r /imu/data:=deck/gyro
