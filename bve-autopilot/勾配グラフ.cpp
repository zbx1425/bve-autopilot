// 勾配グラフ.cpp : 勾配による列車の挙動への影響を計算します
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
#include "勾配グラフ.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <utility>
#include "区間.h"

#pragma warning(disable:4819)

namespace autopilot
{

    namespace
    {

        constexpr mps2 重力加速度 = 9.80665_mps2;

    }

    struct 勾配グラフ::勾配区間
    {
        double 勾配;
        mps2 影響加速度;

        勾配区間(double 勾配) :
            勾配{勾配}, 影響加速度{-0.75 * 重力加速度 * 勾配} { }
        // 本当は tan を sin に変換すべきだがほとんど違わないので無視する

    };

    勾配グラフ::勾配グラフ() = default;
    勾配グラフ::~勾配グラフ() = default;

    void 勾配グラフ::消去()
    {
        _区間リスト.clear();
    }

    void 勾配グラフ::勾配区間追加(m 始点, double 勾配)
    {
        // データを追加するだけなら
        // _区間リスト.insert_or_assign(始点, 勾配区間{勾配});
        // だけでもよいのだが、無駄に多くのデータを追加しないように
        // 以下の長々としたコードで最適化する。

        auto i = _区間リスト.lower_bound(始点);

        if (i != _区間リスト.end()) {
            if (勾配 == i->second.勾配) {
                // 既に同じ勾配の区間があるなら区間を追加しない
                auto n = _区間リスト.extract(i++);
                assert(始点 <= n.key());
                n.key() = 始点;
                _区間リスト.insert(i, std::move(n));
                return;
            }

            if (始点 == i->first) {
                // 既に同じ位置に区間があるなら上書きする
                i->second = 勾配区間{勾配};
                return;
            }
        }

        if (i != _区間リスト.begin()) {
            auto j = std::prev(i);
            assert(j->first < 始点);
            if (勾配 == j->second.勾配) {
                // 既に同じ勾配の区間があるなら区間を追加しない
                return;
            }
        }
        else if (勾配 == 0.0) {
            // 勾配区間のない位置で勾配 0 の区間を作るのは無意味
            return;
        }

        auto j = _区間リスト.try_emplace(i, 始点, 勾配);
        assert(std::next(j) == i);
    }

    void 勾配グラフ::通過(m 位置)
    {
        if (_区間リスト.empty()) {
            return;
        }

        // 通過済みの区間を消す
        auto i = _区間リスト.begin();
        while (true) {
            auto j = std::next(i);
            if (j == _区間リスト.end() || j->first > 位置) {
                break;
            }
            i = j;
        }
        i = _区間リスト.erase(_区間リスト.begin(), i);
        assert(!_区間リスト.empty());
        assert(i == _区間リスト.begin());

        // 傾きが 0 の区間は未通過でも消す
        if (i->second.勾配 == 0.0) {
            _区間リスト.erase(i);
        }
    }

    mps2 勾配グラフ::勾配加速度(区間 対象範囲) const
    {
        m 全体長さ = 対象範囲.長さ();
        if (!(全体長さ > 0.0_m)) {
            return 0.0_mps2;
        }

        mps2 加速度 = 0.0_mps2;
        auto 終点 = m::無限大();
        for (auto i = _区間リスト.rbegin();
            i != _区間リスト.rend();
            終点 = i++->first)
        {
            auto 影響区間 = 重なり({i->first, 終点}, 対象範囲);
            m 影響長さ = 影響区間.長さ();
            if (!(影響長さ > 0.0_m)) {
                continue;
            }

            double 影響割合 = 影響長さ / 全体長さ;
            if (std::isnan(影響割合)) {
                影響割合 = 1;
            }
            加速度 += i->second.影響加速度 * 影響割合;
        }
        return 加速度;
    }

}
