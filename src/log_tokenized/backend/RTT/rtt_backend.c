/**
 * @file rtt_backend.c
 * @brief RTT backend for tokenized logging
 *
 * Implements pw_log_tokenized_HandleLogWithoutMetadata() by writing
 * length-prefixed binary frames to SEGGER RTT.
 *
 * Frame format: [2-byte LE length][encoded message bytes]
 * The host-side decoder reads the length first, then reads that many bytes.
 */

#include "log_tokenized/backend/RTT/rtt_backend.h"

#include <string.h>
#include "SEGGER_RTT.h"

void rtt_backend_init(void) {
    SEGGER_RTT_Init();
}
#ifdef USING_RTT_BACKEND
/// Called by the tokenized logging pipeline for every log message.
/// Writes a length-prefixed frame so the host can parse individual messages.
void pw_log_tokenized_HandleLogWithoutMetadata(
    const uint8_t encoded_message[], size_t size_bytes) {
    if (encoded_message == NULL || size_bytes == 0) {
        return;
    }

    uint8_t frame[2 + P_LOG_TOKENIZED_ENCODING_BUFFER_SIZE_BYTES];

    // Keep header + payload in one RTT write. If header and payload are written
    // separately, SEGGER_RTT_MODE_NO_BLOCK_SKIP may keep only one part when the
    // ring buffer is full, leaving the host-side length-prefixed stream corrupt.
    if (size_bytes > sizeof(frame) - 2) {
        size_bytes = sizeof(frame) - 2;
    }

    // 2-byte little-endian length prefix
    //RTT协议要求每条日志前加若干字节长度信息
    frame[0] = (uint8_t)(size_bytes & 0xFF);
    frame[1] = (uint8_t)((size_bytes >> 8) & 0xFF);
    memcpy(&frame[2], encoded_message, size_bytes);

    // Write one complete frame to RTT channel.
    SEGGER_RTT_Write(P_RTT_LOG_CHANNEL, frame, (unsigned)(size_bytes + 2));
}
#endif  // USING_RTT_BACKEND