// polymorphic_demo.c - Educational polymorphic engine
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct {
    unsigned char *code;
    size_t size;
    unsigned char key;
    unsigned char garbage_count;
} PolymorphicBlock;

// Original function we want to protect/mutate
void original_function(void) {
    printf("Hello from polymorphic world!\n");
    printf("PID: %d\n", getpid());
}

// Generate random garbage instructions
void insert_garbage(unsigned char *buffer, size_t *offset, int count) {
    for(int i = 0; i < count; i++) {
        int type = rand() % 4;
        switch(type) {
            case 0: // NOP
                buffer[(*offset)++] = 0x90;
                break;
            case 1: // PUSH/POP random register
                buffer[(*offset)++] = 0x50 + (rand() % 8);
                buffer[(*offset)++] = 0x58 + (rand() % 8);
                break;
            case 2: // MOV reg, reg
                buffer[(*offset)++] = 0x89;
                buffer[(*offset)++] = 0xC0 + (rand() % 8) * 8 + (rand() % 8);
                break;
            case 3: // XOR with immediate (zero effect)
                buffer[(*offset)++] = 0x80;
                buffer[(*offset)++] = 0xF0 | (rand() % 8);
                buffer[(*offset)++] = rand() % 256;
                // Add matching XOR to cancel out
                buffer[(*offset)++] = 0x80;
                buffer[(*offset)++] = 0xF0 | (rand() % 8);
                buffer[(*offset)++] = rand() % 256;
                break;
        }
    }
}

// Encrypt code block with XOR
void encrypt_code(unsigned char *code, size_t size, unsigned char key) {
    for(size_t i = 0; i < size; i++) {
        code[i] ^= key;
    }
}

// Generate polymorphic decryptor stub
unsigned char* generate_decryptor(size_t code_size, unsigned char key, 
                                   size_t garbage_count, size_t *stub_size) {
    *stub_size = 64 + garbage_count * 10; // Rough estimate
    unsigned char *stub = malloc(*stub_size);
    size_t offset = 0;
    
    // Prologue with garbage
    insert_garbage(stub, &offset, garbage_count / 3);
    
    // XOR decryption loop
    // mov rsi, code_address
    stub[offset++] = 0xBE; // mov esi
    unsigned int addr = 0x41414141; // placeholder
    memcpy(&stub[offset], &addr, 4);
    offset += 4;
    
    insert_garbage(stub, &offset, garbage_count / 3);
    
    // mov rcx, code_size
    stub[offset++] = 0xB9; // mov ecx
    memcpy(&stub[offset], &code_size, 4);
    offset += 4;
    
    // mov al, key
    stub[offset++] = 0xB0; // mov al
    stub[offset++] = key;
    
    insert_garbage(stub, &offset, garbage_count / 3);
    
    // decryption_loop:
    // xor [rsi], al
    stub[offset++] = 0x80;
    stub[offset++] = 0x36;
    stub[offset++] = 0x00;
    // inc rsi
    stub[offset++] = 0x48;
    stub[offset++] = 0xFF;
    stub[offset++] = 0xC6;
    // loop decryption_loop
    stub[offset++] = 0xE2;
    stub[offset++] = 0xF8;
    
    insert_garbage(stub, &offset, garbage_count / 3);
    
    // jmp to decrypted code
    stub[offset++] = 0xFF;
    stub[offset++] = 0xE6; // jmp rsi
    
    // Fix offset for actual stub size
    *stub_size = offset;
    return stub;
}

// Create polymorphic version of a function
PolymorphicBlock* create_polymorphic(void (*func)(void), size_t func_size) {
    PolymorphicBlock *block = malloc(sizeof(PolymorphicBlock));
    srand(time(NULL));
    
    block->key = rand() % 255 + 1;
    block->garbage_count = rand() % 20 + 5;
    
    // Copy original function
    block->code = malloc(func_size);
    memcpy(block->code, func, func_size);
    block->size = func_size;
    
    // Encrypt it
    encrypt_code(block->code, block->size, block->key);
    
    return block;
}

// Execute polymorphic code
void execute_polymorphic(PolymorphicBlock *block) {
    size_t stub_size;
    unsigned char *stub = generate_decryptor(block->size, block->key, 
                                               block->garbage_count, &stub_size);
    
    // Allocate executable memory
    void *exec_mem = mmap(NULL, stub_size + block->size, 
                          PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    // Copy decrypted code after stub
    memcpy(exec_mem, stub, stub_size);
    memcpy((unsigned char*)exec_mem + stub_size, block->code, block->size);
    
    // Fix up address in stub (simplified - would need proper relocation)
    // This is where you'd calculate actual code address
    
    // Execute
    void (*entry)(void) = (void(*)(void))exec_mem;
    entry();
    
    munmap(exec_mem, stub_size + block->size);
    free(stub);
}

int main() {
    printf("=== Polymorphic Engine Demo ===\n\n");
    
    // Estimate function size (in practice, get from linker or calculate)
    size_t func_size = 128; // Approximate
    
    // Original call
    printf("Original function:\n");
    original_function();
    printf("Function address: %p\n\n", original_function);
    
    // Create polymorphic version (different each run)
    PolymorphicBlock *block1 = create_polymorphic(original_function, func_size);
    printf("Polymorphic version 1 (key: 0x%02X, garbage: %zu):\n", 
           block1->key, block1->garbage_count);
    execute_polymorphic(block1);
    
    printf("\n");
    
    // Create another polymorphic version (different structure)
    PolymorphicBlock *block2 = create_polymorphic(original_function, func_size);
    printf("Polymorphic version 2 (key: 0x%02X, garbage: %zu):\n", 
           block2->key, block2->garbage_count);
    execute_polymorphic(block2);
    
    free(block1->code);
    free(block1);
    free(block2->code);
    free(block2);
    
    return 0;
}
