/**
 * @file plog_tokenized_light.c
 * @brief 轻量级的plog_tokenized实现
 * @author 
 * @date 2026-07-03
 */
#include <memory.h>
#include "p_tokenizer.h"
#include "p_log_tokenized/handler.h"
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


static void p_EncodedMessage(pw_tokenizer_Token token,
                 pw_tokenizer_ArgTypes types,
                 va_list args) {

        memcpy(data_, &token, sizeof(token));
        size_ =
            sizeof(token) +
            EncodeArgs(types, args, span<std::byte>(data_).subspan(sizeof(token)));


}
void p_log_tokenized_encode_without_metadata(pw_tokenizer_Token token, pw_tokenizer_ArgTypes types, ...) {
  va_list args;
  va_start(args, types);
  // pw::tokenizer::EncodedMessage<P_LOG_TOKENIZED_ENCODING_BUFFER_SIZE_BYTES>
  //     encoded_message(token, types, args);
  va_end(args);

  // pw_log_tokenized_HandleLogWithoutMetadata(encoded_message.data_as_uint8(),
  //                                           encoded_message.size());
}