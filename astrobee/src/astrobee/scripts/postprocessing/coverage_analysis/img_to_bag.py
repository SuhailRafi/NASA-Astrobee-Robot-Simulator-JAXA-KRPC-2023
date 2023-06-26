#!/usr/bin/python

""" Generates an image bag containing only the images in a given folder.
    Modified from https://answers.ros.org/question/11537/creating-a-bag-file-out-of-a-image-sequence/.
"""

import os
import sys
import time

import roslib
import rospy
from ros import rosbag

roslib.load_manifest("sensor_msgs")
import cv2
from cv_bridge import CvBridge
from PIL import ImageFile
from sensor_msgs.msg import Image


def get_files_from_dir(dir):
    """Generates a list of files from the directory"""
    print("Searching directory %s" % dir)
    all = []
    left_files = []
    right_files = []
    if os.path.exists(dir):
        for path, names, files in os.walk(dir):
            for f in sorted(files):
                if os.path.splitext(f)[1] in [".bmp", ".png", ".jpg", ".ppm"]:
                    if "left" in f or "left" in path:
                        left_files.append(os.path.join(path, f))
                    elif "right" in f or "right" in path:
                        right_files.append(os.path.join(path, f))
                    all.append(os.path.join(path, f))
    return all, left_files, right_files


def create_mono_bag(imgs, bagname):
    """Creates a bag file with camera images"""
    bag = rosbag.Bag(bagname, "w")

    try:
        for i in range(len(imgs)):
            print("Adding %s" % imgs[i])
            img = cv2.imread(imgs[i])
            bridge = CvBridge()

            Stamp = rospy.rostime.Time.from_sec(time.time())
            img_msg = Image()
            img_msg = bridge.cv2_to_imgmsg(img, "bgr8")
            img_msg.header.seq = i
            img_msg.header.stamp = Stamp
            img_msg.header.frame_id = "camera"

            bag.write("mgt/img_sampler/nav_cam/image_record", img_msg, Stamp)
    finally:
        bag.close()


def create_bag(args):
    """Creates the actual bag file by successively adding images"""
    all_imgs, left_imgs, right_imgs = get_files_from_dir(args[0])

    if len(all_imgs) <= 0:
        print("No images found in %s" % args[0])
        exit()

    # create bagfile with mono camera image stream
    create_mono_bag(all_imgs, args[1])


if __name__ == "__main__":
    if len(sys.argv) == 3:
        create_bag(sys.argv[1:])
    else:
        print("Usage: img2bag imagedir bagfilename")
