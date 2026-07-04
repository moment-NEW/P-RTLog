/**
 * @file plog_tokenized_light.c
 * @brief 轻量级的plog_tokenized实现
 * @author 
 * @date 2026-07-03
 */

#include <cstdint>
#include <memory.h>
#include "log_tokenized/log_tokenized_light.h"
#include "p_tokenizer.h"
#include "p_span.h"
#include "config.h"
#include "p_log_tokenized/handler.h"
//绝大部分CC中的pw_tokenizer_Token和pw_tokenizer_ArgTypes之类的类型都是uint32_t类型，所以这里直接使用uint32_t来代替
//至于这是为什么尚不清楚
typedef struct {
    uint8_t data[P_LOG_TOKENIZED_ENCODING_BUFFER_SIZE_BYTES];  // 缓冲区用于存储编码后的日志消息
    size_t size;
} p_encoded_message_t;

/**
 * @brief 编码日志消息
 * @param token 日志消息的token
 * @param types 日志消息的参数类型
 * @param args 可变参数列表
 * @return 编码后的字节数
 */
size_t P_EncodeArgs(uint32_t types,
                  va_list args,
                  p_span_t output) {
   // 这里实现了一个简单的编码逻辑，将参数按类型编码到输出缓冲区中
    // 实际实现可能需要根据types来决定如何编码每个参数
  size_t arg_count = types & P_TOKENIZER_TYPE_COUNT_MASK;
  types >>= P_TOKENIZER_TYPE_COUNT_SIZE_BITS;

  size_t encoded_bytes = 0;
  while (arg_count != 0u) {
    // How many bytes were encoded; 0 indicates that there wasn't enough space.
    size_t argument_bytes = 0;
    //这段谷歌原意大概就是区分不同长度的消息类型。
    switch (types & 0b11u) {
     case 0:  /* int */
        argument_bytes = p_encode_int(va_arg(args, int), output);
        break;
      case 1:  /* int64 */
        argument_bytes = p_encode_int64(va_arg(args, int64_t), output);
        break;
      case 2:  /* double -> float */
        argument_bytes = p_encode_float((float)va_arg(args, double), output);
        break;
      case 3:  /* string */
        argument_bytes = p_encode_string(va_arg(args, const char *), output);
        break;
    }

    // If zero bytes were encoded, the encoding buffer is full.
    if (argument_bytes == 0u) {
      break;
    }

    output = p_span_subspan(&output, argument_bytes,
                            p_span_size(&output) - argument_bytes);
    encoded_bytes += argument_bytes;

    arg_count -= 1;
    types >>= 2;  // each argument type is encoded in two bits
  }

  return encoded_bytes;
}


/**
原文件大概如下：
EncodedMessage(pw_tokenizer_Token token,
                 pw_tokenizer_ArgTypes types,
                 va_list args) {
    std::memcpy(data_, &token, sizeof(token));
    size_ =
        sizeof(token) +
        EncodeArgs(types, args, span<std::byte>(data_).subspan(sizeof(token)));
  }

*/


static void p_EncodedMessage(
                 p_encoded_message_t *msg,
                 uint32_t token,
                 uint32_t types,
                 va_list args) {

        memcpy(msg->data, &token, sizeof(token));
        msg->size =
            sizeof(token) +
            p_encode_args(types, args, span<std::byte>(msg->data).subspan(sizeof(token)));

}
//施工中，先实现依赖的各种函数
void p_log_tokenized_encode_without_metadata(uint32_t token, uint32_t types, ...) {
  va_list args;//va_list 是一个用于访问可变参数的类型，通常用于实现类似 printf 的函数。
  va_start(args, types);
  //p_EncodedMessage<P_LOG_TOKENIZED_ENCODING_BUFFER_SIZE_BYTES>
  //     encoded_message(token, types, args);
  va_end(args);

  // pw_log_tokenized_HandleLogWithoutMetadata(encoded_message.data_as_uint8(),
  //                                           encoded_message.size());
}