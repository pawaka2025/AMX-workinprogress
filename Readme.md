# AMX Exploration

## What is AMX?

AMX (Advanced Matrix Extensions) is a SIMD extension that enables matrix-wise computation of multiple parallel operations. For instance, an AB + Y operation involving a pair of matrices consisting of 16 x 64 `u8` types can be performed in a single computation step.

This has significant implications due to the massive performance improvements compared to AVX-512. However, several challenges remain:

### Challenges:

1. **Limited Intrinsics**  
   - The currently available AMX intrinsics are extremely limited in variety.  
   - Dot-product operations exist, but arithmetic, boolean, bitwise, and other operations are absent.

2. **Rigid Matrix Ratio Constraint**  
   - There is an enforced **1:4 ratio** between rows and columns, which I am still investigating.

3. **Result Matrix Size Reduction**  
   - The dot-product intrinsic forces the resulting matrix to be **75% smaller** than the operand matrices.  
   - This poses issues when working with perfectly square matrices.

4. **Restricted Data Types**  
   - AMX does not provide flexibility in using types beyond those explicitly defined in the intrinsics (only two types so far).

## Purpose of This Repository

This repository serves as:
- A refresher for myself to continue AMX exploration at a later date.
- A starting point for others interested in learning about AMX and its intricacies.
- A primer for me to work on hopefully devloping custom-made AMX instructions via C, Assembly or even lower-level code ie machine.

The code here is adapted from Intel’s AMX sample code:
[https://github.com/intel/AMX-TMUL-Code-Samples/blob/main/src/test-amxtile.c](https://github.com/intel/AMX-TMUL-Code-Samples/blob/main/src/test-amxtile.c)

## License

This project is released under the MIT License.

