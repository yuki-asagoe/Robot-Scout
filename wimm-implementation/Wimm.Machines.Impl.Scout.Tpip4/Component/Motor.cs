using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wimm.Common;
using Wimm.Machines.Component;

namespace Wimm.Machines.Impl.Scout.Tpip4.Component
{
    internal class ScoutMotor : Motor
    {
        public ScoutMotor(string name, string description, MotorDriver host, byte motorID, Func<double> speedModifierProvider) : base(name, description)
        {
            HostDriver = host;
            MotorID = motorID;
            SpeedModifierProvider = speedModifierProvider;
            Features = [
                .. Features,
                new Feature<Delegate>(
                    "stop",
                    "モーターの回転を停止します",
                    Stop
                ),
                new Feature<Delegate>(
                    "brake",
                    "モーターにショートブレーキをかけます",
                    Brake
                ),
            ];
        }
        private MotorDriver HostDriver { get; }
        private byte MotorID { get; }
        private Func<double> SpeedModifierProvider { get; }

        public override Feature<Action<double>> RotationFeature
            => new Feature<Action<double>>(
                Motor.RotationFeatureName,
                "モーターを回転させます。\n\n[引数]\n- double speed - 範囲 -1 ~ 1 です。",
                Rotation
            );
        public void Rotation(double speed)
        {
            MotorDriver.MotorMode mode;
            if (Math.Abs(speed) < 0.0001)
            {
                mode = MotorDriver.MotorMode.Stop;
            }
            else if (speed > 0)
            {
                mode = MotorDriver.MotorMode.Drive;
            }
            else
            {
                mode = MotorDriver.MotorMode.ReverseDrive;
            }
            HostDriver.Put(new MotorDriver.Message(MotorID, mode));
        }
        public void Brake()
        {
            HostDriver.Put(new MotorDriver.Message(MotorID, MotorDriver.MotorMode.Brake));
        }
        public void Stop()
        {
            HostDriver.Put(new MotorDriver.Message(MotorID, MotorDriver.MotorMode.Stop));
        }

        public override string ModuleName => "スカウト モーター";

        public override string ModuleDescription => "スカウトの汎用モータ";
    }
}
