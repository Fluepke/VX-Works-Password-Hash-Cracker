#include <stdio.h>
#include <string.h>
#include <stdint.h>


int loginDefaultEncrypt(char *plaintextPwIn, char *encryptedPwOut) {
    char *pcVar1;
    size_t inputPwLen;
    unsigned int i;
    int iVar2;

    iVar2 = 0;
    inputPwLen = strlen(plaintextPwIn);
    
    if ((7 < inputPwLen) && (inputPwLen = strlen(plaintextPwIn), inputPwLen < 41)) {
        i = 0;
        while (inputPwLen = strlen(plaintextPwIn), i < inputPwLen) {
            pcVar1 = plaintextPwIn + i;
            i = i + 1;
            iVar2 = iVar2 + ((int)*pcVar1 * i ^ i);
        }
        
        sprintf(encryptedPwOut, "%u", iVar2 * 0x1e3a1d5);
        
        for (i = 0; inputPwLen = strlen(encryptedPwOut), i < inputPwLen; i = i + 1) {
            if (encryptedPwOut[i] < '3') {
                encryptedPwOut[i] = encryptedPwOut[i] + '!';
            }
            if (encryptedPwOut[i] < '7') {
                encryptedPwOut[i] = encryptedPwOut[i] + '/';
            }
            if (encryptedPwOut[i] < '9') {
                encryptedPwOut[i] = encryptedPwOut[i] + 'B';
            }
        }
        
        return 0;
    }

    return -1;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <plaintext_password>\n", argv[0]);
    }
    char* plaintext = argv[1];
    char encrypted[50];  // Adjust the size accordingly

    int result = loginDefaultEncrypt(plaintext, encrypted);

    if (result == 0) {
        printf("Hashed Password: %s\n", encrypted);
    } else {
        printf("Hashing failed\n");
    }

    return 0;
}

