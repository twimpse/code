// polymorphic_demo.c - Working polymorphic example
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

// Simple function we want to make polymorphic
void target_function(void) {
    printf("Secret message: Hello World!\n");
    printf("Process ID: %d\n", getpid());
}

// Function to generate random key
unsigned char generate_key(void) {
    return (rand() % 254) + 1;
}

// Simple XOR encryption/decryption
void xor_buffer(unsigned char *buffer, size_t size, unsigned char key) {
    for(size_t i = 0; i < size; i++) {
        buffer[i] ^= key;
    }
}

// Generate simple decrypter stub
unsigned char* create_decrypter_stub(size_t code_size, unsigned char key, 
                                      size_t *stub_size) {
    // Simple stub: just XOR decrypt and jump
    *stub_size = 32; // Small fixed size for simplicity
    unsigned char *stub = malloc(*stub_size);
    memset(stub, 0x90, *stub_size); // Fill with NOPs initially
    
    size_t offset = 0;
    
    // mov rsi, [rip+offset] - load encrypted code address
    stub[offset++] = 0x48;
    stub[offset++] = 0x8D;
    stub[offset++] = 0x35;
    *((unsigned int*)(&stub[offset])) = 0x20; // offset to encrypted code
    offset += 4;
    
    // mov rcx, code_size
    stub[offset++] = 0x48;
    stub[offset++] = 0xC7;
    stub[offset++] = 0xC1;
    *((unsigned int*)(&stub[offset])) = code_size;
    offset += 4;
    
    // mov al, key
    stub[offset++] = 0xB0;
    stub[offset++] = key;
    
    // decryption loop:
    // xor [rsi], al
    stub[offset++] = 0x80;
    stub[offset++] = 0x36;
    stub[offset] = 0x00;
    offset++;
    
    // inc rsi
    stub[offset++] = 0x48;
    stub[offset++] = 0xFF;
    stub[offset++] = 0xC6;
    
    // loop back
    stub[offset++] = 0xE2;
    stub[offset++] = 0xF5; // jump back 11 bytes
    
    // jmp to decrypted code
    stub[offset++] = 0xFF;
    stub[offset++] = 0xE6;
    
    return stub;
}

// Get size of a function (approximate - works for small functions)
size_t get_function_size(void *func) {
    // This is a hack - a real implementation would use proper ELF parsing
    // For demo purposes, we'll use a safe estimate
    return 100; // Our target_function is smaller than this
}

int main() {
    printf("=== Polymorphic Program Demo ===\n\n");
    
    // Seed random
    srand(time(NULL));
    
    // Get original function code
    size_t func_size = get_function_size(target_function);
    unsigned char *func_code = malloc(func_size);
    memcpy(func_code, target_function, func_size);
    
    printf("Original function at address: %p\n", target_function);
    printf("Calling original function:\n");
    target_function();
    
    printf("\n--- Creating polymorphic version 1 ---\n");
    
    // Generate random key and create encrypted version
    unsigned char key1 = generate_key();
    printf("Encryption key: 0x%02X\n", key1);
    
    // Create encrypted copy
    unsigned char *encrypted_code = malloc(func_size);
    memcpy(encrypted_code, func_code, func_size);
    xor_buffer(encrypted_code, func_size, key1);
    
    // Create decrypter stub
    size_t stub_size;
    unsigned char *stub = create_decrypter_stub(func_size, key1, &stub_size);
    
    // Allocate executable memory
    size_t total_size = stub_size + func_size;
    unsigned char *exec_mem = mmap(NULL, total_size, 
                                    PROT_READ | PROT_WRITE | PROT_EXEC,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if(exec_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    
    // Assemble: stub + encrypted code
    memcpy(exec_mem, stub, stub_size);
    memcpy(exec_mem + stub_size, encrypted_code, func_size);
    
    // Fix the address in stub to point to the encrypted code
    // The stub expects the code at offset 0x20 from stub start
    unsigned char *code_ptr_in_stub = exec_mem + 0x0C; // Position of offset
    unsigned int offset_to_code = (unsigned int)(stub_size - 0x20);
    memcpy(code_ptr_in_stub, &offset_to_code, 4);
    
    printf("Executing polymorphic version 1:\n");
    void (*func_ptr)(void) = (void(*)(void))exec_mem;
    func_ptr(); // This will decrypt and execute
    
    printf("\n--- Creating polymorphic version 2 ---\n");
    
    // Create second version with different key
    unsigned char key2 = generate_key();
    printf("Encryption key: 0x%02X\n", key2);
    
    unsigned char *encrypted_code2 = malloc(func_size);
    memcpy(encrypted_code2, func_code, func_size);
    xor_buffer(encrypted_code2, func_size, key2);
    
    unsigned char *stub2 = create_decrypter_stub(func_size, key2, &stub_size);
    
    unsigned char *exec_mem2 = mmap(NULL, total_size, 
                                     PROT_READ | PROT_WRITE | PROT_EXEC,
                                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if(exec_mem2 == MAP_FAILED) {
        perror("mmap2 failed");
        return 1;
    }
    
    memcpy(exec_mem2, stub2, stub_size);
    memcpy(exec_mem2 + stub_size, encrypted_code2, func_size);
    
    code_ptr_in_stub = exec_mem2 + 0x0C;
    offset_to_code = (unsigned int)(stub_size - 0x20);
    memcpy(code_ptr_in_stub, &offset_to_code, 4);
    
    printf("Executing polymorphic version 2:\n");
    func_ptr = (void(*)(void))exec_mem2;
    func_ptr();
    
    // Cleanup
    free(func_code);
    free(encrypted_code);
    free(stub);
    free(encrypted_code2);
    free(stub2);
    munmap(exec_mem, total_size);
    munmap(exec_mem2, total_size);
    
    return 0;
}
