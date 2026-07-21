# 8-Bit Custom CPU - Instruction Set Architecture

## 1. Overview

This is a custom designed 8-bit CPU architecture built for a software emulator project. This ISA is intended to be implemented later on through an FPGA/Verilog hardware implementation. This ISA is designed from scratch rather than copying an existing architecture (eg RISC-V or ARM) in order to experience architectural decision making for myself. 

This ISA is validated against 3 test programs: Fibonacci sequence generation, factorial computation and a standard bubble sort. These effectively show the functionality of the ISA, demonstrating  arithmetic capability, loop-based control flow and memory/array access patterns respectively. The bubble sort in particular influenced the decision to include a signed offset in indirect addressing, spoken about in Section 3.

### Basic design decision
- **Data width:** 8-bit
- **Instruction width:** 16-bit, fixed length
- **Memory:** 64KB, byte-addressable
- **Philosophy:** RISC-style, fixed length instructions with simple decode and load/store architecture. ALU operations only work on registers. Memory is only accessed using load and store instructions, not ALU operations.

**Fixed length over variable length:** Although variable-length instructions, such as **x86**, allow denser code, it requires a much more complex decoder that has to determine instruction length before it can be parsed. Fixed-length instructions allow every instruction to be decoded in an identical matter regardless of type, resulting in a simple fetch-decode-execute cycle. This decision RISC vs CISC decision in real architectures.

---

## 2. Register File

- **8 General-Purpose Registers:** R0 - R7, 3-bit register addressing
- [TBD: special-purpose registers — program counter, stack pointer, flags register]

### Rejected alternatives
- **16 registers:** With a 3-operand R type format (destination, source1, source2), 16 registers require 4 bits x 3 = 12 bits for just the register fields. In conjunction with 5-bit opcodes, there would be 0 bits remaining for addressing modes or immediates, meaning instructions won't fit in the 16 bit budget. 8 registers require 3 bits x 3 = 9 bits, leaving space for opcode and other fields to comfortably fit.

- **4 registers:** Allows for more bits per instruction, but could result in unnecessary register pressure (frequent spills to memory) for even moderately complicated test programs. Using 8 registers strikes a balance between encoding cost and having enough working registers for fibonacci, factorial and bubble sort without constant memory traffic.

---

## 3. Instruction Format

Each instruction is 16 bits. The fields the non-opcode bits represents are dependent on the instruction type. First, the decoder reads the opcode to determine which of the 3 formats to interpret the rest oc the instruction as. This pattern occurs in real life ISAs, such as MIPS R-type/I-type/J-type (Register, Immediates, Jumps).

### R-type (register-direct ALU operations, e.g. ADD Rd, Rs1, Rs2)
```
[15:11] opcode           (5 bits)
[10:8]  destination      (3 bits)
[7:5]   source1          (3 bits)
[4:2]   source2          (3 bits)
[1:0]   unused           (2 bits)
```

### I-type (immediate load, e.g. LOADI Rd, imm)
```
[15:11] opcode            (5 bits)
[10:8]  destination       (3 bits)
[7:0]   immediate         (8 bits, signed two's complement)
```
Uses all 16 bits with zero waste. Immediate width of 8 bits maximises 16 bit instruction length with the opcode length of 5 bits and destination register width 3 bit width. Gives a signed range of -128 to 127, sufficient for loop counters and constants in the planned test programs.

### Indirect-type (register-indirect memory access with offset, e.g. LOAD Rd, [Rs + offset])
```
[15:11] opcode           (5 bits)
[10:8]  destination      (3 bits)
[7:5]   Source Address   (3 bits)
[4:0]   offset           (5 bits, signed two's complement, range -16 to +15)
```
Uses all 16 bits with zero waste.

**Base + offset instead of simple register-indirect:** A standard register indirect format leaves 5 bits unused in the indirect-type instruction. Rather than leaving them reserved, they're used as a signed offset, enabling `LOAD Rd, [Rs + offset]` instead of just `LOAD Rd, [Rs]`. This directly supports bubble sort, which needs to compare adjacent array elements (`arr[i]`, `arr[i+1]`). An offset field allows for two LOADs with offsets 0 and +1 against a single base register, rather than recomputing the address with a separate ADD every iteration.

---

## 4. Addressing Modes

| Mode | Example | Description |
|---|---|---|
| Immediate | `LOADI Rd, #42` | Operand is a literal value encoded in the instruction |
| Register-direct | `ADD Rd, Rs1, Rs2` | Operands are values held in registers |
| Register-indirect (base+offset) | `LOAD Rd, [Rs + offset]` | Operand is in memory, at address = value in Rs plus signed offset |

**Exclusion of absolute/direct addressing:** A direct-addressing instruction requires enough bits to encode a full memory address (16 bits for a 64KB address space), which doesn't fit inside a 16-bit instruction alongside an opcode and destination register. To implement this, the addressable range would need to be shrunk (zero-page style) or a variable-length instruction format would be required. It was also unnecessary as any direct address access is already expressible with `LOADI Rn, addr` followed by `LOAD Rd, [Rn]`, whilst only requiring one extra instruction. Since no planned test program requires direct addressing, the extra complexity wasn't justified.

---

## 5. Instruction Set Table

*TBD — to be filled in as opcodes are assigned. Table format:*

| Mnemonic | Opcode | Format | Description |
|---|---|---|---|
| ADD | TBD | R-type | Rd = Rs1 + Rs2 |
| LOADI | TBD | I-type | Rd = immediate |
| LOAD | TBD | Indirect-type | Rd = memory[Rs + offset] |
| STORE | TBD | Indirect-type | memory[Rs + offset] = Rd |
| ... | | | |

---

## 6. Flags / Status Register

*TBD — depends on which ALU operations need to set which flags (zero, carry,
negative, overflow), and how conditional branch instructions will test flag
state. To be designed after the opcode table, since branch instruction
encoding depends on both the available opcode space and the flag design.*

---

## 7. Design Decisions Log

This is a running log of decisions in the order that they were made, with the alternative considered and the constraint that forced the choice. (See inline reasoning in each section above — this log exists as a chronological cross-reference.)

1. **Fixed-length 16-bit instructions** over variable-length — simpler decode,
   standard for a first custom ISA (Section 1)
2. **8 general-purpose registers, 3-bit addressing** over 16 — bit budget
   doesn't support 16 registers in a 3-operand R-type format (Section 2)
3. **Three addressing modes (immediate, register-direct, register-indirect)**
   over adding absolute/direct — direct addressing doesn't fit the bit budget
   and is redundant given the other three modes (Section 4)
4. **Multiple instruction formats selected by opcode (R-type/I-type/Indirect-type)**
   over one uniform format — avoids wasting bits on unused fields, standard
   practice in real ISAs (Section 3)
5. **8-bit immediate field** — derived directly from remaining bits after
   opcode + dest register, not copied from external precedent (Section 3)
6. **5-bit signed offset added to indirect addressing** — turns unused bits
   into a real capability, directly motivated by bubble sort's need to access
   adjacent array elements without recomputing addresses (Section 3)

---

## 8. Memory Model

*TBD, Current intention: 64KB, byte-addressable, 16-bit address space.*
