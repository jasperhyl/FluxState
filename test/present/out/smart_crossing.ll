; ModuleID = '/workspace/test/present/smart_crossing.fs'
source_filename = "/workspace/test/present/smart_crossing.fs"
target triple = "x86_64-unknown-linux-gnu"

%CrossingController.machine = type { %FS_MachineHeader, %CrossingController.env }
%FS_MachineHeader = type { i32, i32, i64, ptr, ptr, ptr }
%CrossingController.env = type { i8, i8, i8, i32, i32, i32, %FS_String }
%FS_String = type { ptr, i64 }
%DisplayPanel.machine = type { %FS_MachineHeader, %DisplayPanel.env }
%DisplayPanel.env = type { i32, i32, i8, %FS_String }

@fs_machine_CrossingController = internal global %CrossingController.machine zeroinitializer
@fs_machine_DisplayPanel = internal global %DisplayPanel.machine zeroinitializer
@fs.str = private constant [12 x i8] c"prepare_walk"
@fs.str.1 = private constant [4 x i8] c"walk"
@fs.str.2 = private constant [5 x i8] c"flash"
@fs.str.3 = private constant [4 x i8] c"cars"
@fs.str.4 = private constant [9 x i8] c"emergency"
@fs.str.5 = private constant [9 x i8] c"emergency"
@fs.str.6 = private constant [9 x i8] c"emergency"
@fs.str.7 = private constant [9 x i8] c"emergency"
@fs.str.8 = private constant [4 x i8] c"cars"
@fs.str.9 = private constant [11 x i8] c"maintenance"
@fs.str.10 = private constant [4 x i8] c"cars"
@fs.str.11 = private constant [4 x i8] c"cars"
@fs.str.12 = private constant [11 x i8] c"please wait"
@fs.str.13 = private constant [4 x i8] c"walk"
@fs.str.14 = private constant [15 x i8] c"finish crossing"
@fs.str.15 = private constant [15 x i8] c"vehicles may go"
@fs.str.16 = private constant [18 x i8] c"emergency priority"
@fs.str.17 = private constant [18 x i8] c"emergency priority"
@fs.str.18 = private constant [18 x i8] c"emergency priority"
@fs.str.19 = private constant [18 x i8] c"emergency priority"
@fs.str.20 = private constant [15 x i8] c"vehicles may go"
@fs.str.21 = private constant [11 x i8] c"maintenance"
@fs.str.22 = private constant [15 x i8] c"vehicles may go"
@fs.str.23 = private constant [15 x i8] c"vehicles may go"

declare void @fs_enqueue_event(ptr, ptr) local_unnamed_addr

declare void @fs_start_timer(ptr, i32, i64, i64) local_unnamed_addr

declare void @fs_cancel_timer(ptr, i32) local_unnamed_addr

declare i1 @fs_init_machine(ptr, i32, ptr) local_unnamed_addr

declare void @fs_cleanup_machine(ptr) local_unnamed_addr

declare i1 @fs_runtime_step(ptr) local_unnamed_addr

declare void @fs_runtime_run(ptr) local_unnamed_addr

declare void @fs_stop_machine(ptr) local_unnamed_addr

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #0

define internal fastcc void @fs_action_CrossingController_0(ptr nocapture writeonly %0, ptr nocapture readonly %1) unnamed_addr {
entry:
  %event.payload.addr = getelementptr inbounds i8, ptr %1, i64 16
  %event.payload = load ptr, ptr %event.payload.addr, align 8
  %sensor_id = load i32, ptr %event.payload, align 4
  %ped_waiting.env = getelementptr inbounds i8, ptr %0, i64 40
  store i1 true, ptr %ped_waiting.env, align 1
  %last_sensor_id.addr = getelementptr inbounds i8, ptr %0, i64 44
  store i32 %sensor_id, ptr %last_sensor_id.addr, align 4
  %phase.addr = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str, ptr %phase.addr, align 8
  %phase.addr.repack1 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 12, ptr %phase.addr.repack1, align 8
  %2 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  store i32 %sensor_id, ptr %2, align 4
  %3 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 0, ptr %3, align 4
  %4 = getelementptr inbounds i8, ptr %3, i64 4
  store i32 0, ptr %4, align 4
  %5 = getelementptr inbounds i8, ptr %3, i64 8
  store i16 0, ptr %5, align 2
  %6 = getelementptr inbounds i8, ptr %3, i64 12
  store i32 4, ptr %6, align 4
  %7 = getelementptr inbounds i8, ptr %3, i64 16
  store ptr %2, ptr %7, align 8
  %8 = getelementptr inbounds i8, ptr %3, i64 24
  store i64 0, ptr %8, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %3)
  ret void
}

define void @fs_dispatch_CrossingController(ptr %0, ptr nocapture readonly %1) {
entry:
  %2 = load i32, ptr %0, align 4
  %3 = load i32, ptr %1, align 4
  switch i32 %2, label %common.ret [
    i32 0, label %state.VehicleGreen
    i32 1, label %state.VehicleYellow
    i32 2, label %state.PedWalk
    i32 3, label %state.PedFlash
    i32 4, label %state.Emergency
    i32 5, label %state.Maintenance
  ]

common.ret:                                       ; preds = %entry, %state.VehicleGreen, %trigger.0, %state.VehicleYellow, %trigger.5, %state.PedWalk, %state.PedFlash, %state.Maintenance, %state.Emergency, %transition.exec.10, %transition.exec.8, %transition.exec.3, %transition.exec.7, %transition.exec.2, %transition.exec.6, %transition.exec.1, %transition.exec.5, %transition.exec.0, %transition.exec.4, %transition.exec.9
  ret void

state.VehicleGreen:                               ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 3, label %transition.exec.9
    i32 1, label %transition.exec.4
    i32 0, label %trigger.0
  ]

transition.exec.9:                                ; preds = %state.VehicleGreen
  %maintenance_mode.addr.i = getelementptr inbounds i8, ptr %0, i64 42
  store i1 true, ptr %maintenance_mode.addr.i, align 1
  %phase.addr.i = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.9, ptr %phase.addr.i, align 8
  %phase.addr.repack1.i = getelementptr inbounds i8, ptr %0, i64 64
  store i64 11, ptr %phase.addr.repack1.i, align 8
  %4 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 5, ptr %4, align 4
  %5 = getelementptr inbounds i8, ptr %4, i64 4
  store i32 0, ptr %5, align 4
  %6 = getelementptr inbounds i8, ptr %4, i64 8
  store i16 0, ptr %6, align 2
  %7 = getelementptr inbounds i8, ptr %4, i64 12
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(20) %7, i8 0, i64 20, i1 false)
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %4)
  store i32 5, ptr %0, align 4
  br label %common.ret

transition.exec.4:                                ; preds = %state.VehicleGreen
  %event.payload.addr.i = getelementptr inbounds i8, ptr %1, i64 16
  %event.payload.i = load ptr, ptr %event.payload.addr.i, align 8
  %code.i = load i32, ptr %event.payload.i, align 4
  %emergency_active.addr.i = getelementptr inbounds i8, ptr %0, i64 41
  store i1 true, ptr %emergency_active.addr.i, align 1
  %emergency_code.addr.i = getelementptr inbounds i8, ptr %0, i64 48
  store i32 %code.i, ptr %emergency_code.addr.i, align 4
  %phase.addr.i26 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.4, ptr %phase.addr.i26, align 8
  %phase.addr.repack1.i27 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 9, ptr %phase.addr.repack1.i27, align 8
  %8 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  store i32 %code.i, ptr %8, align 4
  %9 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 4, ptr %9, align 4
  %10 = getelementptr inbounds i8, ptr %9, i64 4
  store i32 0, ptr %10, align 4
  %11 = getelementptr inbounds i8, ptr %9, i64 8
  store i16 0, ptr %11, align 2
  %12 = getelementptr inbounds i8, ptr %9, i64 12
  store i32 4, ptr %12, align 4
  %13 = getelementptr inbounds i8, ptr %9, i64 16
  store ptr %8, ptr %13, align 8
  %14 = getelementptr inbounds i8, ptr %9, i64 24
  store i64 0, ptr %14, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %9)
  store i32 4, ptr %0, align 4
  br label %common.ret

trigger.0:                                        ; preds = %state.VehicleGreen
  %emergency_active.addr.i28 = getelementptr inbounds i8, ptr %0, i64 41
  %15 = load i8, ptr %emergency_active.addr.i28, align 1
  %maintenance_mode.addr.i29 = getelementptr inbounds i8, ptr %0, i64 42
  %16 = load i8, ptr %maintenance_mode.addr.i29, align 1
  %.demorgan1.i = or i8 %16, %15
  %.demorgan.i = trunc i8 %.demorgan1.i to i1
  br i1 %.demorgan.i, label %common.ret, label %transition.exec.0

transition.exec.0:                                ; preds = %trigger.0
  tail call fastcc void @fs_action_CrossingController_0(ptr nonnull %0, ptr nonnull %1)
  store i32 1, ptr %0, align 4
  tail call void @fs_start_timer(ptr nonnull %0, i32 5, i64 1000000000, i64 0)
  br label %common.ret

state.VehicleYellow:                              ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 1, label %transition.exec.5
    i32 5, label %trigger.5
  ]

transition.exec.5:                                ; preds = %state.VehicleYellow
  %event.payload.addr.i30 = getelementptr inbounds i8, ptr %1, i64 16
  %event.payload.i31 = load ptr, ptr %event.payload.addr.i30, align 8
  %code.i32 = load i32, ptr %event.payload.i31, align 4
  %emergency_active.addr.i33 = getelementptr inbounds i8, ptr %0, i64 41
  store i1 true, ptr %emergency_active.addr.i33, align 1
  %emergency_code.addr.i34 = getelementptr inbounds i8, ptr %0, i64 48
  store i32 %code.i32, ptr %emergency_code.addr.i34, align 4
  %phase.addr.i35 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.5, ptr %phase.addr.i35, align 8
  %phase.addr.repack1.i36 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 9, ptr %phase.addr.repack1.i36, align 8
  %17 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  store i32 %code.i32, ptr %17, align 4
  %18 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 4, ptr %18, align 4
  %19 = getelementptr inbounds i8, ptr %18, i64 4
  store i32 0, ptr %19, align 4
  %20 = getelementptr inbounds i8, ptr %18, i64 8
  store i16 0, ptr %20, align 2
  %21 = getelementptr inbounds i8, ptr %18, i64 12
  store i32 4, ptr %21, align 4
  %22 = getelementptr inbounds i8, ptr %18, i64 16
  store ptr %17, ptr %22, align 8
  %23 = getelementptr inbounds i8, ptr %18, i64 24
  store i64 0, ptr %23, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %18)
  tail call void @fs_cancel_timer(ptr nonnull %0, i32 5)
  store i32 4, ptr %0, align 4
  br label %common.ret

trigger.5:                                        ; preds = %state.VehicleYellow
  %ped_waiting.env.i = getelementptr inbounds i8, ptr %0, i64 40
  %24 = load i8, ptr %ped_waiting.env.i, align 1
  %25 = trunc i8 %24 to i1
  br i1 %25, label %transition.exec.1, label %common.ret

transition.exec.1:                                ; preds = %trigger.5
  %phase.addr.i37 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.1, ptr %phase.addr.i37, align 8
  %phase.addr.repack1.i38 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 4, ptr %phase.addr.repack1.i38, align 8
  %26 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  %last_sensor_id.addr.i = getelementptr inbounds i8, ptr %0, i64 44
  %27 = load i32, ptr %last_sensor_id.addr.i, align 4
  store i32 %27, ptr %26, align 4
  %28 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 1, ptr %28, align 4
  %29 = getelementptr inbounds i8, ptr %28, i64 4
  store i32 0, ptr %29, align 4
  %30 = getelementptr inbounds i8, ptr %28, i64 8
  store i16 0, ptr %30, align 2
  %31 = getelementptr inbounds i8, ptr %28, i64 12
  store i32 4, ptr %31, align 4
  %32 = getelementptr inbounds i8, ptr %28, i64 16
  store ptr %26, ptr %32, align 8
  %33 = getelementptr inbounds i8, ptr %28, i64 24
  store i64 0, ptr %33, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %28)
  tail call void @fs_cancel_timer(ptr nonnull %0, i32 5)
  store i32 2, ptr %0, align 4
  tail call void @fs_start_timer(ptr nonnull %0, i32 6, i64 1000000000, i64 0)
  br label %common.ret

state.PedWalk:                                    ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 1, label %transition.exec.6
    i32 6, label %transition.exec.2
  ]

transition.exec.6:                                ; preds = %state.PedWalk
  %event.payload.addr.i39 = getelementptr inbounds i8, ptr %1, i64 16
  %event.payload.i40 = load ptr, ptr %event.payload.addr.i39, align 8
  %code.i41 = load i32, ptr %event.payload.i40, align 4
  %emergency_active.addr.i42 = getelementptr inbounds i8, ptr %0, i64 41
  store i1 true, ptr %emergency_active.addr.i42, align 1
  %emergency_code.addr.i43 = getelementptr inbounds i8, ptr %0, i64 48
  store i32 %code.i41, ptr %emergency_code.addr.i43, align 4
  %phase.addr.i44 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.6, ptr %phase.addr.i44, align 8
  %phase.addr.repack1.i45 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 9, ptr %phase.addr.repack1.i45, align 8
  %34 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  store i32 %code.i41, ptr %34, align 4
  %35 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 4, ptr %35, align 4
  %36 = getelementptr inbounds i8, ptr %35, i64 4
  store i32 0, ptr %36, align 4
  %37 = getelementptr inbounds i8, ptr %35, i64 8
  store i16 0, ptr %37, align 2
  %38 = getelementptr inbounds i8, ptr %35, i64 12
  store i32 4, ptr %38, align 4
  %39 = getelementptr inbounds i8, ptr %35, i64 16
  store ptr %34, ptr %39, align 8
  %40 = getelementptr inbounds i8, ptr %35, i64 24
  store i64 0, ptr %40, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %35)
  tail call void @fs_cancel_timer(ptr nonnull %0, i32 6)
  store i32 4, ptr %0, align 4
  br label %common.ret

transition.exec.2:                                ; preds = %state.PedWalk
  %phase.addr.i46 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.2, ptr %phase.addr.i46, align 8
  %phase.addr.repack1.i47 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 5, ptr %phase.addr.repack1.i47, align 8
  %41 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  %last_sensor_id.addr.i48 = getelementptr inbounds i8, ptr %0, i64 44
  %42 = load i32, ptr %last_sensor_id.addr.i48, align 4
  store i32 %42, ptr %41, align 4
  %43 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 2, ptr %43, align 4
  %44 = getelementptr inbounds i8, ptr %43, i64 4
  store i32 0, ptr %44, align 4
  %45 = getelementptr inbounds i8, ptr %43, i64 8
  store i16 0, ptr %45, align 2
  %46 = getelementptr inbounds i8, ptr %43, i64 12
  store i32 4, ptr %46, align 4
  %47 = getelementptr inbounds i8, ptr %43, i64 16
  store ptr %41, ptr %47, align 8
  %48 = getelementptr inbounds i8, ptr %43, i64 24
  store i64 0, ptr %48, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %43)
  tail call void @fs_cancel_timer(ptr nonnull %0, i32 6)
  store i32 3, ptr %0, align 4
  tail call void @fs_start_timer(ptr nonnull %0, i32 7, i64 1000000000, i64 0)
  br label %common.ret

state.PedFlash:                                   ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 1, label %transition.exec.7
    i32 7, label %transition.exec.3
  ]

transition.exec.7:                                ; preds = %state.PedFlash
  %event.payload.addr.i49 = getelementptr inbounds i8, ptr %1, i64 16
  %event.payload.i50 = load ptr, ptr %event.payload.addr.i49, align 8
  %code.i51 = load i32, ptr %event.payload.i50, align 4
  %emergency_active.addr.i52 = getelementptr inbounds i8, ptr %0, i64 41
  store i1 true, ptr %emergency_active.addr.i52, align 1
  %emergency_code.addr.i53 = getelementptr inbounds i8, ptr %0, i64 48
  store i32 %code.i51, ptr %emergency_code.addr.i53, align 4
  %phase.addr.i54 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.7, ptr %phase.addr.i54, align 8
  %phase.addr.repack1.i55 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 9, ptr %phase.addr.repack1.i55, align 8
  %49 = tail call dereferenceable_or_null(4) ptr @malloc(i64 4)
  store i32 %code.i51, ptr %49, align 4
  %50 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 4, ptr %50, align 4
  %51 = getelementptr inbounds i8, ptr %50, i64 4
  store i32 0, ptr %51, align 4
  %52 = getelementptr inbounds i8, ptr %50, i64 8
  store i16 0, ptr %52, align 2
  %53 = getelementptr inbounds i8, ptr %50, i64 12
  store i32 4, ptr %53, align 4
  %54 = getelementptr inbounds i8, ptr %50, i64 16
  store ptr %49, ptr %54, align 8
  %55 = getelementptr inbounds i8, ptr %50, i64 24
  store i64 0, ptr %55, align 4
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %50)
  tail call void @fs_cancel_timer(ptr nonnull %0, i32 7)
  store i32 4, ptr %0, align 4
  br label %common.ret

transition.exec.3:                                ; preds = %state.PedFlash
  %ped_waiting.env.i56 = getelementptr inbounds i8, ptr %0, i64 40
  store i1 false, ptr %ped_waiting.env.i56, align 1
  %crossings_served.addr.i = getelementptr inbounds i8, ptr %0, i64 52
  %56 = load i32, ptr %crossings_served.addr.i, align 4
  %57 = add i32 %56, 1
  store i32 %57, ptr %crossings_served.addr.i, align 4
  %phase.addr.i57 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.3, ptr %phase.addr.i57, align 8
  %phase.addr.repack3.i = getelementptr inbounds i8, ptr %0, i64 64
  store i64 4, ptr %phase.addr.repack3.i, align 8
  %58 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 3, ptr %58, align 4
  %59 = getelementptr inbounds i8, ptr %58, i64 4
  store i32 0, ptr %59, align 4
  %60 = getelementptr inbounds i8, ptr %58, i64 8
  store i16 0, ptr %60, align 2
  %61 = getelementptr inbounds i8, ptr %58, i64 12
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(20) %61, i8 0, i64 20, i1 false)
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %58)
  tail call void @fs_cancel_timer(ptr nonnull %0, i32 7)
  store i32 0, ptr %0, align 4
  br label %common.ret

state.Emergency:                                  ; preds = %entry
  %cond25 = icmp eq i32 %3, 2
  br i1 %cond25, label %transition.exec.8, label %common.ret

transition.exec.8:                                ; preds = %state.Emergency
  %emergency_active.env.i = getelementptr inbounds i8, ptr %0, i64 40
  %emergency_active.addr.i58 = getelementptr inbounds i8, ptr %0, i64 41
  store i1 false, ptr %emergency_active.addr.i58, align 1
  %emergency_code.addr.i59 = getelementptr inbounds i8, ptr %0, i64 48
  store i32 0, ptr %emergency_code.addr.i59, align 4
  store i1 false, ptr %emergency_active.env.i, align 1
  %phase.addr.i60 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.8, ptr %phase.addr.i60, align 8
  %phase.addr.repack1.i61 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 4, ptr %phase.addr.repack1.i61, align 8
  %62 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 3, ptr %62, align 4
  %63 = getelementptr inbounds i8, ptr %62, i64 4
  store i32 0, ptr %63, align 4
  %64 = getelementptr inbounds i8, ptr %62, i64 8
  store i16 0, ptr %64, align 2
  %65 = getelementptr inbounds i8, ptr %62, i64 12
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(20) %65, i8 0, i64 20, i1 false)
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %62)
  store i32 0, ptr %0, align 4
  br label %common.ret

state.Maintenance:                                ; preds = %entry
  %cond = icmp eq i32 %3, 4
  br i1 %cond, label %transition.exec.10, label %common.ret

transition.exec.10:                               ; preds = %state.Maintenance
  %maintenance_mode.addr.i62 = getelementptr inbounds i8, ptr %0, i64 42
  store i1 false, ptr %maintenance_mode.addr.i62, align 1
  %phase.addr.i63 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr @fs.str.10, ptr %phase.addr.i63, align 8
  %phase.addr.repack1.i64 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 4, ptr %phase.addr.repack1.i64, align 8
  %66 = tail call dereferenceable_or_null(32) ptr @malloc(i64 32)
  store i32 3, ptr %66, align 4
  %67 = getelementptr inbounds i8, ptr %66, i64 4
  store i32 0, ptr %67, align 4
  %68 = getelementptr inbounds i8, ptr %66, i64 8
  store i16 0, ptr %68, align 2
  %69 = getelementptr inbounds i8, ptr %66, i64 12
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(20) %69, i8 0, i64 20, i1 false)
  tail call void @fs_enqueue_event(ptr nonnull @fs_machine_DisplayPanel, ptr nonnull %66)
  store i32 0, ptr %0, align 4
  br label %common.ret
}

define void @fs_init_CrossingController() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_CrossingController, i32 0, ptr nonnull @fs_dispatch_CrossingController)
  br i1 %0, label %initialize, label %done

initialize:                                       ; preds = %entry
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 40), align 8
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 41), align 1
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 42), align 2
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 44), align 4
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 48), align 16
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 52), align 4
  store ptr @fs.str.11, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 56), align 8
  store i64 4, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 64), align 16
  br label %done

done:                                             ; preds = %initialize, %entry
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, argmem: readwrite, inaccessiblemem: none)
define void @fs_dispatch_DisplayPanel(ptr nocapture %0, ptr nocapture readonly %1) #1 {
entry:
  %2 = load i32, ptr %0, align 4
  %3 = load i32, ptr %1, align 4
  switch i32 %2, label %common.ret [
    i32 0, label %state.Cars
    i32 1, label %state.Waiting
    i32 2, label %state.Walk
    i32 3, label %state.Flashing
    i32 4, label %state.Alert
    i32 5, label %state.Service
  ]

common.ret.sink.split.sink.split.sink.split:      ; preds = %state.Flashing, %state.Walk, %state.Waiting, %state.Cars, %transition.exec.0, %transition.exec.1, %transition.exec.2
  %.sink84.ph = phi i64 [ 40, %transition.exec.2 ], [ 40, %transition.exec.1 ], [ 40, %transition.exec.0 ], [ 44, %state.Cars ], [ 44, %state.Waiting ], [ 44, %state.Walk ], [ 44, %state.Flashing ]
  %.sink83.ph.ph = phi i1 [ true, %transition.exec.2 ], [ false, %transition.exec.1 ], [ false, %transition.exec.0 ], [ true, %state.Cars ], [ true, %state.Waiting ], [ true, %state.Walk ], [ true, %state.Flashing ]
  %fs.str.22.sink.ph.ph = phi ptr [ @fs.str.14, %transition.exec.2 ], [ @fs.str.13, %transition.exec.1 ], [ @fs.str.12, %transition.exec.0 ], [ @fs.str.16, %state.Cars ], [ @fs.str.17, %state.Waiting ], [ @fs.str.18, %state.Walk ], [ @fs.str.19, %state.Flashing ]
  %.sink82.ph.ph = phi i64 [ 15, %transition.exec.2 ], [ 4, %transition.exec.1 ], [ 11, %transition.exec.0 ], [ 18, %state.Cars ], [ 18, %state.Waiting ], [ 18, %state.Walk ], [ 18, %state.Flashing ]
  %.sink.ph.ph = phi i32 [ 3, %transition.exec.2 ], [ 2, %transition.exec.1 ], [ 1, %transition.exec.0 ], [ %3, %state.Cars ], [ %3, %state.Waiting ], [ %3, %state.Walk ], [ %3, %state.Flashing ]
  %event.payload.addr.i64 = getelementptr inbounds i8, ptr %1, i64 16
  %event.payload.i65 = load ptr, ptr %event.payload.addr.i64, align 8
  %code.i66 = load i32, ptr %event.payload.i65, align 4
  br label %common.ret.sink.split.sink.split

common.ret.sink.split.sink.split:                 ; preds = %common.ret.sink.split.sink.split.sink.split, %state.Alert, %state.Flashing
  %.sink84 = phi i64 [ 40, %state.Flashing ], [ 44, %state.Alert ], [ %.sink84.ph, %common.ret.sink.split.sink.split.sink.split ]
  %code.i.sink = phi i32 [ 0, %state.Flashing ], [ 0, %state.Alert ], [ %code.i66, %common.ret.sink.split.sink.split.sink.split ]
  %.sink83.ph = phi i1 [ false, %state.Flashing ], [ false, %state.Alert ], [ %.sink83.ph.ph, %common.ret.sink.split.sink.split.sink.split ]
  %fs.str.22.sink.ph = phi ptr [ @fs.str.15, %state.Flashing ], [ @fs.str.20, %state.Alert ], [ %fs.str.22.sink.ph.ph, %common.ret.sink.split.sink.split.sink.split ]
  %.sink82.ph = phi i64 [ 15, %state.Flashing ], [ 15, %state.Alert ], [ %.sink82.ph.ph, %common.ret.sink.split.sink.split.sink.split ]
  %.sink.ph = phi i32 [ 0, %state.Flashing ], [ 0, %state.Alert ], [ %.sink.ph.ph, %common.ret.sink.split.sink.split.sink.split ]
  %alert_code.addr.i = getelementptr inbounds i8, ptr %0, i64 %.sink84
  store i32 %code.i.sink, ptr %alert_code.addr.i, align 4
  br label %common.ret.sink.split

common.ret.sink.split:                            ; preds = %common.ret.sink.split.sink.split, %state.Service, %state.Cars
  %.sink83 = phi i1 [ true, %state.Cars ], [ false, %state.Service ], [ %.sink83.ph, %common.ret.sink.split.sink.split ]
  %fs.str.22.sink = phi ptr [ @fs.str.21, %state.Cars ], [ @fs.str.22, %state.Service ], [ %fs.str.22.sink.ph, %common.ret.sink.split.sink.split ]
  %.sink82 = phi i64 [ 11, %state.Cars ], [ 15, %state.Service ], [ %.sink82.ph, %common.ret.sink.split.sink.split ]
  %.sink = phi i32 [ %3, %state.Cars ], [ 0, %state.Service ], [ %.sink.ph, %common.ret.sink.split.sink.split ]
  %blinking.addr.i79 = getelementptr inbounds i8, ptr %0, i64 48
  store i1 %.sink83, ptr %blinking.addr.i79, align 1
  %message.addr.i80 = getelementptr inbounds i8, ptr %0, i64 56
  store ptr %fs.str.22.sink, ptr %message.addr.i80, align 8
  %message.addr.repack1.i81 = getelementptr inbounds i8, ptr %0, i64 64
  store i64 %.sink82, ptr %message.addr.repack1.i81, align 8
  store i32 %.sink, ptr %0, align 4
  br label %common.ret

common.ret:                                       ; preds = %common.ret.sink.split, %entry, %state.Cars, %state.Waiting, %state.Walk, %state.Flashing, %state.Service, %state.Alert
  ret void

state.Cars:                                       ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 5, label %common.ret.sink.split
    i32 4, label %common.ret.sink.split.sink.split.sink.split
    i32 0, label %transition.exec.0
  ]

transition.exec.0:                                ; preds = %state.Cars
  br label %common.ret.sink.split.sink.split.sink.split

state.Waiting:                                    ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 4, label %common.ret.sink.split.sink.split.sink.split
    i32 1, label %transition.exec.1
  ]

transition.exec.1:                                ; preds = %state.Waiting
  br label %common.ret.sink.split.sink.split.sink.split

state.Walk:                                       ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 4, label %common.ret.sink.split.sink.split.sink.split
    i32 2, label %transition.exec.2
  ]

transition.exec.2:                                ; preds = %state.Walk
  br label %common.ret.sink.split.sink.split.sink.split

state.Flashing:                                   ; preds = %entry
  switch i32 %3, label %common.ret [
    i32 4, label %common.ret.sink.split.sink.split.sink.split
    i32 3, label %common.ret.sink.split.sink.split
  ]

state.Alert:                                      ; preds = %entry
  %cond27 = icmp eq i32 %3, 3
  br i1 %cond27, label %common.ret.sink.split.sink.split, label %common.ret

state.Service:                                    ; preds = %entry
  %cond = icmp eq i32 %3, 3
  br i1 %cond, label %common.ret.sink.split, label %common.ret
}

define void @fs_init_DisplayPanel() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_DisplayPanel, i32 0, ptr nonnull @fs_dispatch_DisplayPanel)
  br i1 %0, label %initialize, label %done

initialize:                                       ; preds = %entry
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 40), align 8
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 44), align 4
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 48), align 16
  store ptr @fs.str.23, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 56), align 8
  store i64 15, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 64), align 16
  br label %done

done:                                             ; preds = %initialize, %entry
  ret void
}

define void @fs_module_init() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_CrossingController, i32 0, ptr nonnull @fs_dispatch_CrossingController)
  br i1 %0, label %initialize.i, label %fs_init_CrossingController.exit

initialize.i:                                     ; preds = %entry
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 40), align 8
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 41), align 1
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 42), align 2
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 44), align 4
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 48), align 16
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 52), align 4
  store ptr @fs.str.11, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 56), align 8
  store i64 4, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 64), align 16
  br label %fs_init_CrossingController.exit

fs_init_CrossingController.exit:                  ; preds = %entry, %initialize.i
  %1 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_DisplayPanel, i32 0, ptr nonnull @fs_dispatch_DisplayPanel)
  br i1 %1, label %initialize.i1, label %fs_init_DisplayPanel.exit

initialize.i1:                                    ; preds = %fs_init_CrossingController.exit
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 40), align 8
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 44), align 4
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 48), align 16
  store ptr @fs.str.23, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 56), align 8
  store i64 15, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 64), align 16
  br label %fs_init_DisplayPanel.exit

fs_init_DisplayPanel.exit:                        ; preds = %fs_init_CrossingController.exit, %initialize.i1
  ret void
}

define void @fs_module_cleanup() local_unnamed_addr {
entry:
  tail call void @fs_cleanup_machine(ptr nonnull @fs_machine_CrossingController)
  tail call void @fs_cleanup_machine(ptr nonnull @fs_machine_DisplayPanel)
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef nonnull ptr @fs_get_machine_CrossingController() local_unnamed_addr #2 {
entry:
  ret ptr @fs_machine_CrossingController
}

define void @fs_run_CrossingController() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_CrossingController, i32 0, ptr nonnull @fs_dispatch_CrossingController)
  br i1 %0, label %initialize.i.i, label %fs_init_CrossingController.exit.i

initialize.i.i:                                   ; preds = %entry
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 40), align 8
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 41), align 1
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 42), align 2
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 44), align 4
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 48), align 16
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 52), align 4
  store ptr @fs.str.11, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 56), align 8
  store i64 4, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 64), align 16
  br label %fs_init_CrossingController.exit.i

fs_init_CrossingController.exit.i:                ; preds = %initialize.i.i, %entry
  %1 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_DisplayPanel, i32 0, ptr nonnull @fs_dispatch_DisplayPanel)
  br i1 %1, label %initialize.i1.i, label %fs_module_init.exit

initialize.i1.i:                                  ; preds = %fs_init_CrossingController.exit.i
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 40), align 8
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 44), align 4
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 48), align 16
  store ptr @fs.str.23, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 56), align 8
  store i64 15, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 64), align 16
  br label %fs_module_init.exit

fs_module_init.exit:                              ; preds = %fs_init_CrossingController.exit.i, %initialize.i1.i
  tail call void @fs_runtime_run(ptr nonnull @fs_machine_CrossingController)
  ret void
}

define i1 @fs_step_CrossingController() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_CrossingController, i32 0, ptr nonnull @fs_dispatch_CrossingController)
  br i1 %0, label %initialize.i.i, label %fs_init_CrossingController.exit.i

initialize.i.i:                                   ; preds = %entry
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 40), align 8
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 41), align 1
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 42), align 2
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 44), align 4
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 48), align 16
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 52), align 4
  store ptr @fs.str.11, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 56), align 8
  store i64 4, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 64), align 16
  br label %fs_init_CrossingController.exit.i

fs_init_CrossingController.exit.i:                ; preds = %initialize.i.i, %entry
  %1 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_DisplayPanel, i32 0, ptr nonnull @fs_dispatch_DisplayPanel)
  br i1 %1, label %initialize.i1.i, label %fs_module_init.exit

initialize.i1.i:                                  ; preds = %fs_init_CrossingController.exit.i
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 40), align 8
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 44), align 4
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 48), align 16
  store ptr @fs.str.23, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 56), align 8
  store i64 15, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 64), align 16
  br label %fs_module_init.exit

fs_module_init.exit:                              ; preds = %fs_init_CrossingController.exit.i, %initialize.i1.i
  %2 = tail call i1 @fs_runtime_step(ptr nonnull @fs_machine_CrossingController)
  ret i1 %2
}

define void @fs_stop_CrossingController() local_unnamed_addr {
entry:
  tail call void @fs_stop_machine(ptr nonnull @fs_machine_CrossingController)
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef nonnull ptr @fs_get_machine_DisplayPanel() local_unnamed_addr #2 {
entry:
  ret ptr @fs_machine_DisplayPanel
}

define void @fs_run_DisplayPanel() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_CrossingController, i32 0, ptr nonnull @fs_dispatch_CrossingController)
  br i1 %0, label %initialize.i.i, label %fs_init_CrossingController.exit.i

initialize.i.i:                                   ; preds = %entry
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 40), align 8
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 41), align 1
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 42), align 2
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 44), align 4
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 48), align 16
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 52), align 4
  store ptr @fs.str.11, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 56), align 8
  store i64 4, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 64), align 16
  br label %fs_init_CrossingController.exit.i

fs_init_CrossingController.exit.i:                ; preds = %initialize.i.i, %entry
  %1 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_DisplayPanel, i32 0, ptr nonnull @fs_dispatch_DisplayPanel)
  br i1 %1, label %initialize.i1.i, label %fs_module_init.exit

initialize.i1.i:                                  ; preds = %fs_init_CrossingController.exit.i
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 40), align 8
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 44), align 4
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 48), align 16
  store ptr @fs.str.23, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 56), align 8
  store i64 15, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 64), align 16
  br label %fs_module_init.exit

fs_module_init.exit:                              ; preds = %fs_init_CrossingController.exit.i, %initialize.i1.i
  tail call void @fs_runtime_run(ptr nonnull @fs_machine_DisplayPanel)
  ret void
}

define i1 @fs_step_DisplayPanel() local_unnamed_addr {
entry:
  %0 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_CrossingController, i32 0, ptr nonnull @fs_dispatch_CrossingController)
  br i1 %0, label %initialize.i.i, label %fs_init_CrossingController.exit.i

initialize.i.i:                                   ; preds = %entry
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 40), align 8
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 41), align 1
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 42), align 2
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 44), align 4
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 48), align 16
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 52), align 4
  store ptr @fs.str.11, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 56), align 8
  store i64 4, ptr getelementptr inbounds (i8, ptr @fs_machine_CrossingController, i64 64), align 16
  br label %fs_init_CrossingController.exit.i

fs_init_CrossingController.exit.i:                ; preds = %initialize.i.i, %entry
  %1 = tail call i1 @fs_init_machine(ptr nonnull @fs_machine_DisplayPanel, i32 0, ptr nonnull @fs_dispatch_DisplayPanel)
  br i1 %1, label %initialize.i1.i, label %fs_module_init.exit

initialize.i1.i:                                  ; preds = %fs_init_CrossingController.exit.i
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 40), align 8
  store i32 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 44), align 4
  store i8 0, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 48), align 16
  store ptr @fs.str.23, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 56), align 8
  store i64 15, ptr getelementptr inbounds (i8, ptr @fs_machine_DisplayPanel, i64 64), align 16
  br label %fs_module_init.exit

fs_module_init.exit:                              ; preds = %fs_init_CrossingController.exit.i, %initialize.i1.i
  %2 = tail call i1 @fs_runtime_step(ptr nonnull @fs_machine_DisplayPanel)
  ret i1 %2
}

define void @fs_stop_DisplayPanel() local_unnamed_addr {
entry:
  tail call void @fs_stop_machine(ptr nonnull @fs_machine_DisplayPanel)
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(read, argmem: readwrite, inaccessiblemem: none) }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: write) }
