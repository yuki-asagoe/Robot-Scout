using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wimm.Common;
using Wimm.Machines.Component;
using Wimm.Machines.Impl.Scout.Tpip4.Can;

namespace Wimm.Machines.Impl.Scout.Tpip4.Component
{
    internal class SubMotor : Motor
    {
        internal enum DriverPort { M1 = 0, M2 = 1 }
        public SubMotor(string name, string description, CanCommunicationUnit canMessage, Func<double> speedModifierProvider) : base(name, description)
        {
            Port = DriverPort.M1;
            if (canMessage.Data.Length != 5)
            {
                throw new ArgumentException($"与えられた<{nameof(CanCommunicationUnit)}>インスタンスのデータのサイズが5byteではありません。");
            }
            CanMessageFrame = canMessage;
            SpeedModifierProvider = speedModifierProvider;
        }
        private DriverPort Port { get; }
        private CanCommunicationUnit CanMessageFrame { get; }
        private Func<double> SpeedModifierProvider { get; }

        private void RotationImpl(double speed)
        {
            speed = Math.Clamp(speed, -1, 1) * SpeedModifierProvider();
            CanMessageFrame.Data[(int)Port * 2] = (byte)Math.Sign(speed); //回転方向決定
            CanMessageFrame.Data[(int)Port * 2 + 1] = (byte)(byte.MaxValue * Math.Abs(speed)); // 回転速度決定
        }

        public override Feature<Action<double>> RotationFeature => new Feature<Action<double>>(
            Motor.RotationFeatureName,
            "モーターを回転させます。\n\n[引数]\n- double speed - 範囲 -1 ~ 1 です。",
            RotationImpl
        );

        public override string ModuleName => "スカウト サブモーター";

        public override string ModuleDescription => "スカウト搭載のオーパーツモーターのM1実装";
    }
}
