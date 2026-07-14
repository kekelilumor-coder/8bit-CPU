# 8-Bit Custom CPU - Instruction Set Architecture

## 1. Overview

This is a cutom designed 8-bit CPU architecture built for a software emulator project. This ISA is intended to be implemented later on through an FPGA/Verilog hardware implementation. This ISA is designed from scratch rather than copying an existing architecture (eg RISC-V or ARM) in order to experience architectural descision making for myslef. 

### Basic design descision
- **Data width:** 8-bit
- **Instruction width:** 16-bit, fixed length
- **Memory:** 64KB, byte-addressable
- **Philosophy:**