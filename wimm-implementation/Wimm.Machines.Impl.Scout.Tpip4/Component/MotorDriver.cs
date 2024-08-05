using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Wimm.Machines.TpipForRasberryPi.Import;

namespace Wimm.Machines.Impl.Scout.Tpip4.Component
{
    internal class MotorDriver(byte i2cAddress)
    {
        public byte I2CAddress { get; } = i2cAddress;
        private Message?[] MessagesBuffer { get; } = new Message?[5];

        public void Put(Message message) => MessagesBuffer[message.MotorID] = message;
        public void SendAll()
        {
            var messages = new LinkedList<byte>();
            foreach (var m in MessagesBuffer)
            {
                if(m is Message v)
                {
                    messages.AddLast(v.Construct());
                }
            }
            var data = messages.ToArray();
            Array.Fill(MessagesBuffer, null);
            Debug.WriteLine($"Scout Motor Driver (DEC:{this.I2CAddress}) : {data.Length} byte sent");
            TPJT4.NativeMethods.Send_I2Cdata(0, data, I2CAddress, data.Length);
        }

        public record struct Message(byte MotorID, MotorMode Type)
        {
            public byte Construct() => (byte)((MotorID << 4) | ((byte)Type));
        }
        public enum MotorMode : byte
        {
            Stop = 0, Brake, Drive, ReverseDrive
        }
    }
}
