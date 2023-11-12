# VxWorks Password Hash Cracker

VxWorks 5 uses a proprietary password hashing algorithm.

This repo provides a multithreaded brute-forcer to crack those hashes.

## Dependencies
1. POSIX compliant operating system
2. `gcc`
3. `make`

## Building
1. `make`

## Usage
1. Hashing passwords: `./hash <plaintext_password>`
2. Bruteforce password hashes: `./brute <dictionary_file> <encrypted_password>`

**Note**: `dictionary_file` is expected to contain one password candidate per line.
