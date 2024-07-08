using System.Collections.Immutable;
using Wimm.Common;
using Wimm.Machines.Impl.Scout.Tpip4.Component;
using Wimm.Machines.TpipForRasberryPi;

namespace Wimm.Machines.Impl.Scout.Tpip4
{
    public class Scout : TpipForRasberryPiMachine
    {
        public override string Name => "スカウト";
        private ImmutableArray<MotorDriver> MotorDrivers;
        public Scout(MachineConstructorArgs? args) : base(args)
        {
            Camera = new Tpip4Camera("__");
            if (args is not null && Camera is Tpip4Camera camera) { Hwnd?.AddHook(camera.WndProc); }
            Information = [
                new InformationNode("MotorDriver",
                    MotorDrivers.Select(it =>
                        new InformationNode(
                            it.I2CAddress.ToString(), []
                        )
                    ).ToImmutableArray()
                )
            ];
            MotorDrivers = [new MotorDriver(0x55)];
            {
                var speedModifier = () => SpeedModifier;
                StructuredModules = new ModuleGroup("modules",
                    [
                        new ModuleGroup("cralwers",
                            [],
                            [
                                new ScoutMotor("left","機動用左クローラ",MotorDrivers[0],0,speedModifier),
                                new ScoutMotor("right","機動用右クローラ",MotorDrivers[0],1,speedModifier)
                            ]
                        ),
                        new ModuleGroup("itemcontainer",
                            [],
                            [
                                new ScoutMotor("puller","",MotorDrivers[0],2,speedModifier),
                                //new ScoutMotor("right","機動用右クローラ",MotorDrivers[0],3,speedModifier)
                            ]
                        )
                    ],
                    []
                );
            }
        }
        protected override ControlProcess StartControlProcess()
        {
            return new ScoutControlProcess(MotorDrivers);
        }

        internal class ScoutControlProcess(IEnumerable<MotorDriver> motorDrivers) : ControlProcess
        {
            private IEnumerable<MotorDriver> MotorDrivers = motorDrivers;
            public override void Dispose()
            {
                base.Dispose();
                foreach (var i in MotorDrivers)
                {
                    i.SendAll();
                }
            }
        }
    }
}
