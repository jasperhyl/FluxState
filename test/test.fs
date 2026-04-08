// test/test.fs
// 严格匹配当前实现版本的语法与语义能力。
//
// 注意：
// 1. 不使用 this，因为当前实现不支持。
// 2. 不使用普通函数调用表达式，因为当前 sema 会将 CallExpr 判为 unsupported。
// 3. emit 使用正确语法：
//      emit(TargetMachine, Event(...));
// 4. 当前版本已经支持使用 machine 名称作为 emit target，
//    因此本文件同时覆盖同机 emit 与跨 machine emit。
// 5. 当前不支持块内变量定义，只能在machine级别定义

machine SyntaxCoverage {
    state Idle, Armed, Waiting, Cooling, Error;
    state Done;
    initial Idle;

    int peer = 0;
    int sink = 1;
    short retry_count;
    float ratio = 2;
    double threshold = 3;
    char marker;
    bool enabled = true;
    bool latched = false;
    string label = "boot";
    string note = "idle";

    event Ping();
    event Configure(int code, short retry, float scale, double limit, char tag, bool enabled_flag, string name);
    event Dispatch(int first, int second, bool flag);
    event Reset(string reason);
    event Finish(bool ok, string message);

    from Idle on Ping() -> Armed
    do {
        enabled = true;
        latched = false;
        label = "armed";
        peer = 3;
        sink = (peer + retry_count) * 2;
    }

    from Armed on Configure(code, retry, scale, limit, tag, enabled_flag, name) when enabled && enabled_flag == true -> Waiting
    do {
        peer = code + retry * 2 - (3 / 1);
        retry_count = retry;
        ratio = scale + 1;
        threshold = limit / 2;
        marker = tag;
        enabled = enabled_flag;
        latched = enabled_flag && enabled;
        label = name;
        note = "configured";
        emit(SyntaxCoverage, Dispatch(code, retry, enabled));
        emit(Secondary, Start(code));
        emit(Secondary, Flip(enabled, name));
        {
            peer = (code + retry) * (retry - 1);
            sink = peer;
            note = "nested";
        }
    }

    from Waiting after 5s when (peer > 0 && enabled) || false -> Cooling
    do {
        label = "cooling";
        latched = true;
    }

    from Cooling after 1m after 2 -> Done
    do {
        note = "done";
        emit(SyntaxCoverage, Ping());
    }

    from Done after 1h -> Error
    do {
        peer = -1;
        enabled = false;
        latched = false;
        label = "error";
    }

    from Error on Reset(reason) when peer != 0 || enabled == false -> Idle
    do {
        label = reason;
        peer = 0;
        sink = 0;
    }

    from Error on Ping() -> Error
    do {
        note = "loop";
        emit(SyntaxCoverage, Finish(enabled, label));
    }
}

machine Secondary {
    state S0, S1, S2;
    initial S0;

    int sink = 0;
    short count;
    bool flag = false;
    string note = "secondary";

    event Start(int value);
    event Stop();
    event Flip(bool next_flag, string next_note);

    from S0 on Start(value) when value >= 0 -> S1
    do {
        sink = value;
        note = "started";
        emit(SyntaxCoverage, Ping());
    }

    from S1 on Flip(next_flag, next_note) when sink <= 100 -> S2
    do {
        flag = next_flag;
        note = next_note;
        sink = sink + count;
        emit(Secondary, Stop());
        emit(SyntaxCoverage, Finish(next_flag, next_note));
    }

    from S2 on Stop() when sink < 10 || flag == false -> S0
    do {
        sink = sink + 1;
        flag = false;
        note = "restart";
        emit(SyntaxCoverage, Reset(note));
    }
}
