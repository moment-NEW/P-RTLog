#!/usr/bin/env python3
"""
P-RTLog host-side decoder.

Extracts tokenized string database from ELF and decodes RTT log messages.

Usage:
  python decode_rtt_log.py <elf_file> <rtt_log_file>

Example:
  python decode_rtt_log.py firmware.elf rtt_capture.bin
"""

import sys
import struct
from pathlib import Path
from typing import Dict, Tuple

try:
    from elftools.elf.elffile import ELFFile
except ImportError:
    print("Error: pyelftools not installed. Run: pip install pyelftools")
    sys.exit(1)


# Magic number for tokenizer entries (from tokenize_string.h)
TOKENIZER_ENTRY_MAGIC = 0xBAA98DEE


def extract_token_database(elf_path: Path) -> Dict[int, str]:
    """
    Extract token → string mapping from ELF's .pw_tokenizer.entries section.
    
    Returns:
        Dict mapping token (uint32) to format string.
    """
    token_db: Dict[int, str] = {}
    
    with open(elf_path, 'rb') as f:
        elf = ELFFile(f)
        
        # Find .pw_tokenizer.entries section
        section = None
        for sec in elf.iter_sections():
            if sec.name == '.pw_tokenizer.entries':
                section = sec
                break
        
        if section is None:
            print(f"Warning: .pw_tokenizer.entries section not found in {elf_path}")
            return token_db
        
        data = section.data()
        offset = 0
        
        # Parse entries: each entry has a header followed by domain and string
        while offset + 16 <= len(data):  # 16 bytes = header size
            # Parse header: magic, token, domain_length, string_length
            magic, token, domain_len, string_len = struct.unpack_from(
                '<IIII', data, offset
            )
            offset += 16
            
            if magic != TOKENIZER_ENTRY_MAGIC:
                print(f"Warning: Invalid magic at offset {offset-16}, stopping")
                break
            
            # Read domain string (includes null terminator)
            if offset + domain_len > len(data):
                break
            domain = data[offset:offset + domain_len - 1].decode('utf-8', errors='replace')
            offset += domain_len
            
            # Read format string (includes null terminator)
            if offset + string_len > len(data):
                break
            format_str = data[offset:offset + string_len - 1].decode('utf-8', errors='replace')
            offset += string_len
            
            token_db[token] = format_str
    
    return token_db


def decode_rtt_log(log_path: Path, token_db: Dict[int, str]):
    """
    Decode RTT log file with length-prefixed frames.
    
    Frame format: [2-byte LE length][encoded message]
    Encoded message: [4-byte token][varint-encoded args...]
    """
    with open(log_path, 'rb') as f:
        data = f.read()
    
    offset = 0
    message_count = 0
    
    while offset + 2 <= len(data):
        # Read 2-byte length prefix (little-endian)
        length = struct.unpack_from('<H', data, offset)[0]
        offset += 2
        
        if offset + length > len(data):
            print(f"Warning: Truncated message at offset {offset-2}")
            break
        
        # Extract encoded message
        encoded = data[offset:offset + length]
        offset += length
        message_count += 1
        
        if len(encoded) < 4:
            print(f"[{message_count}] Error: Message too short ({len(encoded)} bytes)")
            continue
        
        # Parse token (first 4 bytes)
        token = struct.unpack_from('<I', encoded, 0)[0]
        
        # Look up format string
        if token in token_db:
            format_str = token_db[token]
            print(f"[{message_count}] Token 0x{token:08X}: {format_str}")
            # TODO: Decode varint-encoded arguments and format the string
            print(f"         Args: {encoded[4:].hex()} (decoding not yet implemented)")
        else:
            print(f"[{message_count}] Unknown token 0x{token:08X}")
            print(f"         Raw: {encoded.hex()}")


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    
    elf_path = Path(sys.argv[1])
    log_path = Path(sys.argv[2])
    
    if not elf_path.exists():
        print(f"Error: ELF file not found: {elf_path}")
        sys.exit(1)
    
    if not log_path.exists():
        print(f"Error: Log file not found: {log_path}")
        sys.exit(1)
    
    print(f"Extracting token database from {elf_path}...")
    token_db = extract_token_database(elf_path)
    print(f"Found {len(token_db)} tokenized strings\n")
    
    print(f"Decoding RTT log from {log_path}...")
    print("=" * 60)
    decode_rtt_log(log_path, token_db)


if __name__ == '__main__':
    main()
