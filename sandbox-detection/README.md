# Sandbox Detection Techniques
Here are samples of code related to different techniques to detect if the binary is run inside a sandbox
## check_ram.cpp
Sandboxes usually has small RAM memory sizes so it can be used to identify sandboxes.
## cpuid_instruction.cpp
CPUID instruction is used to retrieve processor manufacturer information, if it is not Intel or AMD, it is likely a sandbox.
## uptime.cpp
Short system uptimes can indicate the use of sandboxes.
# References
- APT groups techniques
- https://github.com/Mr-Un1k0d3r/MaliciousMacroGenerator
