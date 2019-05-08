// 減速パターン.cpp : 等加速度で減速するパターンを表します
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
#include "減速パターン.h"
#include <algorithm>
#include <limits>
#include <tuple>
#include "共通状態.h"
#include "走行モデル.h"

namespace autopilot
{

    速度型 減速パターン::単純期待速度(距離型 位置) const
    {
        if (位置 >= 目標位置) {
            return 目標速度;
        }

        走行モデル 走行(目標位置, 目標速度, 0);
        走行.指定位置まで走行(位置, -目標減速度);
        return 走行.速度();
    }

    std::pair<速度型, 加速度型> 減速パターン::期待速度と期待減速度(
        距離型 現在位置) const
    {
        if (現在位置 >= 目標位置) {
            return { 目標速度, 0 };
        }

        // 目標位置・目標速度の状態から逆算する
        走行モデル 走行(目標位置, 目標速度, 0);

        // 目標に近付いたら減速度を下げる
        // ただし停止目標に対してこれをやると手前に止まりがちなのでやらない
        加速度型 期待減速度 = 目標減速度 / 2;
        if (目標速度 > 0) {
            走行.指定時間走行(-5, -期待減速度);
        }

        if (走行.位置() <= 現在位置) {
            // 行き過ぎたので計算し直す
            走行.変更(目標位置, 目標速度, 0);
            走行.指定位置まで走行(現在位置, -期待減速度);
        }
        else {
            // 現在位置がもっと手前なら目標減速度で減速する
            期待減速度 = 目標減速度;
            走行.指定位置まで走行(現在位置, -期待減速度);
        }

        return { 走行.速度(), 期待減速度 };
    }

    int 減速パターン::出力制動ノッチ(
        距離型 現在位置, 速度型 現在速度, const 制動特性 & 制動,
        加速度型 勾配影響) const
    {
        距離型 残距離 = 目標位置 - 現在位置;
        速度型 期待速度;
        加速度型 期待減速度;
        std::tie(期待速度, 期待減速度) = 期待速度と期待減速度(現在位置);

        double 出力制動ノッチ相当;

        if (残距離 >= 0) {
            加速度型 出力減速度 =
                期待減速度 * (現在速度 / 期待速度) - (期待速度 - 現在速度) / 2;
            // 期待(減)速度に漸次的に近付けるように減速度を調整する。
            // 基本的には 2 秒後に期待速度に到達するような減速度を出力する。
            // ただし現在速度が期待速度と異なることにより
            // 期待速度の実際の減速度も期待減速度とは異なるので
            // 期待減速度に現在速度と期待速度の比を掛けて補正する。

            出力制動ノッチ相当 = 制動.ノッチ(出力減速度 + 勾配影響);
            if (出力減速度 < 期待減速度) {
                出力制動ノッチ相当 = std::floor(出力制動ノッチ相当);
            }
            else {
                出力制動ノッチ相当 = std::round(出力制動ノッチ相当);
            }
        }
        else {
            // 過走した!
            出力制動ノッチ相当 = 制動.常用ノッチ数();
        }

        // 目標速度に近付いたらノッチを緩めて衝撃を抑える
        double 緩めノッチ = std::ceil(
            制動.ノッチ((現在速度 - 目標速度) / 制動.緩解時間() + 勾配影響));
        出力制動ノッチ相当 = std::min(出力制動ノッチ相当, 緩めノッチ);

        int 出力制動ノッチ = static_cast<int>(出力制動ノッチ相当);
        if (出力制動ノッチ <= 制動.無効ノッチ数()) {
            出力制動ノッチ = 0;
        }
        else if (出力制動ノッチ > 制動.常用ノッチ数()) {
            出力制動ノッチ = 制動.常用ノッチ数();
        }

        return 出力制動ノッチ;
    }

    int 減速パターン::出力ノッチ(
        距離型 現在位置, 速度型 現在速度, const 共通状態 & 状態) const
    {
        加速度型 勾配影響 = 状態.車両勾配加速度();
        int 出力制動ノッチ = this->出力制動ノッチ(
            現在位置, 現在速度, 状態.制動(), 勾配影響);

        // 制動を弱めてもまたすぐ強くすることになるなら弱めない
        加速度型 期待減速度 = 期待速度と期待減速度(現在位置).second;
        速度型 基準速度 = 目標速度 + 状態.制動().緩解時間() * 期待減速度;
        if (出力制動ノッチ < 状態.前回出力().Brake && 現在速度 > 基準速度) {
            時間型 猶予 = (現在速度 - 基準速度) / 期待減速度 / 8;
            加速度型 出力減速度 =
                状態.制動().減速度(出力制動ノッチ) - 勾配影響;
            走行モデル モデル(現在位置, 現在速度, 0);
            モデル.指定時間走行(猶予, -出力減速度);
            int 猶予後出力制動ノッチ = this->出力制動ノッチ(
                モデル.位置(), モデル.速度(), 状態.制動(), 勾配影響);
            出力制動ノッチ = std::max(出力制動ノッチ,
                std::min(猶予後出力制動ノッチ, 状態.前回出力().Brake));
        }

        if (出力制動ノッチ > 0) {
            return -出力制動ノッチ;
        }

        // 制限速度が近いなら力行させない
        走行モデル モデル(現在位置, 現在速度, 0);
        モデル.指定時間走行(1,
            状態.前回出力().Power > 0 ? 状態.加速度() : mps_from_kmph(5));
        モデル.指定時間走行(5, std::max(勾配影響, 0.0));
        出力制動ノッチ = this->出力制動ノッチ(
            モデル.位置(), モデル.速度(), 状態.制動(), 勾配影響);
        if (出力制動ノッチ > 0) {
            return 0;
        }
        return std::numeric_limits<int>::max();
    }

}
