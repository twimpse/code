#!/usr/bin/env python3
"""
Simple shellcode XOR encrypter
Usage: python encrypt_shellcode.py <input_file> <key>
"""

import sys

def read_shellcode(filename):
    """Read shellcode from file (supports raw binary or \x format)"""
    with open(filename, 'r') as f:
        content = f.read().strip()
    
    # If it's in \x31\xc0\x50 format
    if '\\x' in content:
        content = content.replace('\\x', '')
        content = content.replace(' ', '')
        shellcode = bytes.fromhex(content)
    else:
        # Raw hex string like "31c050"
        shellcode = bytes.fromhex(content)
    
    return shellcode

def xor_encrypt(shellcode, key):
    """XOR encrypt shellcode with single byte key"""
    return bytes([b ^ key for b in shellcode])

def main():
    if len(sys.argv) < 3:
        print("Usage: python encrypt_shellcode.py <shellcode_file> <key>")
        print("Example: python encrypt_shellcode.py shellcode.txt 0xAA")
        print("\nShellcode file can contain:")
        print("  - \\x31\\xc0\\x50 format")
        print("  - 31c050 format (hex string)")
        sys.exit(1)
    
    # Get key from command line
    key_str = sys.argv[2]
    if key_str.startswith('0x'):
        key = int(key_str, 16)
    else:
        key = int(key_str)
    
    # Read and encrypt
    shellcode = read_shellcode(sys.argv[1])
    encrypted = xor_encrypt(shellcode, key)
    
    # Output in \x format
    output = ''.join(f'\\x{b:02x}' for b in encrypted)
    print(output)

if __name__ == "__main__":
    main()
