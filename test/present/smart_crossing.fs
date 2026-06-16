// Smart crossing demo for FluxState.
//
// Real scenario:
//   A city intersection controller reacts to pedestrian button presses,
//   emergency vehicle priority, and maintenance mode. A second machine models
//   the roadside display panel that receives events from the controller.
//
// This file intentionally stays within the currently supported FluxState
// language subset:
//   - no this
//   - no function calls in actions
//   - no local variable declarations inside blocks
//   - no string equality guards

machine CrossingController {
    state VehicleGreen, VehicleYellow, PedWalk, PedFlash;
    state Emergency, Maintenance;
    initial VehicleGreen;

    bool ped_waiting = false;
    bool emergency_active = false;
    bool maintenance_mode = false;
    int last_sensor_id = 0;
    int emergency_code = 0;
    int crossings_served = 0;
    string phase = "cars";

    event PedButton(int sensor_id);
    event EmergencyOn(int code);
    event EmergencyOff();
    event MaintenanceOn();
    event MaintenanceOff();

    from VehicleGreen on PedButton(sensor_id) when emergency_active == false && maintenance_mode == false -> VehicleYellow
    do {
        ped_waiting = true;
        last_sensor_id = sensor_id;
        phase = "prepare_walk";
        emit(DisplayPanel, ShowWaiting(sensor_id));
    }

    from VehicleYellow after 1s when ped_waiting == true -> PedWalk
    do {
        phase = "walk";
        emit(DisplayPanel, ShowWalk(last_sensor_id));
    }

    from PedWalk after 1s -> PedFlash
    do {
        phase = "flash";
        emit(DisplayPanel, ShowFlash(last_sensor_id));
    }

    from PedFlash after 1s -> VehicleGreen
    do {
        ped_waiting = false;
        crossings_served = crossings_served + 1;
        phase = "cars";
        emit(DisplayPanel, ShowCars());
    }

    from VehicleGreen on EmergencyOn(code) -> Emergency
    do {
        emergency_active = true;
        emergency_code = code;
        phase = "emergency";
        emit(DisplayPanel, ShowEmergency(code));
    }

    from VehicleYellow on EmergencyOn(code) -> Emergency
    do {
        emergency_active = true;
        emergency_code = code;
        phase = "emergency";
        emit(DisplayPanel, ShowEmergency(code));
    }

    from PedWalk on EmergencyOn(code) -> Emergency
    do {
        emergency_active = true;
        emergency_code = code;
        phase = "emergency";
        emit(DisplayPanel, ShowEmergency(code));
    }

    from PedFlash on EmergencyOn(code) -> Emergency
    do {
        emergency_active = true;
        emergency_code = code;
        phase = "emergency";
        emit(DisplayPanel, ShowEmergency(code));
    }

    from Emergency on EmergencyOff() -> VehicleGreen
    do {
        emergency_active = false;
        emergency_code = 0;
        ped_waiting = false;
        phase = "cars";
        emit(DisplayPanel, ShowCars());
    }

    from VehicleGreen on MaintenanceOn() -> Maintenance
    do {
        maintenance_mode = true;
        phase = "maintenance";
        emit(DisplayPanel, ShowMaintenance());
    }

    from Maintenance on MaintenanceOff() -> VehicleGreen
    do {
        maintenance_mode = false;
        phase = "cars";
        emit(DisplayPanel, ShowCars());
    }
}

machine DisplayPanel {
    state Cars, Waiting, Walk, Flashing, Alert, Service;
    initial Cars;

    int current_sensor = 0;
    int alert_code = 0;
    bool blinking = false;
    string message = "vehicles may go";

    event ShowWaiting(int sensor_id);
    event ShowWalk(int sensor_id);
    event ShowFlash(int sensor_id);
    event ShowCars();
    event ShowEmergency(int code);
    event ShowMaintenance();

    from Cars on ShowWaiting(sensor_id) -> Waiting
    do {
        current_sensor = sensor_id;
        blinking = false;
        message = "please wait";
    }

    from Waiting on ShowWalk(sensor_id) -> Walk
    do {
        current_sensor = sensor_id;
        blinking = false;
        message = "walk";
    }

    from Walk on ShowFlash(sensor_id) -> Flashing
    do {
        current_sensor = sensor_id;
        blinking = true;
        message = "finish crossing";
    }

    from Flashing on ShowCars() -> Cars
    do {
        current_sensor = 0;
        blinking = false;
        message = "vehicles may go";
    }

    from Cars on ShowEmergency(code) -> Alert
    do {
        alert_code = code;
        blinking = true;
        message = "emergency priority";
    }

    from Waiting on ShowEmergency(code) -> Alert
    do {
        alert_code = code;
        blinking = true;
        message = "emergency priority";
    }

    from Walk on ShowEmergency(code) -> Alert
    do {
        alert_code = code;
        blinking = true;
        message = "emergency priority";
    }

    from Flashing on ShowEmergency(code) -> Alert
    do {
        alert_code = code;
        blinking = true;
        message = "emergency priority";
    }

    from Alert on ShowCars() -> Cars
    do {
        alert_code = 0;
        blinking = false;
        message = "vehicles may go";
    }

    from Cars on ShowMaintenance() -> Service
    do {
        blinking = true;
        message = "maintenance";
    }

    from Service on ShowCars() -> Cars
    do {
        blinking = false;
        message = "vehicles may go";
    }
}
