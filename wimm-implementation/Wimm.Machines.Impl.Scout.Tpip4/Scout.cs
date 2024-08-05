using System.Collections.Immutable;
using Wimm.Common;
using Wimm.Machines.Impl.Scout.Tpip4.Can;
using Wimm.Machines.Impl.Scout.Tpip4.Component;
using Wimm.Machines.TpipForRasberryPi;

namespace Wimm.Machines.Impl.Scout.Tpip4
{
    [LoadTarget]
    public class Scout : TpipForRasberryPiMachine
    {
        public override string Name => "スカウト";
        private ImmutableArray<MotorDriver> MotorDrivers;
        private CanCommunicationUnit SubMotorUnit=new(
            new() {
                DestinationAddress=(CanDestinationAddress)0x55,
                SourceAddress=(CanDestinationAddress)0x22,
                MessageType=CanDataType.Command
            },
            5
        );
        public Scout(MachineConstructorArgs? args) : base(args)
        {
            Camera = new Tpip4Camera("__");
            if (args is not null && Camera is Tpip4Camera camera) { Hwnd?.AddHook(camera.WndProc); }
            MotorDrivers = [new MotorDriver(0x55)];
            Information = [
                new InformationNode("MotorDriver",
                    MotorDrivers.Select(it =>
                        new InformationNode(
                            it.I2CAddress.ToString(), []
                        )
                    ).ToImmutableArray()
                )
            ];
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
                    ],
                    [
                        new SubMotor("arm_motor","ロジャーアームモータ",SubMotorUnit,speedModifier),
                        new ItemContainer("container","支援物資コンテナ",SubMotorUnit)
                    ]
                );
            }
        }
        protected override ControlProcess StartControlProcess()
        {
            SubMotorUnit.ResetData();
            return new ScoutControlProcess(MotorDrivers, [SubMotorUnit]);
        }

        internal class ScoutControlProcess(IEnumerable<MotorDriver> motorDrivers,IEnumerable<CanCommunicationUnit> canFrames) : ControlProcess
        {
            private IEnumerable<MotorDriver> MotorDrivers = motorDrivers;
            public override void Dispose()
            {
                base.Dispose();
                foreach (var i in MotorDrivers)
                {
                    i.SendAll();
                }
                foreach(var i in canFrames)
                {
                    i.Send();
                }
            }
        }
    }
}
