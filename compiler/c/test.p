define internal i64 @pp(i64 %a1) {
entry:
  %a = alloca i64
  store i64 %a1, i64* %a
  %0 = load i64, i64* %a
  ret i64 %0
}

define internal i64 @do_math(i64 %a1) {
entry:
  %a = alloca i64
  store i64 %a1, i64* %a
  %x = alloca i64
  %0 = load i64, i64* %a
  %1 = mul i64 %0, 5
  store i64 %1, i64* %x
  br i1 true, label %then, label %else

then:                                             ; preds = %entry
  %2 = call i64 @pp(i64 5)
  br label %ifcont

else:                                             ; preds = %entry
  %3 = call i64 @pp(i64 6)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  ret i64 0
}

define internal i64 @main() {
entry:
  %0 = call i64 @do_math(i64 5)
  ret i64 0
}

