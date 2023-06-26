# This Python file uses the following encoding: utf-8
"""autogenerated by genpy from ff_msgs/CameraState.msg. Do not edit."""
import codecs
import sys
python3 = True if sys.hexversion > 0x03000000 else False
import genpy
import struct


class CameraState(genpy.Message):
  _md5sum = "644cfd14384d17cf28911b625a446f53"
  _type = "ff_msgs/CameraState"
  _has_header = False  # flag to mark the presence of a Header object
  _full_text = """# Copyright (c) 2017, United States Government, as represented by the
# Administrator of the National Aeronautics and Space Administration.
# 
# All rights reserved.
# 
# The Astrobee platform is licensed under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License. You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#
# CameraState message, *MUST* be kept in sync with camera portion of
# rapid::ext::astrobee::TelemetryState

# nav_cam, dock_cam, etc.
string camera_name

# streaming to ground
bool streaming

# image width
uint16 stream_width
# image height
uint16 stream_height
# Rate in Hz
float32 stream_rate

# recording to disk
bool recording

# image width
uint16 record_width
# image height
uint16 record_height
# Rate in Hz
float32 record_rate

# only for sci cam
float32 bandwidth
"""
  __slots__ = ['camera_name','streaming','stream_width','stream_height','stream_rate','recording','record_width','record_height','record_rate','bandwidth']
  _slot_types = ['string','bool','uint16','uint16','float32','bool','uint16','uint16','float32','float32']

  def __init__(self, *args, **kwds):
    """
    Constructor. Any message fields that are implicitly/explicitly
    set to None will be assigned a default value. The recommend
    use is keyword arguments as this is more robust to future message
    changes.  You cannot mix in-order arguments and keyword arguments.

    The available fields are:
       camera_name,streaming,stream_width,stream_height,stream_rate,recording,record_width,record_height,record_rate,bandwidth

    :param args: complete set of field values, in .msg order
    :param kwds: use keyword arguments corresponding to message field names
    to set specific fields.
    """
    if args or kwds:
      super(CameraState, self).__init__(*args, **kwds)
      # message fields cannot be None, assign default values for those that are
      if self.camera_name is None:
        self.camera_name = ''
      if self.streaming is None:
        self.streaming = False
      if self.stream_width is None:
        self.stream_width = 0
      if self.stream_height is None:
        self.stream_height = 0
      if self.stream_rate is None:
        self.stream_rate = 0.
      if self.recording is None:
        self.recording = False
      if self.record_width is None:
        self.record_width = 0
      if self.record_height is None:
        self.record_height = 0
      if self.record_rate is None:
        self.record_rate = 0.
      if self.bandwidth is None:
        self.bandwidth = 0.
    else:
      self.camera_name = ''
      self.streaming = False
      self.stream_width = 0
      self.stream_height = 0
      self.stream_rate = 0.
      self.recording = False
      self.record_width = 0
      self.record_height = 0
      self.record_rate = 0.
      self.bandwidth = 0.

  def _get_types(self):
    """
    internal API method
    """
    return self._slot_types

  def serialize(self, buff):
    """
    serialize message into buffer
    :param buff: buffer, ``StringIO``
    """
    try:
      _x = self.camera_name
      length = len(_x)
      if python3 or type(_x) == unicode:
        _x = _x.encode('utf-8')
        length = len(_x)
      buff.write(struct.Struct('<I%ss'%length).pack(length, _x))
      _x = self
      buff.write(_get_struct_B2HfB2H2f().pack(_x.streaming, _x.stream_width, _x.stream_height, _x.stream_rate, _x.recording, _x.record_width, _x.record_height, _x.record_rate, _x.bandwidth))
    except struct.error as se: self._check_types(struct.error("%s: '%s' when writing '%s'" % (type(se), str(se), str(locals().get('_x', self)))))
    except TypeError as te: self._check_types(ValueError("%s: '%s' when writing '%s'" % (type(te), str(te), str(locals().get('_x', self)))))

  def deserialize(self, str):
    """
    unpack serialized message in str into this message instance
    :param str: byte array of serialized message, ``str``
    """
    if python3:
      codecs.lookup_error("rosmsg").msg_type = self._type
    try:
      end = 0
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.camera_name = str[start:end].decode('utf-8', 'rosmsg')
      else:
        self.camera_name = str[start:end]
      _x = self
      start = end
      end += 22
      (_x.streaming, _x.stream_width, _x.stream_height, _x.stream_rate, _x.recording, _x.record_width, _x.record_height, _x.record_rate, _x.bandwidth,) = _get_struct_B2HfB2H2f().unpack(str[start:end])
      self.streaming = bool(self.streaming)
      self.recording = bool(self.recording)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e)  # most likely buffer underfill


  def serialize_numpy(self, buff, numpy):
    """
    serialize message with numpy array types into buffer
    :param buff: buffer, ``StringIO``
    :param numpy: numpy python module
    """
    try:
      _x = self.camera_name
      length = len(_x)
      if python3 or type(_x) == unicode:
        _x = _x.encode('utf-8')
        length = len(_x)
      buff.write(struct.Struct('<I%ss'%length).pack(length, _x))
      _x = self
      buff.write(_get_struct_B2HfB2H2f().pack(_x.streaming, _x.stream_width, _x.stream_height, _x.stream_rate, _x.recording, _x.record_width, _x.record_height, _x.record_rate, _x.bandwidth))
    except struct.error as se: self._check_types(struct.error("%s: '%s' when writing '%s'" % (type(se), str(se), str(locals().get('_x', self)))))
    except TypeError as te: self._check_types(ValueError("%s: '%s' when writing '%s'" % (type(te), str(te), str(locals().get('_x', self)))))

  def deserialize_numpy(self, str, numpy):
    """
    unpack serialized message in str into this message instance using numpy for array types
    :param str: byte array of serialized message, ``str``
    :param numpy: numpy python module
    """
    if python3:
      codecs.lookup_error("rosmsg").msg_type = self._type
    try:
      end = 0
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.camera_name = str[start:end].decode('utf-8', 'rosmsg')
      else:
        self.camera_name = str[start:end]
      _x = self
      start = end
      end += 22
      (_x.streaming, _x.stream_width, _x.stream_height, _x.stream_rate, _x.recording, _x.record_width, _x.record_height, _x.record_rate, _x.bandwidth,) = _get_struct_B2HfB2H2f().unpack(str[start:end])
      self.streaming = bool(self.streaming)
      self.recording = bool(self.recording)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e)  # most likely buffer underfill

_struct_I = genpy.struct_I
def _get_struct_I():
    global _struct_I
    return _struct_I
_struct_B2HfB2H2f = None
def _get_struct_B2HfB2H2f():
    global _struct_B2HfB2H2f
    if _struct_B2HfB2H2f is None:
        _struct_B2HfB2H2f = struct.Struct("<B2HfB2H2f")
    return _struct_B2HfB2H2f
