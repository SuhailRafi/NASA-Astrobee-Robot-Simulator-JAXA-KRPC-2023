#! /usr/bin/env python
# coding:UTF-8

import argparse
import rospy
import json
import os
import std_msgs

from ff_msgs.msg import GuestScienceData

# Default values for boot options.
DEFAULT_PHASE_CONFIG_PATH = "files/random_phase_config.json"

# String key sent from apk.
JSON_STATUS  = "status"
UNDOCK_START = "Undock Start" # The key for stop to publish phase config.

class PhaseProvider():
  def __init__(self, phase_config_path:str):
    self.NODE_NAME         = 'kibo_rpc_phase_provider'
    self.PHASE_INFO_TOPIC  = '/krpc/comm/phases_info'
    self.GDS_MSG_TOPIC     = '/gs/data'

    self.phase_config_path = phase_config_path
    self.api_subscribed    = False
    self.phase_config_str  = ''

  def start(self) -> None:
    rospy.init_node(self.NODE_NAME)
    self.start_subscriber()
    self.start_publisher()
    rospy.spin()

  def finish(self, is_OK:bool) -> None:
    log_msg = f'{self.NODE_NAME} Finish'
    if is_OK:
      rospy.loginfo(log_msg)
    else:
      rospy.logerr(log_msg)
    # rospy.signal_shutdown('finish')

  def start_subscriber(self):
    rospy.Subscriber(self.GDS_MSG_TOPIC, GuestScienceData, self.gs_callback, queue_size=20)
    rospy.loginfo(f"{self.NODE_NAME} Ready")

  def start_publisher(self):
    pub_phases_info = rospy.Publisher(self.PHASE_INFO_TOPIC, std_msgs.msg.String, queue_size=1)

    # Load phase config.
    if self.load_phase_config():

      rate = rospy.Rate(10) # 10Hz
      while not rospy.is_shutdown():
          if not self.api_subscribed:
            pub_phases_info.publish(self.phase_config_str)
            rate.sleep()
          else:
            # Finish to publish.
            break
      self.finish(True)

  def load_phase_config(self) -> bool:
    if not os.path.exists(self.phase_config_path):
      rospy.logerr(f'Specified path by <{self.phase_config_path}> is not existed.')
      self.finish(False)
      return False
    else:
      # Read random phase config file.
      phase_config = self.load_json(self.phase_config_path)
      self.phase_config_str = self.json2jsonstr(phase_config)
      return True

  def load_json(self, path:str) -> dict:
    with open(path, "r") as f:
        data = json.load(f)
    return data

  def json2jsonstr(self, data:dict) -> str:
    data_str = json.dumps(data)
    return data_str

  # Callback from GDS messages.
  def gs_callback(self, data) -> None:
    try:
      request = json.loads(data.data)
      self.parse_judge_msg(request)
    except:
      # Cannot parse = Ignore irrelevant data
      pass

  # JUDGE API processing inlet
  def parse_judge_msg(self, data) -> None:

    # Check input value.
    if data is None:
      rospy.logwarn("Request data is None.")
      return

    # Get payload and timestamp.
    status = data.get(JSON_STATUS, None)

    # Into procesing payload.
    rospy.loginfo(f"status arrived: {status}")
    if(status == UNDOCK_START):
      # Judge api side got phase config or not by this status.
      self.api_subscribed = True
      rospy.loginfo('Phase config was subscribed.')
      self.finish(True)

def parse_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('-p', '--phase_config_path', default=DEFAULT_PHASE_CONFIG_PATH)
  args = parser.parse_args()
  return args

def main():
  # Use activation argument not to use rosparam because of protecting access from APK.
  args = parse_args()
  phaseProvider = PhaseProvider(args.phase_config_path)

  try:
    phaseProvider.start()
  except rospy.ROSInterruptException:
    # For [ctrl+c]
    pass

if __name__ == '__main__':
  main()
