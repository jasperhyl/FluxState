; ModuleID = 'test/test.fs'
source_filename = "test/test.fs"
target triple = "x86_64-unknown-linux-gnu"

%SyntaxCoverage.machine = type { %FS_MachineHeader, %SyntaxCoverage.env }
%FS_MachineHeader = type { i32, i32, i64, ptr, ptr, ptr }
%SyntaxCoverage.env = type { i32, i32, i16, float, double, i32, i8, i8, %FS_String, %FS_String }
%FS_String = type { ptr, i64 }
%Secondary.machine = type { %FS_MachineHeader, %Secondary.env }
%Secondary.env = type { i32, i16, i8, %FS_String }
%FS_Event = type { i32, i32, i16, i32, ptr, i64 }
%SyntaxCoverage.event.Configure.payload = type { i32, i16, float, double, i32, i8, %FS_String }
%SyntaxCoverage.event.Reset.payload = type { %FS_String }
%SyntaxCoverage.event.Dispatch.payload = type { i32, i32, i8 }
%Secondary.event.Start.payload = type { i32 }
%Secondary.event.Flip.payload = type { i8, %FS_String }
%SyntaxCoverage.event.Finish.payload = type { i8, %FS_String }

@fs_machine_SyntaxCoverage = internal global %SyntaxCoverage.machine zeroinitializer
@fs_machine_Secondary = internal global %Secondary.machine zeroinitializer
@fs.str = private constant [5 x i8] c"armed"
@fs.str.1 = private constant [10 x i8] c"configured"
@fs.str.2 = private constant [6 x i8] c"nested"
@fs.str.3 = private constant [7 x i8] c"cooling"
@fs.str.4 = private constant [4 x i8] c"done"
@fs.str.5 = private constant [5 x i8] c"error"
@fs.str.6 = private constant [4 x i8] c"loop"
@fs.str.7 = private constant [4 x i8] c"boot"
@fs.str.8 = private constant [4 x i8] c"idle"
@fs.str.9 = private constant [7 x i8] c"started"
@fs.str.10 = private constant [7 x i8] c"restart"
@fs.str.11 = private constant [9 x i8] c"secondary"

declare void @fs_enqueue_event(ptr, ptr)

declare void @fs_start_timer(ptr, i32, i64, i64)

declare void @fs_cancel_timer(ptr, i32)

declare i1 @fs_init_machine(ptr, i32, ptr)

declare void @fs_cleanup_machine(ptr)

declare i1 @fs_runtime_step(ptr)

declare void @fs_runtime_run(ptr)

declare void @fs_stop_machine(ptr)

declare ptr @malloc(i64)

define internal i1 @fs_guard_SyntaxCoverage_1(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 0
  %code = load i32, ptr %2, align 4
  %3 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 1
  %retry = load i16, ptr %3, align 2
  %4 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 2
  %scale = load float, ptr %4, align 4
  %5 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 3
  %limit = load double, ptr %5, align 8
  %6 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 4
  %tag = load i32, ptr %6, align 4
  %7 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 5
  %enabled_flag = load i8, ptr %7, align 1
  %8 = trunc i8 %enabled_flag to i1
  %9 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 6
  %name = load %FS_String, ptr %9, align 8
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  %10 = load i8, ptr %enabled.addr, align 1
  %11 = trunc i8 %10 to i1
  %12 = icmp eq i1 %8, true
  %13 = and i1 %11, %12
  ret i1 %13
}

define internal i1 @fs_guard_SyntaxCoverage_2(ptr %0, ptr %1) {
entry:
  %peer.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env, i32 0, i32 0
  %2 = load i32, ptr %peer.addr, align 4
  %3 = icmp sgt i32 %2, 0
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  %4 = load i8, ptr %enabled.addr, align 1
  %5 = trunc i8 %4 to i1
  %6 = and i1 %3, %5
  %7 = or i1 %6, false
  ret i1 %7
}

define internal i1 @fs_guard_SyntaxCoverage_5(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %SyntaxCoverage.event.Reset.payload, ptr %event.payload, i32 0, i32 0
  %reason = load %FS_String, ptr %2, align 8
  %peer.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env, i32 0, i32 0
  %3 = load i32, ptr %peer.addr, align 4
  %4 = icmp ne i32 %3, 0
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  %5 = load i8, ptr %enabled.addr, align 1
  %6 = trunc i8 %5 to i1
  %7 = icmp eq i1 %6, false
  %8 = or i1 %4, %7
  ret i1 %8
}

define internal void @fs_action_SyntaxCoverage_0(ptr %0, ptr %1) {
entry:
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  store i1 true, ptr %enabled.addr, align 1
  %latched.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %latched.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %latched.env, i32 0, i32 7
  store i1 false, ptr %latched.addr, align 1
  %label.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %label.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %label.env, i32 0, i32 8
  store %FS_String { ptr @fs.str, i64 5 }, ptr %label.addr, align 8
  %peer.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env, i32 0, i32 0
  store i32 3, ptr %peer.addr, align 4
  %sink.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %sink.env, i32 0, i32 1
  %peer.env1 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr2 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env1, i32 0, i32 0
  %2 = load i32, ptr %peer.addr2, align 4
  %retry_count.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %retry_count.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %retry_count.env, i32 0, i32 2
  %3 = load i16, ptr %retry_count.addr, align 2
  %4 = sext i16 %3 to i32
  %5 = add i32 %2, %4
  %6 = mul i32 %5, 2
  store i32 %6, ptr %sink.addr, align 4
  ret void
}

define internal void @fs_action_SyntaxCoverage_1(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 0
  %code = load i32, ptr %2, align 4
  %3 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 1
  %retry = load i16, ptr %3, align 2
  %4 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 2
  %scale = load float, ptr %4, align 4
  %5 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 3
  %limit = load double, ptr %5, align 8
  %6 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 4
  %tag = load i32, ptr %6, align 4
  %7 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 5
  %enabled_flag = load i8, ptr %7, align 1
  %8 = trunc i8 %enabled_flag to i1
  %9 = getelementptr inbounds nuw %SyntaxCoverage.event.Configure.payload, ptr %event.payload, i32 0, i32 6
  %name = load %FS_String, ptr %9, align 8
  %peer.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env, i32 0, i32 0
  %10 = sext i16 %retry to i32
  %11 = mul i32 %10, 2
  %12 = add i32 %code, %11
  %13 = sub i32 %12, 3
  store i32 %13, ptr %peer.addr, align 4
  %retry_count.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %retry_count.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %retry_count.env, i32 0, i32 2
  store i16 %retry, ptr %retry_count.addr, align 2
  %ratio.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %ratio.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %ratio.env, i32 0, i32 3
  %14 = fadd float %scale, 1.000000e+00
  store float %14, ptr %ratio.addr, align 4
  %threshold.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %threshold.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %threshold.env, i32 0, i32 4
  %15 = fdiv double %limit, 2.000000e+00
  store double %15, ptr %threshold.addr, align 8
  %marker.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %marker.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %marker.env, i32 0, i32 5
  store i32 %tag, ptr %marker.addr, align 4
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  store i1 %8, ptr %enabled.addr, align 1
  %latched.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %latched.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %latched.env, i32 0, i32 7
  %enabled.env1 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr2 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env1, i32 0, i32 6
  %16 = load i8, ptr %enabled.addr2, align 1
  %17 = trunc i8 %16 to i1
  %18 = and i1 %8, %17
  store i1 %18, ptr %latched.addr, align 1
  %label.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %label.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %label.env, i32 0, i32 8
  store %FS_String %name, ptr %label.addr, align 8
  %note.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %note.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %note.env, i32 0, i32 9
  store %FS_String { ptr @fs.str.1, i64 10 }, ptr %note.addr, align 8
  %machine.hdr = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %machine.hdr3 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %19 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Dispatch.payload, ptr null, i32 1) to i64))
  %20 = getelementptr inbounds nuw %SyntaxCoverage.event.Dispatch.payload, ptr %19, i32 0, i32 0
  store i32 %code, ptr %20, align 4
  %21 = getelementptr inbounds nuw %SyntaxCoverage.event.Dispatch.payload, ptr %19, i32 0, i32 1
  %22 = sext i16 %retry to i32
  store i32 %22, ptr %21, align 4
  %enabled.env4 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr5 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env4, i32 0, i32 6
  %23 = load i8, ptr %enabled.addr5, align 1
  %24 = trunc i8 %23 to i1
  %25 = getelementptr inbounds nuw %SyntaxCoverage.event.Dispatch.payload, ptr %19, i32 0, i32 2
  %26 = zext i1 %24 to i8
  store i8 %26, ptr %25, align 1
  %27 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %28 = getelementptr inbounds nuw %FS_Event, ptr %27, i32 0, i32 0
  store i32 2, ptr %28, align 4
  %29 = getelementptr inbounds nuw %FS_Event, ptr %27, i32 0, i32 1
  store i32 0, ptr %29, align 4
  %30 = getelementptr inbounds nuw %FS_Event, ptr %27, i32 0, i32 2
  store i16 0, ptr %30, align 2
  %31 = getelementptr inbounds nuw %FS_Event, ptr %27, i32 0, i32 3
  store i32 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Dispatch.payload, ptr null, i32 1) to i32), ptr %31, align 4
  %32 = getelementptr inbounds nuw %FS_Event, ptr %27, i32 0, i32 4
  store ptr %19, ptr %32, align 8
  %33 = getelementptr inbounds nuw %FS_Event, ptr %27, i32 0, i32 5
  store i64 0, ptr %33, align 4
  call void @fs_enqueue_event(ptr %machine.hdr3, ptr %27)
  %machine.hdr6 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %34 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%Secondary.event.Start.payload, ptr null, i32 1) to i64))
  %35 = getelementptr inbounds nuw %Secondary.event.Start.payload, ptr %34, i32 0, i32 0
  store i32 %code, ptr %35, align 4
  %36 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %37 = getelementptr inbounds nuw %FS_Event, ptr %36, i32 0, i32 0
  store i32 0, ptr %37, align 4
  %38 = getelementptr inbounds nuw %FS_Event, ptr %36, i32 0, i32 1
  store i32 0, ptr %38, align 4
  %39 = getelementptr inbounds nuw %FS_Event, ptr %36, i32 0, i32 2
  store i16 0, ptr %39, align 2
  %40 = getelementptr inbounds nuw %FS_Event, ptr %36, i32 0, i32 3
  store i32 ptrtoint (ptr getelementptr (%Secondary.event.Start.payload, ptr null, i32 1) to i32), ptr %40, align 4
  %41 = getelementptr inbounds nuw %FS_Event, ptr %36, i32 0, i32 4
  store ptr %34, ptr %41, align 8
  %42 = getelementptr inbounds nuw %FS_Event, ptr %36, i32 0, i32 5
  store i64 0, ptr %42, align 4
  call void @fs_enqueue_event(ptr @fs_machine_Secondary, ptr %36)
  %machine.hdr7 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %43 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%Secondary.event.Flip.payload, ptr null, i32 1) to i64))
  %enabled.env8 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr9 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env8, i32 0, i32 6
  %44 = load i8, ptr %enabled.addr9, align 1
  %45 = trunc i8 %44 to i1
  %46 = getelementptr inbounds nuw %Secondary.event.Flip.payload, ptr %43, i32 0, i32 0
  %47 = zext i1 %45 to i8
  store i8 %47, ptr %46, align 1
  %48 = getelementptr inbounds nuw %Secondary.event.Flip.payload, ptr %43, i32 0, i32 1
  store %FS_String %name, ptr %48, align 8
  %49 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %50 = getelementptr inbounds nuw %FS_Event, ptr %49, i32 0, i32 0
  store i32 2, ptr %50, align 4
  %51 = getelementptr inbounds nuw %FS_Event, ptr %49, i32 0, i32 1
  store i32 0, ptr %51, align 4
  %52 = getelementptr inbounds nuw %FS_Event, ptr %49, i32 0, i32 2
  store i16 0, ptr %52, align 2
  %53 = getelementptr inbounds nuw %FS_Event, ptr %49, i32 0, i32 3
  store i32 ptrtoint (ptr getelementptr (%Secondary.event.Flip.payload, ptr null, i32 1) to i32), ptr %53, align 4
  %54 = getelementptr inbounds nuw %FS_Event, ptr %49, i32 0, i32 4
  store ptr %43, ptr %54, align 8
  %55 = getelementptr inbounds nuw %FS_Event, ptr %49, i32 0, i32 5
  store i64 0, ptr %55, align 4
  call void @fs_enqueue_event(ptr @fs_machine_Secondary, ptr %49)
  %peer.env10 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr11 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env10, i32 0, i32 0
  %56 = sext i16 %retry to i32
  %57 = add i32 %code, %56
  %58 = sext i16 %retry to i32
  %59 = sub i32 %58, 1
  %60 = mul i32 %57, %59
  store i32 %60, ptr %peer.addr11, align 4
  %sink.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %sink.env, i32 0, i32 1
  %peer.env12 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr13 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env12, i32 0, i32 0
  %61 = load i32, ptr %peer.addr13, align 4
  store i32 %61, ptr %sink.addr, align 4
  %note.env14 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %note.addr15 = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %note.env14, i32 0, i32 9
  store %FS_String { ptr @fs.str.2, i64 6 }, ptr %note.addr15, align 8
  ret void
}

define internal void @fs_action_SyntaxCoverage_2(ptr %0, ptr %1) {
entry:
  %label.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %label.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %label.env, i32 0, i32 8
  store %FS_String { ptr @fs.str.3, i64 7 }, ptr %label.addr, align 8
  %latched.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %latched.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %latched.env, i32 0, i32 7
  store i1 true, ptr %latched.addr, align 1
  ret void
}

define internal void @fs_action_SyntaxCoverage_3(ptr %0, ptr %1) {
entry:
  %note.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %note.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %note.env, i32 0, i32 9
  store %FS_String { ptr @fs.str.4, i64 4 }, ptr %note.addr, align 8
  %machine.hdr = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %machine.hdr1 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %2 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %3 = getelementptr inbounds nuw %FS_Event, ptr %2, i32 0, i32 0
  store i32 0, ptr %3, align 4
  %4 = getelementptr inbounds nuw %FS_Event, ptr %2, i32 0, i32 1
  store i32 0, ptr %4, align 4
  %5 = getelementptr inbounds nuw %FS_Event, ptr %2, i32 0, i32 2
  store i16 0, ptr %5, align 2
  %6 = getelementptr inbounds nuw %FS_Event, ptr %2, i32 0, i32 3
  store i32 0, ptr %6, align 4
  %7 = getelementptr inbounds nuw %FS_Event, ptr %2, i32 0, i32 4
  store ptr null, ptr %7, align 8
  %8 = getelementptr inbounds nuw %FS_Event, ptr %2, i32 0, i32 5
  store i64 0, ptr %8, align 4
  call void @fs_enqueue_event(ptr %machine.hdr1, ptr %2)
  ret void
}

define internal void @fs_action_SyntaxCoverage_4(ptr %0, ptr %1) {
entry:
  %peer.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env, i32 0, i32 0
  store i32 -1, ptr %peer.addr, align 4
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  store i1 false, ptr %enabled.addr, align 1
  %latched.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %latched.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %latched.env, i32 0, i32 7
  store i1 false, ptr %latched.addr, align 1
  %label.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %label.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %label.env, i32 0, i32 8
  store %FS_String { ptr @fs.str.5, i64 5 }, ptr %label.addr, align 8
  ret void
}

define internal void @fs_action_SyntaxCoverage_5(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %SyntaxCoverage.event.Reset.payload, ptr %event.payload, i32 0, i32 0
  %reason = load %FS_String, ptr %2, align 8
  %label.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %label.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %label.env, i32 0, i32 8
  store %FS_String %reason, ptr %label.addr, align 8
  %peer.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %peer.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %peer.env, i32 0, i32 0
  store i32 0, ptr %peer.addr, align 4
  %sink.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %sink.env, i32 0, i32 1
  store i32 0, ptr %sink.addr, align 4
  ret void
}

define internal void @fs_action_SyntaxCoverage_6(ptr %0, ptr %1) {
entry:
  %note.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %note.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %note.env, i32 0, i32 9
  store %FS_String { ptr @fs.str.6, i64 4 }, ptr %note.addr, align 8
  %machine.hdr = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %machine.hdr1 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %2 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Finish.payload, ptr null, i32 1) to i64))
  %enabled.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %enabled.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %enabled.env, i32 0, i32 6
  %3 = load i8, ptr %enabled.addr, align 1
  %4 = trunc i8 %3 to i1
  %5 = getelementptr inbounds nuw %SyntaxCoverage.event.Finish.payload, ptr %2, i32 0, i32 0
  %6 = zext i1 %4 to i8
  store i8 %6, ptr %5, align 1
  %label.env = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 1
  %label.addr = getelementptr inbounds nuw %SyntaxCoverage.env, ptr %label.env, i32 0, i32 8
  %7 = load %FS_String, ptr %label.addr, align 8
  %8 = getelementptr inbounds nuw %SyntaxCoverage.event.Finish.payload, ptr %2, i32 0, i32 1
  store %FS_String %7, ptr %8, align 8
  %9 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %10 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 0
  store i32 4, ptr %10, align 4
  %11 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 1
  store i32 0, ptr %11, align 4
  %12 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 2
  store i16 0, ptr %12, align 2
  %13 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 3
  store i32 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Finish.payload, ptr null, i32 1) to i32), ptr %13, align 4
  %14 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 4
  store ptr %2, ptr %14, align 8
  %15 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 5
  store i64 0, ptr %15, align 4
  call void @fs_enqueue_event(ptr %machine.hdr1, ptr %9)
  ret void
}

define void @fs_dispatch_SyntaxCoverage(ptr %0, ptr %1) {
entry:
  %hdr = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %current_state.addr = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr, i32 0, i32 0
  %event.tag.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 0
  %2 = load i32, ptr %current_state.addr, align 4
  %3 = load i32, ptr %event.tag.addr, align 4
  switch i32 %2, label %dispatch.return [
    i32 0, label %state.Idle
    i32 1, label %state.Armed
    i32 2, label %state.Waiting
    i32 3, label %state.Cooling
    i32 4, label %state.Error
    i32 5, label %state.Done
  ]

dispatch.return:                                  ; preds = %state.Done, %trigger.3, %state.Error, %state.Cooling, %trigger.5, %state.Waiting, %trigger.1, %state.Armed, %state.Idle, %entry
  ret void

state.Idle:                                       ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 0, label %trigger.0
  ]

trigger.0:                                        ; preds = %state.Idle
  br label %transition.exec.0

transition.exec.0:                                ; preds = %trigger.0
  %hdr1 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr1, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_0(ptr %0, ptr %1)
  store i32 1, ptr %state.addr, align 4
  ret void

state.Armed:                                      ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 1, label %trigger.1
  ]

trigger.1:                                        ; preds = %state.Armed
  %4 = call i1 @fs_guard_SyntaxCoverage_1(ptr %0, ptr %1)
  br i1 %4, label %transition.exec.1, label %dispatch.return

transition.exec.1:                                ; preds = %trigger.1
  %hdr2 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr3 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr2, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_1(ptr %0, ptr %1)
  store i32 2, ptr %state.addr3, align 4
  call void @fs_start_timer(ptr %hdr2, i32 5, i64 5000000000, i64 0)
  ret void

state.Waiting:                                    ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 5, label %trigger.5
  ]

trigger.5:                                        ; preds = %state.Waiting
  %5 = call i1 @fs_guard_SyntaxCoverage_2(ptr %0, ptr %1)
  br i1 %5, label %transition.exec.2, label %dispatch.return

transition.exec.2:                                ; preds = %trigger.5
  %hdr4 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr5 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr4, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_2(ptr %0, ptr %1)
  call void @fs_cancel_timer(ptr %hdr4, i32 5)
  store i32 3, ptr %state.addr5, align 4
  call void @fs_start_timer(ptr %hdr4, i32 6, i64 60000000000, i64 2000000000)
  ret void

state.Cooling:                                    ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 6, label %trigger.6
  ]

trigger.6:                                        ; preds = %state.Cooling
  br label %transition.exec.3

transition.exec.3:                                ; preds = %trigger.6
  %hdr6 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr7 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr6, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_3(ptr %0, ptr %1)
  call void @fs_cancel_timer(ptr %hdr6, i32 6)
  store i32 5, ptr %state.addr7, align 4
  call void @fs_start_timer(ptr %hdr6, i32 7, i64 1000000000, i64 0)
  ret void

state.Error:                                      ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 0, label %trigger.08
    i32 3, label %trigger.3
  ]

trigger.08:                                       ; preds = %state.Error
  br label %transition.exec.6

transition.exec.6:                                ; preds = %trigger.08
  %hdr9 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr10 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr9, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_6(ptr %0, ptr %1)
  store i32 4, ptr %state.addr10, align 4
  ret void

trigger.3:                                        ; preds = %state.Error
  %6 = call i1 @fs_guard_SyntaxCoverage_5(ptr %0, ptr %1)
  br i1 %6, label %transition.exec.5, label %dispatch.return

transition.exec.5:                                ; preds = %trigger.3
  %hdr11 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr12 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr11, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_5(ptr %0, ptr %1)
  store i32 0, ptr %state.addr12, align 4
  ret void

state.Done:                                       ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 7, label %trigger.7
  ]

trigger.7:                                        ; preds = %state.Done
  br label %transition.exec.4

transition.exec.4:                                ; preds = %trigger.7
  %hdr13 = getelementptr inbounds nuw %SyntaxCoverage.machine, ptr %0, i32 0, i32 0
  %state.addr14 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr13, i32 0, i32 0
  call void @fs_action_SyntaxCoverage_4(ptr %0, ptr %1)
  call void @fs_cancel_timer(ptr %hdr13, i32 7)
  store i32 4, ptr %state.addr14, align 4
  ret void
}

define void @fs_init_SyntaxCoverage() {
entry:
  %0 = call i1 @fs_init_machine(ptr @fs_machine_SyntaxCoverage, i32 0, ptr @fs_dispatch_SyntaxCoverage)
  br i1 %0, label %initialize, label %done

initialize:                                       ; preds = %entry
  store i32 0, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), align 4
  store i32 1, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 1), align 4
  store float 2.000000e+00, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 3), align 4
  store double 3.000000e+00, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 4), align 8
  store i8 1, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 6), align 1
  store i8 0, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 7), align 1
  store %FS_String { ptr @fs.str.7, i64 4 }, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 8), align 8
  store %FS_String { ptr @fs.str.8, i64 4 }, ptr getelementptr inbounds nuw (%SyntaxCoverage.env, ptr getelementptr inbounds nuw (%SyntaxCoverage.machine, ptr @fs_machine_SyntaxCoverage, i32 0, i32 1), i32 0, i32 9), align 8
  br label %done

done:                                             ; preds = %initialize, %entry
  ret void
}

define internal i1 @fs_guard_Secondary_0(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %Secondary.event.Start.payload, ptr %event.payload, i32 0, i32 0
  %value = load i32, ptr %2, align 4
  %3 = icmp sge i32 %value, 0
  ret i1 %3
}

define internal i1 @fs_guard_Secondary_1(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %Secondary.event.Flip.payload, ptr %event.payload, i32 0, i32 0
  %next_flag = load i8, ptr %2, align 1
  %3 = trunc i8 %next_flag to i1
  %4 = getelementptr inbounds nuw %Secondary.event.Flip.payload, ptr %event.payload, i32 0, i32 1
  %next_note = load %FS_String, ptr %4, align 8
  %sink.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %Secondary.env, ptr %sink.env, i32 0, i32 0
  %5 = load i32, ptr %sink.addr, align 4
  %6 = icmp sle i32 %5, 100
  ret i1 %6
}

define internal i1 @fs_guard_Secondary_2(ptr %0, ptr %1) {
entry:
  %sink.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %Secondary.env, ptr %sink.env, i32 0, i32 0
  %2 = load i32, ptr %sink.addr, align 4
  %3 = icmp slt i32 %2, 10
  %flag.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %flag.addr = getelementptr inbounds nuw %Secondary.env, ptr %flag.env, i32 0, i32 2
  %4 = load i8, ptr %flag.addr, align 1
  %5 = trunc i8 %4 to i1
  %6 = icmp eq i1 %5, false
  %7 = or i1 %3, %6
  ret i1 %7
}

define internal void @fs_action_Secondary_0(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %Secondary.event.Start.payload, ptr %event.payload, i32 0, i32 0
  %value = load i32, ptr %2, align 4
  %sink.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %Secondary.env, ptr %sink.env, i32 0, i32 0
  store i32 %value, ptr %sink.addr, align 4
  %note.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %note.addr = getelementptr inbounds nuw %Secondary.env, ptr %note.env, i32 0, i32 3
  store %FS_String { ptr @fs.str.9, i64 7 }, ptr %note.addr, align 8
  %machine.hdr = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %3 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %4 = getelementptr inbounds nuw %FS_Event, ptr %3, i32 0, i32 0
  store i32 0, ptr %4, align 4
  %5 = getelementptr inbounds nuw %FS_Event, ptr %3, i32 0, i32 1
  store i32 0, ptr %5, align 4
  %6 = getelementptr inbounds nuw %FS_Event, ptr %3, i32 0, i32 2
  store i16 0, ptr %6, align 2
  %7 = getelementptr inbounds nuw %FS_Event, ptr %3, i32 0, i32 3
  store i32 0, ptr %7, align 4
  %8 = getelementptr inbounds nuw %FS_Event, ptr %3, i32 0, i32 4
  store ptr null, ptr %8, align 8
  %9 = getelementptr inbounds nuw %FS_Event, ptr %3, i32 0, i32 5
  store i64 0, ptr %9, align 4
  call void @fs_enqueue_event(ptr @fs_machine_SyntaxCoverage, ptr %3)
  ret void
}

define internal void @fs_action_Secondary_1(ptr %0, ptr %1) {
entry:
  %event.payload.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 4
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %2 = getelementptr inbounds nuw %Secondary.event.Flip.payload, ptr %event.payload, i32 0, i32 0
  %next_flag = load i8, ptr %2, align 1
  %3 = trunc i8 %next_flag to i1
  %4 = getelementptr inbounds nuw %Secondary.event.Flip.payload, ptr %event.payload, i32 0, i32 1
  %next_note = load %FS_String, ptr %4, align 8
  %flag.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %flag.addr = getelementptr inbounds nuw %Secondary.env, ptr %flag.env, i32 0, i32 2
  store i1 %3, ptr %flag.addr, align 1
  %note.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %note.addr = getelementptr inbounds nuw %Secondary.env, ptr %note.env, i32 0, i32 3
  store %FS_String %next_note, ptr %note.addr, align 8
  %sink.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %Secondary.env, ptr %sink.env, i32 0, i32 0
  %sink.env1 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr2 = getelementptr inbounds nuw %Secondary.env, ptr %sink.env1, i32 0, i32 0
  %5 = load i32, ptr %sink.addr2, align 4
  %count.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %count.addr = getelementptr inbounds nuw %Secondary.env, ptr %count.env, i32 0, i32 1
  %6 = load i16, ptr %count.addr, align 2
  %7 = sext i16 %6 to i32
  %8 = add i32 %5, %7
  store i32 %8, ptr %sink.addr, align 4
  %machine.hdr = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %machine.hdr3 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %9 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %10 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 0
  store i32 1, ptr %10, align 4
  %11 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 1
  store i32 0, ptr %11, align 4
  %12 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 2
  store i16 0, ptr %12, align 2
  %13 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 3
  store i32 0, ptr %13, align 4
  %14 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 4
  store ptr null, ptr %14, align 8
  %15 = getelementptr inbounds nuw %FS_Event, ptr %9, i32 0, i32 5
  store i64 0, ptr %15, align 4
  call void @fs_enqueue_event(ptr %machine.hdr3, ptr %9)
  %machine.hdr4 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %16 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Finish.payload, ptr null, i32 1) to i64))
  %17 = getelementptr inbounds nuw %SyntaxCoverage.event.Finish.payload, ptr %16, i32 0, i32 0
  %18 = zext i1 %3 to i8
  store i8 %18, ptr %17, align 1
  %19 = getelementptr inbounds nuw %SyntaxCoverage.event.Finish.payload, ptr %16, i32 0, i32 1
  store %FS_String %next_note, ptr %19, align 8
  %20 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %21 = getelementptr inbounds nuw %FS_Event, ptr %20, i32 0, i32 0
  store i32 4, ptr %21, align 4
  %22 = getelementptr inbounds nuw %FS_Event, ptr %20, i32 0, i32 1
  store i32 0, ptr %22, align 4
  %23 = getelementptr inbounds nuw %FS_Event, ptr %20, i32 0, i32 2
  store i16 0, ptr %23, align 2
  %24 = getelementptr inbounds nuw %FS_Event, ptr %20, i32 0, i32 3
  store i32 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Finish.payload, ptr null, i32 1) to i32), ptr %24, align 4
  %25 = getelementptr inbounds nuw %FS_Event, ptr %20, i32 0, i32 4
  store ptr %16, ptr %25, align 8
  %26 = getelementptr inbounds nuw %FS_Event, ptr %20, i32 0, i32 5
  store i64 0, ptr %26, align 4
  call void @fs_enqueue_event(ptr @fs_machine_SyntaxCoverage, ptr %20)
  ret void
}

define internal void @fs_action_Secondary_2(ptr %0, ptr %1) {
entry:
  %sink.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr = getelementptr inbounds nuw %Secondary.env, ptr %sink.env, i32 0, i32 0
  %sink.env1 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %sink.addr2 = getelementptr inbounds nuw %Secondary.env, ptr %sink.env1, i32 0, i32 0
  %2 = load i32, ptr %sink.addr2, align 4
  %3 = add i32 %2, 1
  store i32 %3, ptr %sink.addr, align 4
  %flag.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %flag.addr = getelementptr inbounds nuw %Secondary.env, ptr %flag.env, i32 0, i32 2
  store i1 false, ptr %flag.addr, align 1
  %note.env = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %note.addr = getelementptr inbounds nuw %Secondary.env, ptr %note.env, i32 0, i32 3
  store %FS_String { ptr @fs.str.10, i64 7 }, ptr %note.addr, align 8
  %machine.hdr = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Reset.payload, ptr null, i32 1) to i64))
  %note.env3 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 1
  %note.addr4 = getelementptr inbounds nuw %Secondary.env, ptr %note.env3, i32 0, i32 3
  %5 = load %FS_String, ptr %note.addr4, align 8
  %6 = getelementptr inbounds nuw %SyntaxCoverage.event.Reset.payload, ptr %4, i32 0, i32 0
  store %FS_String %5, ptr %6, align 8
  %7 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%FS_Event, ptr null, i32 1) to i64))
  %8 = getelementptr inbounds nuw %FS_Event, ptr %7, i32 0, i32 0
  store i32 3, ptr %8, align 4
  %9 = getelementptr inbounds nuw %FS_Event, ptr %7, i32 0, i32 1
  store i32 0, ptr %9, align 4
  %10 = getelementptr inbounds nuw %FS_Event, ptr %7, i32 0, i32 2
  store i16 0, ptr %10, align 2
  %11 = getelementptr inbounds nuw %FS_Event, ptr %7, i32 0, i32 3
  store i32 ptrtoint (ptr getelementptr (%SyntaxCoverage.event.Reset.payload, ptr null, i32 1) to i32), ptr %11, align 4
  %12 = getelementptr inbounds nuw %FS_Event, ptr %7, i32 0, i32 4
  store ptr %4, ptr %12, align 8
  %13 = getelementptr inbounds nuw %FS_Event, ptr %7, i32 0, i32 5
  store i64 0, ptr %13, align 4
  call void @fs_enqueue_event(ptr @fs_machine_SyntaxCoverage, ptr %7)
  ret void
}

define void @fs_dispatch_Secondary(ptr %0, ptr %1) {
entry:
  %hdr = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %current_state.addr = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr, i32 0, i32 0
  %event.tag.addr = getelementptr inbounds nuw %FS_Event, ptr %1, i32 0, i32 0
  %2 = load i32, ptr %current_state.addr, align 4
  %3 = load i32, ptr %event.tag.addr, align 4
  switch i32 %2, label %dispatch.return [
    i32 0, label %state.S0
    i32 1, label %state.S1
    i32 2, label %state.S2
  ]

dispatch.return:                                  ; preds = %trigger.1, %state.S2, %trigger.2, %state.S1, %trigger.0, %state.S0, %entry
  ret void

state.S0:                                         ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 0, label %trigger.0
  ]

trigger.0:                                        ; preds = %state.S0
  %4 = call i1 @fs_guard_Secondary_0(ptr %0, ptr %1)
  br i1 %4, label %transition.exec.0, label %dispatch.return

transition.exec.0:                                ; preds = %trigger.0
  %hdr1 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %state.addr = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr1, i32 0, i32 0
  call void @fs_action_Secondary_0(ptr %0, ptr %1)
  store i32 1, ptr %state.addr, align 4
  ret void

state.S1:                                         ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 2, label %trigger.2
  ]

trigger.2:                                        ; preds = %state.S1
  %5 = call i1 @fs_guard_Secondary_1(ptr %0, ptr %1)
  br i1 %5, label %transition.exec.1, label %dispatch.return

transition.exec.1:                                ; preds = %trigger.2
  %hdr2 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %state.addr3 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr2, i32 0, i32 0
  call void @fs_action_Secondary_1(ptr %0, ptr %1)
  store i32 2, ptr %state.addr3, align 4
  ret void

state.S2:                                         ; preds = %entry
  switch i32 %3, label %dispatch.return [
    i32 1, label %trigger.1
  ]

trigger.1:                                        ; preds = %state.S2
  %6 = call i1 @fs_guard_Secondary_2(ptr %0, ptr %1)
  br i1 %6, label %transition.exec.2, label %dispatch.return

transition.exec.2:                                ; preds = %trigger.1
  %hdr4 = getelementptr inbounds nuw %Secondary.machine, ptr %0, i32 0, i32 0
  %state.addr5 = getelementptr inbounds nuw %FS_MachineHeader, ptr %hdr4, i32 0, i32 0
  call void @fs_action_Secondary_2(ptr %0, ptr %1)
  store i32 0, ptr %state.addr5, align 4
  ret void
}

define void @fs_init_Secondary() {
entry:
  %0 = call i1 @fs_init_machine(ptr @fs_machine_Secondary, i32 0, ptr @fs_dispatch_Secondary)
  br i1 %0, label %initialize, label %done

initialize:                                       ; preds = %entry
  store i32 0, ptr getelementptr inbounds nuw (%Secondary.machine, ptr @fs_machine_Secondary, i32 0, i32 1), align 4
  store i8 0, ptr getelementptr inbounds nuw (%Secondary.env, ptr getelementptr inbounds nuw (%Secondary.machine, ptr @fs_machine_Secondary, i32 0, i32 1), i32 0, i32 2), align 1
  store %FS_String { ptr @fs.str.11, i64 9 }, ptr getelementptr inbounds nuw (%Secondary.env, ptr getelementptr inbounds nuw (%Secondary.machine, ptr @fs_machine_Secondary, i32 0, i32 1), i32 0, i32 3), align 8
  br label %done

done:                                             ; preds = %initialize, %entry
  ret void
}

define void @fs_module_init() {
entry:
  call void @fs_init_SyntaxCoverage()
  call void @fs_init_Secondary()
  ret void
}

define void @fs_module_cleanup() {
entry:
  call void @fs_cleanup_machine(ptr @fs_machine_SyntaxCoverage)
  call void @fs_cleanup_machine(ptr @fs_machine_Secondary)
  ret void
}

define ptr @fs_get_machine_SyntaxCoverage() {
entry:
  ret ptr @fs_machine_SyntaxCoverage
}

define void @fs_run_SyntaxCoverage() {
entry:
  call void @fs_module_init()
  call void @fs_runtime_run(ptr @fs_machine_SyntaxCoverage)
  ret void
}

define i1 @fs_step_SyntaxCoverage() {
entry:
  call void @fs_module_init()
  %0 = call i1 @fs_runtime_step(ptr @fs_machine_SyntaxCoverage)
  ret i1 %0
}

define void @fs_stop_SyntaxCoverage() {
entry:
  call void @fs_stop_machine(ptr @fs_machine_SyntaxCoverage)
  ret void
}

define ptr @fs_get_machine_Secondary() {
entry:
  ret ptr @fs_machine_Secondary
}

define void @fs_run_Secondary() {
entry:
  call void @fs_module_init()
  call void @fs_runtime_run(ptr @fs_machine_Secondary)
  ret void
}

define i1 @fs_step_Secondary() {
entry:
  call void @fs_module_init()
  %0 = call i1 @fs_runtime_step(ptr @fs_machine_Secondary)
  ret i1 %0
}

define void @fs_stop_Secondary() {
entry:
  call void @fs_stop_machine(ptr @fs_machine_Secondary)
  ret void
}
