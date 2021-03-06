// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: logic2world_robot.proto

#ifndef PROTOBUF_logic2world_5frobot_2eproto__INCLUDED
#define PROTOBUF_logic2world_5frobot_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
#include "logic2world_msg_type.pb.h"
#include "msg_info_def.pb.h"
#include "msg_type_def.pb.h"
#include "pump_type.pb.h"
// @@protoc_insertion_point(includes)

namespace logic2world_protocols {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_logic2world_5frobot_2eproto();
void protobuf_AssignDesc_logic2world_5frobot_2eproto();
void protobuf_ShutdownFile_logic2world_5frobot_2eproto();

class packetl2w_request_robot;

// ===================================================================

class packetl2w_request_robot : public ::google::protobuf::Message {
 public:
  packetl2w_request_robot();
  virtual ~packetl2w_request_robot();

  packetl2w_request_robot(const packetl2w_request_robot& from);

  inline packetl2w_request_robot& operator=(const packetl2w_request_robot& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const packetl2w_request_robot& default_instance();

  void Swap(packetl2w_request_robot* other);

  // implements Message ----------------------------------------------

  packetl2w_request_robot* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const packetl2w_request_robot& from);
  void MergeFrom(const packetl2w_request_robot& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .logic2world_protocols.e_server_msg_type packet_id = 1 [default = e_mst_l2w_robot_request];
  inline bool has_packet_id() const;
  inline void clear_packet_id();
  static const int kPacketIdFieldNumber = 1;
  inline ::logic2world_protocols::e_server_msg_type packet_id() const;
  inline void set_packet_id(::logic2world_protocols::e_server_msg_type value);

  // optional int64 needgold = 2;
  inline bool has_needgold() const;
  inline void clear_needgold();
  static const int kNeedgoldFieldNumber = 2;
  inline ::google::protobuf::int64 needgold() const;
  inline void set_needgold(::google::protobuf::int64 value);

  // optional int32 needvip = 3;
  inline bool has_needvip() const;
  inline void clear_needvip();
  static const int kNeedvipFieldNumber = 3;
  inline ::google::protobuf::int32 needvip() const;
  inline void set_needvip(::google::protobuf::int32 value);

  // optional int32 gameid = 4;
  inline bool has_gameid() const;
  inline void clear_gameid();
  static const int kGameidFieldNumber = 4;
  inline ::google::protobuf::int32 gameid() const;
  inline void set_gameid(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:logic2world_protocols.packetl2w_request_robot)
 private:
  inline void set_has_packet_id();
  inline void clear_has_packet_id();
  inline void set_has_needgold();
  inline void clear_has_needgold();
  inline void set_has_needvip();
  inline void clear_has_needvip();
  inline void set_has_gameid();
  inline void clear_has_gameid();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int64 needgold_;
  int packet_id_;
  ::google::protobuf::int32 needvip_;
  ::google::protobuf::int32 gameid_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  friend void  protobuf_AddDesc_logic2world_5frobot_2eproto();
  friend void protobuf_AssignDesc_logic2world_5frobot_2eproto();
  friend void protobuf_ShutdownFile_logic2world_5frobot_2eproto();

  void InitAsDefaultInstance();
  static packetl2w_request_robot* default_instance_;
};
// ===================================================================


// ===================================================================

// packetl2w_request_robot

// optional .logic2world_protocols.e_server_msg_type packet_id = 1 [default = e_mst_l2w_robot_request];
inline bool packetl2w_request_robot::has_packet_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void packetl2w_request_robot::set_has_packet_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void packetl2w_request_robot::clear_has_packet_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void packetl2w_request_robot::clear_packet_id() {
  packet_id_ = 20012;
  clear_has_packet_id();
}
inline ::logic2world_protocols::e_server_msg_type packetl2w_request_robot::packet_id() const {
  return static_cast< ::logic2world_protocols::e_server_msg_type >(packet_id_);
}
inline void packetl2w_request_robot::set_packet_id(::logic2world_protocols::e_server_msg_type value) {
  assert(::logic2world_protocols::e_server_msg_type_IsValid(value));
  set_has_packet_id();
  packet_id_ = value;
}

// optional int64 needgold = 2;
inline bool packetl2w_request_robot::has_needgold() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void packetl2w_request_robot::set_has_needgold() {
  _has_bits_[0] |= 0x00000002u;
}
inline void packetl2w_request_robot::clear_has_needgold() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void packetl2w_request_robot::clear_needgold() {
  needgold_ = GOOGLE_LONGLONG(0);
  clear_has_needgold();
}
inline ::google::protobuf::int64 packetl2w_request_robot::needgold() const {
  return needgold_;
}
inline void packetl2w_request_robot::set_needgold(::google::protobuf::int64 value) {
  set_has_needgold();
  needgold_ = value;
}

// optional int32 needvip = 3;
inline bool packetl2w_request_robot::has_needvip() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void packetl2w_request_robot::set_has_needvip() {
  _has_bits_[0] |= 0x00000004u;
}
inline void packetl2w_request_robot::clear_has_needvip() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void packetl2w_request_robot::clear_needvip() {
  needvip_ = 0;
  clear_has_needvip();
}
inline ::google::protobuf::int32 packetl2w_request_robot::needvip() const {
  return needvip_;
}
inline void packetl2w_request_robot::set_needvip(::google::protobuf::int32 value) {
  set_has_needvip();
  needvip_ = value;
}

// optional int32 gameid = 4;
inline bool packetl2w_request_robot::has_gameid() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void packetl2w_request_robot::set_has_gameid() {
  _has_bits_[0] |= 0x00000008u;
}
inline void packetl2w_request_robot::clear_has_gameid() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void packetl2w_request_robot::clear_gameid() {
  gameid_ = 0;
  clear_has_gameid();
}
inline ::google::protobuf::int32 packetl2w_request_robot::gameid() const {
  return gameid_;
}
inline void packetl2w_request_robot::set_gameid(::google::protobuf::int32 value) {
  set_has_gameid();
  gameid_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace logic2world_protocols

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_logic2world_5frobot_2eproto__INCLUDED
