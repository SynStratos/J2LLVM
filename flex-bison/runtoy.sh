./toy < input &> output.ll
llvm-as-6.0 output.ll
lli-6.0 output.bc
