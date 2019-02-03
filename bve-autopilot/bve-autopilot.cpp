﻿// bve-autopilot.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//
// Copyright © 2019 Watanabe, Yuki
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA

#include "stdafx.h"
#include <memory>
#include "Main.h"

static std::unique_ptr<autopilot::Main> main;

ATS_API int WINAPI GetPluginVersion() {
    return ATS_VERSION;
}

ATS_API void WINAPI Load() {
    main = std::make_unique<autopilot::Main>();
}

ATS_API void WINAPI Dispose() {
    main = nullptr;
}

ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC spec) {
    if (main != nullptr) {
        main->車両仕様設定(spec);
    }
}

ATS_API void WINAPI Initialize(int brake) {
    if (main != nullptr) {
        main->リセット(brake);
    }
}

ATS_API ATS_HANDLES WINAPI Elapse(
    ATS_VEHICLESTATE state, int *panelValues, int *soundStates) {
    if (main != nullptr) {
        return main->経過(state, panelValues, soundStates);
    }
    return ATS_HANDLES{};
}

ATS_API void WINAPI SetPower(int notch) {
    if (main != nullptr) {
        main->力行操作(notch);
    }
}

ATS_API void WINAPI SetBrake(int notch) {
    if (main != nullptr) {
        main->制動操作(notch);
    }
}

ATS_API void WINAPI SetReverser(int notch) {
    if (main != nullptr) {
        main->逆転器操作(notch);
    }
}

ATS_API void WINAPI KeyDown(int key) {
}

ATS_API void WINAPI KeyUp(int key) {
}

ATS_API void WINAPI HornBlow(int type) {
}

ATS_API void WINAPI DoorOpen() {
}

ATS_API void WINAPI DoorClose() {
}

ATS_API void WINAPI SetSignal(int aspect) {
}

ATS_API void WINAPI SetBeaconData(ATS_BEACONDATA data) {
}
