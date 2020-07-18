/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: FingerprintProtocol.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "FingerprintProtocol.pb-c.h"
void   textsecure__logical_fingerprint__init
                     (Textsecure__LogicalFingerprint         *message)
{
  static const Textsecure__LogicalFingerprint init_value = TEXTSECURE__LOGICAL_FINGERPRINT__INIT;
  *message = init_value;
}
size_t textsecure__logical_fingerprint__get_packed_size
                     (const Textsecure__LogicalFingerprint *message)
{
  assert(message->base.descriptor == &textsecure__logical_fingerprint__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t textsecure__logical_fingerprint__pack
                     (const Textsecure__LogicalFingerprint *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &textsecure__logical_fingerprint__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t textsecure__logical_fingerprint__pack_to_buffer
                     (const Textsecure__LogicalFingerprint *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &textsecure__logical_fingerprint__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Textsecure__LogicalFingerprint *
       textsecure__logical_fingerprint__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Textsecure__LogicalFingerprint *)
     protobuf_c_message_unpack (&textsecure__logical_fingerprint__descriptor,
                                allocator, len, data);
}
void   textsecure__logical_fingerprint__free_unpacked
                     (Textsecure__LogicalFingerprint *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &textsecure__logical_fingerprint__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   textsecure__combined_fingerprints__init
                     (Textsecure__CombinedFingerprints         *message)
{
  static const Textsecure__CombinedFingerprints init_value = TEXTSECURE__COMBINED_FINGERPRINTS__INIT;
  *message = init_value;
}
size_t textsecure__combined_fingerprints__get_packed_size
                     (const Textsecure__CombinedFingerprints *message)
{
  assert(message->base.descriptor == &textsecure__combined_fingerprints__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t textsecure__combined_fingerprints__pack
                     (const Textsecure__CombinedFingerprints *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &textsecure__combined_fingerprints__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t textsecure__combined_fingerprints__pack_to_buffer
                     (const Textsecure__CombinedFingerprints *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &textsecure__combined_fingerprints__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Textsecure__CombinedFingerprints *
       textsecure__combined_fingerprints__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Textsecure__CombinedFingerprints *)
     protobuf_c_message_unpack (&textsecure__combined_fingerprints__descriptor,
                                allocator, len, data);
}
void   textsecure__combined_fingerprints__free_unpacked
                     (Textsecure__CombinedFingerprints *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &textsecure__combined_fingerprints__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor textsecure__logical_fingerprint__field_descriptors[2] =
{
  {
    "content",
    1,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_BYTES,
    offsetof(Textsecure__LogicalFingerprint, has_content),
    offsetof(Textsecure__LogicalFingerprint, content),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "identifier",
    2,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_BYTES,
    offsetof(Textsecure__LogicalFingerprint, has_identifier),
    offsetof(Textsecure__LogicalFingerprint, identifier),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned textsecure__logical_fingerprint__field_indices_by_name[] = {
  0,   /* field[0] = content */
  1,   /* field[1] = identifier */
};
static const ProtobufCIntRange textsecure__logical_fingerprint__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor textsecure__logical_fingerprint__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "textsecure.LogicalFingerprint",
  "LogicalFingerprint",
  "Textsecure__LogicalFingerprint",
  "textsecure",
  sizeof(Textsecure__LogicalFingerprint),
  2,
  textsecure__logical_fingerprint__field_descriptors,
  textsecure__logical_fingerprint__field_indices_by_name,
  1,  textsecure__logical_fingerprint__number_ranges,
  (ProtobufCMessageInit) textsecure__logical_fingerprint__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor textsecure__combined_fingerprints__field_descriptors[3] =
{
  {
    "version",
    1,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT32,
    offsetof(Textsecure__CombinedFingerprints, has_version),
    offsetof(Textsecure__CombinedFingerprints, version),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "localFingerprint",
    2,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Textsecure__CombinedFingerprints, localfingerprint),
    &textsecure__logical_fingerprint__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "remoteFingerprint",
    3,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Textsecure__CombinedFingerprints, remotefingerprint),
    &textsecure__logical_fingerprint__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned textsecure__combined_fingerprints__field_indices_by_name[] = {
  1,   /* field[1] = localFingerprint */
  2,   /* field[2] = remoteFingerprint */
  0,   /* field[0] = version */
};
static const ProtobufCIntRange textsecure__combined_fingerprints__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor textsecure__combined_fingerprints__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "textsecure.CombinedFingerprints",
  "CombinedFingerprints",
  "Textsecure__CombinedFingerprints",
  "textsecure",
  sizeof(Textsecure__CombinedFingerprints),
  3,
  textsecure__combined_fingerprints__field_descriptors,
  textsecure__combined_fingerprints__field_indices_by_name,
  1,  textsecure__combined_fingerprints__number_ranges,
  (ProtobufCMessageInit) textsecure__combined_fingerprints__init,
  NULL,NULL,NULL    /* reserved[123] */
};