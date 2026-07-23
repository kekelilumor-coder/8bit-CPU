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

| Mnemonic | Opcode (5-bit) | Format | Description |
|---|---|---|---|
| ADD | `00000` | R-type | Rd = Rs1 + Rs2 |
| SUB | `00001` | R-type | Rd = Rs1 - Rs2 |
| HALT | `00010` | R-type (degenerate - dest/src1/src2 fields unused) | Stop execution |
| MOV | `00011` | R-type (degenerate - src2 field unused) | Rd = Rs1 |
| LOADI | `01000` | I-type | Rd = immediate (signed, -128 to 127) |
| ADDI | `01001` | I-type (destination doubles as source) | Rd = Rd + immediate |
| LOAD | `10000` | Indirect-type | Rd = memory[Rs + offset] |
| STORE | `10001` | Indirect-type | memory[Rs + offset] = Rd |
| *(reserved)* | `00100`–`00111` | R-type range | Reserved for future R-type ops |
| *(reserved)* | `01010`–`01111` | I-type range | Reserved for future I-type ops |
| *(reserved)* | `10010`–`11111` | Indirect-type / branch range | Reserved for jump/branch instructions once flags are designed |

---

## 6. Flags / Status Register

*TBD — depends on which ALU operations need to set which flags (zero, carry,
negative, overflow), and how conditional branch instructions will test flag
state. To be designed after the opcode table, since branch instruction
encoding depends on both the available opcode space and the flag design.*

---

## 7. Design Decisions Log

This is a running log of decisions in the order that they were made, with the alternative considered and the constraint that forced the choice. Relevant sections can be referred to fo more detailed reasoning. 

1. **Fixed-length 16-bit instructions** used instead of variable-length: simpler to decode and more appropriate for first custom ISA. **(Section 1)**


2. **8 general-purpose registers with 3 bit addressing** over 16: bit budget doesn't support 16 registers in a 3-operand R-type format. **(Section 2)**

3. **Three addressing modes (immediate, register-direct, register-indirect)** and omitting absolute/direct: direct addressing doesn't fit the bit budget and is not required given the other three modes. **(Section 4)**

4. **Multiple instruction formats selected by opcode (R-type/I-type/Indirect-type)** instead of one uniform format: avoids wasting bits on unused fields. **(Section 3)**

5. **8-bit immediate field:** derived directly from remaining bits after opcode + destination register. **(Section 3)**

6. **5-bit signed offset added to indirect addressing:** makes use of unused bits. Influenced by bubble sort's need to access adjacent array elements without recomputing addresses. **(Section 3)**

7. **No dedicated MUL, no shift instructions (LSL/LSR/ASR/XSR):**  factorial's multiplication is implemented through repeated addition using ADD. Shift-and-add multiplication is a real hardware technique but adds unrequired opcode and implementation complexity. **(Section 5)**

8. **No bitwise logic (AND/OR/XOR/NOT):** none of the test programs need bit manipulation, therefore not added. **(Section 5)**

9. **No NOP:** not currently requirement (no pipeline/timing constraints in a pure emulator). Can be added if the project is extended toward hardware timing simulation. **(Section 5)**

10. **HALT and MOV treated as degenerate R-type instructions** rather than introducing a fourth zero/one-operand format: keeps the decoder handling only three instruction shapes. **(Section 5)**

11. **Opcodes grouped by format** (`000xx`=R-type, `010xx`=I-type, `10xxx`= Indirect-type/branch) rather than assigned sequentially, so the opcode's high bits alone can indicate instruction format before full decode. **(Section 5)**

---

## 8. Memory Model

*TBD, Current intention: 64KB, byte-addressable, 16-bit address space.*
