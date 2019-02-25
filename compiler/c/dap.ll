; ModuleID = 'mul.cpp'  
source_filename = "mul.cpp"  
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"  
target triple = "x86_64-unknown-linux-gnu"  
  
; Function Attrs: nounwind uwtable  
define i64 @_Z4imulll(i64, i64) #0 {  
  %3 = alloca i64, align 8  
  %4 = alloca i64, align 8  
  store i64 %0, i64* %3, align 8  
  store i64 %1, i64* %4, align 8  
  %5 = load i64, i64* %3, align 8  
  %6 = load i64, i64* %4, align 8  
  %7 = mul nsw i64 %5, %6  
  ret i64 %7  
}  
  
attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }  
  
!llvm.ident = !{!0}  
  
!0 = !{!"clang version 3.9.0 (tags/RELEASE_390/final)"}  
