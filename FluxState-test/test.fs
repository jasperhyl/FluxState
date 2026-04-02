machine SmartCrossing {
    state CarGreen, CarYellow, PedWalk, PedFlash, Emergency;
    initial CarGreen;
    bool ped_waiting = false;
    event pedPress();
    event emergencyOn();
    event emergencyOff();
    from CarGreen on pedPress() -> CarGreen
    do {
        ped_waiting = true;
        log("pedestrian request accepted");
    }
    from CarGreen after 30s when ped_waiting == true -> CarYellow
    do {
        log("switching to yellow");
    }
    from CarYellow after 5s -> PedWalk
    do {
        log("pedestrians may cross");
    }
    from PedWalk after 10s -> PedFlash
    from PedFlash after 5s -> CarGreen
    do {
        ped_waiting = false;
        log("return to vehicle priority");
    }
    from CarGreen on emergencyOn() -> Emergency
    from CarYellow on emergencyOn() -> Emergency
    from PedWalk on emergencyOn() -> Emergency
    from PedFlash on emergencyOn() -> Emergency
    from Emergency on emergencyOff() -> CarGreen
    do {
        ped_waiting = false;
        log("emergency cleared");
    }
}