using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wimm.Common;
using Wimm.Machines.Impl.Scout.Tpip4.Can;
using static Wimm.Machines.TpipForRasberryPi.Import.TPJT4;

namespace Wimm.Machines.Impl.Scout.Tpip4.Component
{
    internal class ItemContainer : Module
    {
        private static readonly short[] StopLevels = [50, 255, 511, 766, 980];
        public ItemContainer(string name, string description, CanCommunicationUnit canMessage) : base(name, description)
        {
            CanMessageFrame = canMessage;
            Features = [
                ..Features,
                new Feature<Delegate>(
                    "setposition",
                    "支援物資コンテナのモータを指定位置まで回転します。\n\n[引数]\n- int position - 範囲 0 ~ 4 です。",
                    SetPositionImpl
                )
            ];
        }
        private CanCommunicationUnit CanMessageFrame { get; }
        private int currentLevel { get; set; } = 0;
        private void SetPositionImpl(int position)
        {
            if(position<0 || position >= StopLevels.Length || position == currentLevel)
            {
                return;
            }
            bool rotateForward = position > currentLevel;
            CanMessageFrame.Data[3] = unchecked((byte)(rotateForward ? 1 : -1));
            CanMessageFrame.Data[4] = (byte)((0xFF00 & StopLevels[position])>>8);
            CanMessageFrame.Data[5] = (byte)(0xFF & StopLevels[position]);
        }
        public override string ModuleName => "スカウト 支援物資コンテナ";

        public override string ModuleDescription => "スカウト搭載の支援物資投下用コンテナ";

    }
}
