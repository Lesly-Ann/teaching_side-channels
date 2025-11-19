#ifndef VICTIM_MEMCMP_H
#define VICTIM_MEMCMP_H

#define SECRET_PWD      "m}f" // Obfuscated password
#define OBFUSCATE       "XOR"

#define DELAY 1 // Add artificial delay in victim comparison

// Initialize the secret password (deobfuscate it once)
void victim_init(void);

// Read password guess from user (allocates; caller must free)
char *read_from_user(void);

// Check password; returns 1 if correct, 0 otherwise
int check_pwd(char *user, int user_len);

void print_success_art();

#endif // VICTIM_MEMCMP_H
