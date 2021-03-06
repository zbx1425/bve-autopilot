// 勾配グラフ.h : 勾配による列車の挙動への影響を計算します
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

#pragma once
#include <map>
#include "区間.h"
#include "物理量.h"

#pragma warning(push)
#pragma warning(disable:4819)

namespace autopilot
{

    class 勾配グラフ
    {
    public:
        勾配グラフ();
        ~勾配グラフ();

        void 消去();
        void 勾配区間追加(m 始点, double 勾配);
        void 通過(m 位置);

        // 指定した範囲に列車が存在するときの勾配による加速度への影響を
        // 計算します。
        // 下り勾配では正の加速度がかかるので正の値を返します。
        mps2 勾配加速度(区間 対象範囲) const;

    private:
        struct 勾配区間;

        // 区間の始点からその区間のデータへの写像
        std::map<m, 勾配区間> _区間リスト;
    };

}

#pragma warning(pop)
