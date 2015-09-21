// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: NutsWrapperMessage.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "NutsWrapperMessage.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace {

const ::google::protobuf::Descriptor* NutsWrapperMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  NutsWrapperMessage_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* MessageType_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_NutsWrapperMessage_2eproto() {
  protobuf_AddDesc_NutsWrapperMessage_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "NutsWrapperMessage.proto");
  GOOGLE_CHECK(file != NULL);
  NutsWrapperMessage_descriptor_ = file->message_type(0);
  static const int NutsWrapperMessage_offsets_[8] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, sequencenum_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, route_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, source_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, destination_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, messagename_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, messagecode_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, messageentity_),
  };
  NutsWrapperMessage_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      NutsWrapperMessage_descriptor_,
      NutsWrapperMessage::default_instance_,
      NutsWrapperMessage_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(NutsWrapperMessage, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(NutsWrapperMessage));
  MessageType_descriptor_ = file->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_NutsWrapperMessage_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    NutsWrapperMessage_descriptor_, &NutsWrapperMessage::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_NutsWrapperMessage_2eproto() {
  delete NutsWrapperMessage::default_instance_;
  delete NutsWrapperMessage_reflection_;
}

void protobuf_AddDesc_NutsWrapperMessage_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\030NutsWrapperMessage.proto\"\272\001\n\022NutsWrapp"
    "erMessage\022\032\n\004Type\030\001 \002(\0162\014.MessageType\022\023\n"
    "\013SequenceNum\030\002 \002(\r\022\r\n\005Route\030\003 \003(\r\022\016\n\006Sou"
    "rce\030\004 \001(\r\022\023\n\013Destination\030\005 \001(\r\022\023\n\013Messag"
    "eName\030\006 \001(\t\022\023\n\013MessageCode\030\007 \002(\004\022\025\n\rMess"
    "ageEntity\030\010 \002(\014*|\n\013MessageType\022\023\n\017UNKNOW"
    "N_MSGTYPE\020\000\022\024\n\020REQUEST_WITH_RSP\020\001\022\027\n\023REQ"
    "UEST_WITHOUT_RSP\020\002\022\023\n\017RESPONSE_NORMAL\020\003\022"
    "\024\n\020RESPONSE_TIMEOUT\020\004", 341);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "NutsWrapperMessage.proto", &protobuf_RegisterTypes);
  NutsWrapperMessage::default_instance_ = new NutsWrapperMessage();
  NutsWrapperMessage::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_NutsWrapperMessage_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_NutsWrapperMessage_2eproto {
  StaticDescriptorInitializer_NutsWrapperMessage_2eproto() {
    protobuf_AddDesc_NutsWrapperMessage_2eproto();
  }
} static_descriptor_initializer_NutsWrapperMessage_2eproto_;
const ::google::protobuf::EnumDescriptor* MessageType_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return MessageType_descriptor_;
}
bool MessageType_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      return true;
    default:
      return false;
  }
}


// ===================================================================

#ifndef _MSC_VER
const int NutsWrapperMessage::kTypeFieldNumber;
const int NutsWrapperMessage::kSequenceNumFieldNumber;
const int NutsWrapperMessage::kRouteFieldNumber;
const int NutsWrapperMessage::kSourceFieldNumber;
const int NutsWrapperMessage::kDestinationFieldNumber;
const int NutsWrapperMessage::kMessageNameFieldNumber;
const int NutsWrapperMessage::kMessageCodeFieldNumber;
const int NutsWrapperMessage::kMessageEntityFieldNumber;
#endif  // !_MSC_VER

NutsWrapperMessage::NutsWrapperMessage()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void NutsWrapperMessage::InitAsDefaultInstance() {
}

NutsWrapperMessage::NutsWrapperMessage(const NutsWrapperMessage& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void NutsWrapperMessage::SharedCtor() {
  _cached_size_ = 0;
  type_ = 0;
  sequencenum_ = 0u;
  source_ = 0u;
  destination_ = 0u;
  messagename_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  messagecode_ = GOOGLE_ULONGLONG(0);
  messageentity_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

NutsWrapperMessage::~NutsWrapperMessage() {
  SharedDtor();
}

void NutsWrapperMessage::SharedDtor() {
  if (messagename_ != &::google::protobuf::internal::kEmptyString) {
    delete messagename_;
  }
  if (messageentity_ != &::google::protobuf::internal::kEmptyString) {
    delete messageentity_;
  }
  if (this != default_instance_) {
  }
}

void NutsWrapperMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* NutsWrapperMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return NutsWrapperMessage_descriptor_;
}

const NutsWrapperMessage& NutsWrapperMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_NutsWrapperMessage_2eproto();
  return *default_instance_;
}

NutsWrapperMessage* NutsWrapperMessage::default_instance_ = NULL;

NutsWrapperMessage* NutsWrapperMessage::New() const {
  return new NutsWrapperMessage;
}

void NutsWrapperMessage::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    type_ = 0;
    sequencenum_ = 0u;
    source_ = 0u;
    destination_ = 0u;
    if (has_messagename()) {
      if (messagename_ != &::google::protobuf::internal::kEmptyString) {
        messagename_->clear();
      }
    }
    messagecode_ = GOOGLE_ULONGLONG(0);
    if (has_messageentity()) {
      if (messageentity_ != &::google::protobuf::internal::kEmptyString) {
        messageentity_->clear();
      }
    }
  }
  route_.Clear();
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool NutsWrapperMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .MessageType Type = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::MessageType_IsValid(value)) {
            set_type(static_cast< ::MessageType >(value));
          } else {
            mutable_unknown_fields()->AddVarint(1, value);
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_SequenceNum;
        break;
      }

      // required uint32 SequenceNum = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_SequenceNum:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &sequencenum_)));
          set_has_sequencenum();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_Route;
        break;
      }

      // repeated uint32 Route = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_Route:
          DO_((::google::protobuf::internal::WireFormatLite::ReadRepeatedPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 1, 24, input, this->mutable_route())));
        } else if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag)
                   == ::google::protobuf::internal::WireFormatLite::
                      WIRETYPE_LENGTH_DELIMITED) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPackedPrimitiveNoInline<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, this->mutable_route())));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_Route;
        if (input->ExpectTag(32)) goto parse_Source;
        break;
      }

      // optional uint32 Source = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_Source:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &source_)));
          set_has_source();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(40)) goto parse_Destination;
        break;
      }

      // optional uint32 Destination = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_Destination:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &destination_)));
          set_has_destination();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(50)) goto parse_MessageName;
        break;
      }

      // optional string MessageName = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_MessageName:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_messagename()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->messagename().data(), this->messagename().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(56)) goto parse_MessageCode;
        break;
      }

      // required uint64 MessageCode = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_MessageCode:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &messagecode_)));
          set_has_messagecode();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(66)) goto parse_MessageEntity;
        break;
      }

      // required bytes MessageEntity = 8;
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_MessageEntity:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_messageentity()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void NutsWrapperMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required .MessageType Type = 1;
  if (has_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      1, this->type(), output);
  }

  // required uint32 SequenceNum = 2;
  if (has_sequencenum()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->sequencenum(), output);
  }

  // repeated uint32 Route = 3;
  for (int i = 0; i < this->route_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(
      3, this->route(i), output);
  }

  // optional uint32 Source = 4;
  if (has_source()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(4, this->source(), output);
  }

  // optional uint32 Destination = 5;
  if (has_destination()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(5, this->destination(), output);
  }

  // optional string MessageName = 6;
  if (has_messagename()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->messagename().data(), this->messagename().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      6, this->messagename(), output);
  }

  // required uint64 MessageCode = 7;
  if (has_messagecode()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(7, this->messagecode(), output);
  }

  // required bytes MessageEntity = 8;
  if (has_messageentity()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      8, this->messageentity(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* NutsWrapperMessage::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required .MessageType Type = 1;
  if (has_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      1, this->type(), target);
  }

  // required uint32 SequenceNum = 2;
  if (has_sequencenum()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->sequencenum(), target);
  }

  // repeated uint32 Route = 3;
  for (int i = 0; i < this->route_size(); i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteUInt32ToArray(3, this->route(i), target);
  }

  // optional uint32 Source = 4;
  if (has_source()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(4, this->source(), target);
  }

  // optional uint32 Destination = 5;
  if (has_destination()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(5, this->destination(), target);
  }

  // optional string MessageName = 6;
  if (has_messagename()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->messagename().data(), this->messagename().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        6, this->messagename(), target);
  }

  // required uint64 MessageCode = 7;
  if (has_messagecode()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(7, this->messagecode(), target);
  }

  // required bytes MessageEntity = 8;
  if (has_messageentity()) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        8, this->messageentity(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int NutsWrapperMessage::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required .MessageType Type = 1;
    if (has_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->type());
    }

    // required uint32 SequenceNum = 2;
    if (has_sequencenum()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->sequencenum());
    }

    // optional uint32 Source = 4;
    if (has_source()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->source());
    }

    // optional uint32 Destination = 5;
    if (has_destination()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->destination());
    }

    // optional string MessageName = 6;
    if (has_messagename()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->messagename());
    }

    // required uint64 MessageCode = 7;
    if (has_messagecode()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->messagecode());
    }

    // required bytes MessageEntity = 8;
    if (has_messageentity()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->messageentity());
    }

  }
  // repeated uint32 Route = 3;
  {
    int data_size = 0;
    for (int i = 0; i < this->route_size(); i++) {
      data_size += ::google::protobuf::internal::WireFormatLite::
        UInt32Size(this->route(i));
    }
    total_size += 1 * this->route_size() + data_size;
  }

  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void NutsWrapperMessage::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const NutsWrapperMessage* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const NutsWrapperMessage*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void NutsWrapperMessage::MergeFrom(const NutsWrapperMessage& from) {
  GOOGLE_CHECK_NE(&from, this);
  route_.MergeFrom(from.route_);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_type()) {
      set_type(from.type());
    }
    if (from.has_sequencenum()) {
      set_sequencenum(from.sequencenum());
    }
    if (from.has_source()) {
      set_source(from.source());
    }
    if (from.has_destination()) {
      set_destination(from.destination());
    }
    if (from.has_messagename()) {
      set_messagename(from.messagename());
    }
    if (from.has_messagecode()) {
      set_messagecode(from.messagecode());
    }
    if (from.has_messageentity()) {
      set_messageentity(from.messageentity());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void NutsWrapperMessage::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void NutsWrapperMessage::CopyFrom(const NutsWrapperMessage& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool NutsWrapperMessage::IsInitialized() const {
  if ((_has_bits_[0] & 0x000000c3) != 0x000000c3) return false;

  return true;
}

void NutsWrapperMessage::Swap(NutsWrapperMessage* other) {
  if (other != this) {
    std::swap(type_, other->type_);
    std::swap(sequencenum_, other->sequencenum_);
    route_.Swap(&other->route_);
    std::swap(source_, other->source_);
    std::swap(destination_, other->destination_);
    std::swap(messagename_, other->messagename_);
    std::swap(messagecode_, other->messagecode_);
    std::swap(messageentity_, other->messageentity_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata NutsWrapperMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = NutsWrapperMessage_descriptor_;
  metadata.reflection = NutsWrapperMessage_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)