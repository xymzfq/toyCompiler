
; ModuleID = 'main'
source_filename = "main"

@.str = private constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define internal void @echo(i64 %toPrint) {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i64 %toPrint)
  ret void
}

define internal i64 @main() {
entry:
  ret i64 0
}

